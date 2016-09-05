#include <stdio.h>
#include "storage.h"
#include "macros.h"
#include <pthread.h>
#include <zconf.h>
#include <signal.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>

// Флаг для выхода по SIGINT
uint8_t Exit = 0;
// Дескриптор сокета
int sock;
// Имя сокета
const char SocketName[] = "SocketName";
// Максимальное количество соединений
#define MAX_CONNECTIONS 5

// Функция разбора строки, поступившей от клиента
static int ParseInString( char * InString, char * OutString );
// Функция обработки сигнала SIGINT
void SigIntHandler( int a );
// Функция потока дедупликации
void * DeduplicateThreadBody( void * a );
// Функция потока обработки запросов от клиентов
void * WorkThreadBody( void * a );

// Функция разбора строки, поступившей от клиента
static int ParseInString( char * InString, char * OutString )
{
    char CommandStr[ 10 ];
    char Arg1[512];
    char Arg2[512];

    sscanf( InString, "%s %s", CommandStr, Arg1 );

    if ( !strncasecmp( CommandStr, "list",  4 ) ) {
        st_list( OutString, 512 );
    }
    else if ( !strncasecmp( CommandStr, "put",   3 ) ) {
        sscanf( InString, "%s %s %s", CommandStr, Arg1, Arg2 );
        st_put( Arg1, Arg2 );
    }
    else if ( !strncasecmp( CommandStr, "get",   3 ) ) {
        sscanf( InString, "%s %s", CommandStr, Arg1 );
        st_get( Arg1, OutString, 512 );
    }
    else if ( !strncasecmp( CommandStr, "erase", 5 ) ) {
        sscanf( InString, "%s %s", CommandStr, Arg1 );
        st_erase( Arg1 );
    }
    else {
        strcat( OutString, "BadQuery\n" );
    }
    return ERR_OK;
}

// Функция обработки сигнала SIGINT
void SigIntHandler( int a )
{
    Exit = 1;
}

// Функция потока дедупликации
void * DeduplicateThreadBody( void * a )
{
    while (!Exit) {
        st_deduplicate();
        sleep(1);
    }
    return NULL;
}

// Функция потока обработки запросов от клиентов
void * WorkThreadBody( void * a )
{
    char RecvBuffer[512];
    char SendBuffer[512];
    int msgsock;
    ssize_t RecvdBytes;
    struct timespec ts, tsb;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000;

    while ( !Exit ) {
        msgsock = accept( sock, 0, 0 );
        if ( msgsock < 0 ) {
            sleep(1);
            continue;
        }
        memset( RecvBuffer, '\0', 512 );
        RecvdBytes = read( msgsock, RecvBuffer, 512 );
        if ( RecvdBytes < 0 ) {
            perror( "RecvError" );
        }
        else if ( RecvdBytes > 0 ) {
            memset( SendBuffer, '\0', 512 );
            ParseInString( RecvBuffer, SendBuffer );
            write( msgsock, SendBuffer, strlen( SendBuffer ) );
        }
        nanosleep( &ts, &tsb );
    }
}

int main(int argc, char *argv[] )
{
    // Инициализация таблицы, в которой хранятся данные
    st_init();
    signal( SIGINT, SigIntHandler );

    // Создание серверного сокета
    struct sockaddr_un server;

    sock = socket( AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0 );
    if ( sock < 0 ) {
        perror( "Socket Create Fail" );
        close(sock);
        unlink( SocketName );
        return -1;
    }
    server.sun_family = AF_UNIX;
    strcpy( server.sun_path, SocketName );
    if ( bind( sock, (struct sockaddr*)&server, sizeof( struct sockaddr_un ) ) ) {
        perror( "Socket bind Fail" );
        close(sock);
        unlink( SocketName );
        return -2;
    }
    listen( sock, MAX_CONNECTIONS );

    pthread_t DeduplicateThread;
    if ( pthread_create( &DeduplicateThread, NULL, DeduplicateThreadBody, NULL ) ) {
        perror( "Deduplicate thread creation error" );
        close(sock);
        unlink( SocketName );
        return 1;
    }

    pthread_t WorkThreads[ MAX_CONNECTIONS ];
    for ( uint8_t i = 0; i < MAX_CONNECTIONS; i++ ) {
        pthread_create( &WorkThreads[ i ], NULL, WorkThreadBody, NULL );
    }

    for ( uint8_t i = 0; i < MAX_CONNECTIONS; i++) {
        pthread_join( WorkThreads[ i ], NULL );
    }
    pthread_join( DeduplicateThread, NULL );

    st_clean();
    close(sock);
    unlink( SocketName );

    return 0;
}