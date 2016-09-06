//
// Created by skr on 03.09.16.
//
#include <stdint.h>
#include "storage.h"
#include "macros.h"
#include <glib-2.0/glib.h>
#include <string.h>
#include "minini/minIni.h"

const char FileName[] = "FileName";

// Вид, в котором строковое значение хранится в хранилище
typedef struct {
    char * Str;     // Значение
    size_t Copies;  // Количество копий данного контейнера в хранилище
} T_Container;

// Указатель на само хранилище
GHashTable * Storage = NULL;
// Количество клиентов, обращающихся к хранилищу в данный момент
uint32_t ReadersQuan = 0;

// Мьютексы для организации доступа к хранилищу
pthread_mutex_t ReaderBlock;
pthread_mutex_t OrderBlock;
pthread_mutex_t AccessBlock;

// Функции блокировки и разблокировки для доступа к хранилищу
static void LockR();
static void UnlockR();
static void LockW();
static void UnlockW();
// Функция, вызываемая в функции serialize_full
static void WriteToFile( gpointer aKey, gpointer aValue, gpointer aUd );
// Запись в файл
static int serialize_full();
// Служебная функция дедупликации
static void ModifyDouble( gpointer aKey, gpointer aValue, gpointer aUd);
// Служебная функция добавления значения в строку
static void AddToStr( gpointer aKey, gpointer aValue, gpointer aUd );
// Служебная функция освобождения памяти из-под ключа
static void FreeKey(gpointer aData);
// Служебная функция освобождения памяти из-под контейнера
static void FreeContainer(gpointer aData);
// Служебная функция создания контейнера
static int CreateContainer( char * aStr, T_Container **aContainer );
// Служебная функция загрузки данных в хранилище из файла
static int LoadFromFile();

//-------------------------------------------------------------------------
//--- Функции блокировки и разблокировки для доступа к хранилищу
static void LockR()
{
    pthread_mutex_lock( &OrderBlock );
    pthread_mutex_lock( &ReaderBlock );
    if ( ReadersQuan == 0 ) {
        pthread_mutex_lock( &AccessBlock );
    }
    ReadersQuan++;
    pthread_mutex_unlock( &OrderBlock );
    pthread_mutex_unlock( &ReaderBlock );

    //reading
    //...
}
static void UnlockR()
{
    //...
    //reading

    pthread_mutex_lock( &ReaderBlock );
    ReadersQuan--;
    if ( ReadersQuan == 0 ) {
        pthread_mutex_unlock( &AccessBlock );
    }
    pthread_mutex_unlock( &ReaderBlock );
}
static void LockW()
{
    pthread_mutex_lock( &OrderBlock );
    pthread_mutex_lock( &AccessBlock );
    pthread_mutex_unlock( &OrderBlock );

    //...
    // writing
}
static void UnlockW()
{
    //...
    //writing
    pthread_mutex_unlock( &AccessBlock );
}

//-------------------------------------------------------------------------
// Запись в файл одного элемента
static void WriteToFile( gpointer aKey, gpointer aValue, gpointer aUd )
{
    ASSERT_PAR_R_VOID( aKey );
    ASSERT_PAR_R_VOID( aValue );

    T_Container * Container = ( T_Container * )aValue;
    char * Key = ( char *) aKey;
    uint32_t KeyQuan = 0;

    static const char KeyQuanKey[] = "KeyQuan";
    char Buffer[256];
    // Чтение количества ключей, связанных с данным значением
    ini_gets( Container->Str, KeyQuanKey, 0, Buffer, 256, FileName );
    sscanf( Buffer, "%u", &KeyQuan );
    KeyQuan++;
    snprintf( Buffer, 256, "%u", KeyQuan );
    // Запись нового количества ключей (если было 0, стало 1)
    ini_puts( Container->Str, KeyQuanKey, Buffer, FileName );
    snprintf( Buffer, 256, "Key_%u", KeyQuan );
    // Запись ключа
    ini_puts( Container->Str, Buffer, Key, FileName );
}

//-------------------------------------------------------------------------
// Запись в файл
static int serialize_full()
{
    LockR();

    // Стирание файла перед записью
    FILE * file;
    file = fopen( FileName, "w+" );
    if ( file ) {
        fclose(file);
    }

    g_hash_table_foreach( Storage, WriteToFile, NULL );

    UnlockR();
}

//-------------------------------------------------------------------------
//--- Служебная функция дедупликации
static void ModifyDouble( gpointer aKey, gpointer aValue, gpointer aUd)
{
    ASSERT_PAR_R_VOID( aKey );
    ASSERT_PAR_R_VOID( aValue );
    GHashTableIter iter;
    gpointer Key, Value;

    char * StrIn,
        * StrIter;
    T_Container * ContIn,
        * ContIter;

    StrIn = ( char * )aKey;
    ContIn = ( T_Container * )aValue;

    g_hash_table_iter_init (&iter, Storage);
    while ( g_hash_table_iter_next( &iter, &Key, &Value ) )
    {
        StrIter = ( char * )Key;
        ContIter = ( T_Container * )Value;

        if ( !strcmp( StrIn, StrIter ) ) {       // Keys Match, continue
            continue;
        }

        if ( ContIter->Copies > 1 ) {
            /* this copy contains the original of the value
             * no change needed
             */
            continue;
        }

        // searching for matching values
        if ( !strcmp( ContIter->Str, ContIn->Str ) ) {  // found duplicated values
            // replace with a correct pointer, memory deletion is automated
            g_hash_table_replace( Storage, StrIter, ContIn );
            ContIn->Copies++;
        }
    }
}
//-------------------------------------------------------------------------
// Служебная функция добавления значения в строку
static void AddToStr( gpointer aKey, gpointer aValue, gpointer aUd )
{
    ASSERT_PAR_R_VOID( aKey );
    ASSERT_PAR_R_VOID( aUd );
    char * Key;
    Key = ( char * )aKey;
    char * OutStr = ( char * )aUd;
    strcat( OutStr, Key );
    strcat( OutStr, "\n" );
}

//-------------------------------------------------------------------------
// Служебная функция освобождения памяти из-под ключа
static void FreeKey(gpointer aData)
{
    if ( aData ) {
        free(aData);
    }
    aData = NULL;
}

//-------------------------------------------------------------------------
// Служебная функция освобождения памяти из-под контейнера
static void FreeContainer(gpointer aData)
{
    if ( !aData ) {
        return;
    }
    T_Container * Container = ( T_Container *) aData;
    if ( Container->Copies > 1 ) {
        Container->Copies--;
        return;
    }
    if ( Container->Str ) {
        free(Container->Str);
    }
    free( Container );
}

//-------------------------------------------------------------------------
// Служебная функция создания контейнера
static int CreateContainer( char * aStr, T_Container **aContainer )
{
    ASSERT_PAR_R( aStr );
    ASSERT_PAR_R( aContainer );
    *aContainer = ( T_Container * )calloc( 1, sizeof( T_Container ) );
    (*aContainer)->Str = calloc( 1, strlen( aStr ) + 1 );
    strcpy( (*aContainer)->Str, aStr );
    (*aContainer)->Copies = 1;

    return ERR_OK;
}

//-------------------------------------------------------------------------
// Служебная функция загрузки данных в хранилище из файла
static int LoadFromFile()
{
    int SectionNum = 0;
    char SectionName[512];
    char KeyName[512];
    char KeyKey[10];
    const char KeyQuanStr[] = "KeyQuan";
    const char KeyFormatStr[] = "Key_%u";
    long KeyQuan;

    while ( ini_getsection( SectionNum++, SectionName, 512, FileName ) ) {
        KeyQuan = ini_getl( SectionName, KeyQuanStr, 0, FileName );
        for ( int i = 1; i <= KeyQuan; i++ ) {
            snprintf( KeyKey, 10, KeyFormatStr, i );
            if ( ini_gets( SectionName, KeyKey, "", KeyName, 512, FileName ) ) {
                st_put( KeyName, SectionName );
            }
        }
    }
}

//-------------------------------------------------------------------------
// Init storage
int st_init()
{
    // Block Init
    pthread_mutex_init( &ReaderBlock, NULL );
    pthread_mutex_init( &AccessBlock, NULL );
    pthread_mutex_init( &OrderBlock, NULL );

    // Hash table init, no blocking needed
    Storage = g_hash_table_new_full( g_str_hash, g_str_equal, NULL, FreeContainer );
//    Storage = g_hash_table_new_full( g_str_hash, g_str_equal, FreeKey, FreeContainer );
    LoadFromFile();

    return ERR_OK;
}

//-------------------------------------------------------------------------
int st_list(char *aOutString, size_t aStrLength)
{
    ASSERT_PAR_R( aOutString );
    ASSERT_PAR_R( aStrLength );
    memset( aOutString, '\0', aStrLength );
    LockR();
    g_hash_table_foreach( Storage, AddToStr, (gpointer)aOutString );
    UnlockR();
    if ( strlen( aOutString ) ) {
        aOutString[strlen(aOutString) - 1] = '\0';
    }
    return ERR_OK;
}

//-------------------------------------------------------------------------
// Put record in the storage
int st_put(char *aKey, char *aValue)
{
    ASSERT_PAR_R( aKey );
    ASSERT_PAR_R( aValue );
    LockW();

    T_Container * Container;
    CreateContainer( aValue, &Container );
    char *Key = ( char * )calloc( 1, strlen(aKey) + 1);
    strcpy( Key, aKey );

    g_hash_table_insert( Storage, Key, Container );
    WriteToFile( aKey, Container, NULL );
    UnlockW();
    return ERR_OK;
}

//-------------------------------------------------------------------------
// Get record from the storage by key
int st_get(const char *aKey, char *aValue, uint32_t aValueSize)
{
    ASSERT_PAR_R( aKey );
    ASSERT_PAR_R( aValue );
    LockR();
    gpointer Value;
    if ( g_hash_table_lookup_extended( Storage, aKey, NULL, &Value ) ) {
        strcpy( aValue, ((T_Container *)Value )->Str );
        UnlockR();
        return ERR_OK;
    }
    else {
        UnlockR();
        return ERR_NOT_FOUND;
    }
}

//-------------------------------------------------------------------------
// Erase record from the storage by key
int st_erase(const char *aKey)
{
    ASSERT_PAR_R( aKey );
    LockW();
    g_hash_table_remove( Storage, aKey );
    UnlockW();

    serialize_full();

    return ERR_OK;
}

//-------------------------------------------------------------------------
// Clean
int st_clean()
{
    // destroying semaphores
    pthread_mutex_destroy( &ReaderBlock );
    pthread_mutex_destroy( &OrderBlock );
    pthread_mutex_destroy( &AccessBlock );

    g_hash_table_destroy( Storage );
    return ERR_OK;
}

//-------------------------------------------------------------------------
// Deduplicate
int st_deduplicate()
{
    LockW();

    g_hash_table_foreach( Storage, ModifyDouble, NULL );

    UnlockW();

    serialize_full();

    return ERR_OK;
}
