#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <zconf.h>
#include <stdint.h>


// Имя сокета, которому необходимо слать запросы
const char SocketName[] = "SocketName";

// Функция вывода корректного формата аргументов при их некорректном вводе
static int PrintCorrectFormat()
{
    printf( "Commands:\nput <key> <value>\nlist\nget <key>\nerase <key>\n" );
}

int main(int argc, char *argv[] )
{
    int sock;
    struct sockaddr_un server;
    char SendBuffer[512];
    struct timeval tv;

    if ( argc < 2 ) {
        PrintCorrectFormat();
        return 0;
    }

    sock = socket( AF_UNIX, SOCK_STREAM, 0 );
    if ( sock < 0 ) {
        perror( "opening socket" );
        return -1;
    }
    // Установка неблокирующего ожидания ответа от сервера для сокета
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof( struct timeval ) );

    server.sun_family = AF_UNIX;
    strcpy( server.sun_path, SocketName );

    if ( connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        close(sock);
        perror( "connection to socket" );
        return -2;
    }

    memset( SendBuffer, '\0', 512 );
    // разбор поступивших на вход параметров
    char CommandStr[5];
    // Копирование первого параметра, должна быть команда
    strncpy( CommandStr, argv[ 1 ], 50 );
         if ( !strncasecmp( CommandStr, "list",  4 ) )
         {
             strcpy( SendBuffer, "list" );
         }
    else if ( !strncasecmp( CommandStr, "put",   3 ) )
         {
             if ( argc != 4 ) {
                 PrintCorrectFormat();
                 close(sock);
                 return 0;
             }
             strcat( SendBuffer, "put " );
             strcat( SendBuffer, argv[ 2 ] );
             strcat( SendBuffer, " " );
             strcat( SendBuffer, argv[ 3 ] );
         }
    else if ( !strncasecmp( CommandStr, "get",   3 ) )
         {
             if ( argc != 3 ) {
                 PrintCorrectFormat();
                 close(sock);
                 return 0;
             }
             strcat( SendBuffer, "get " );
             strcat( SendBuffer, argv[ 2 ] );
         }
    else if ( !strncasecmp( CommandStr, "erase", 5 ) ) {
             if ( argc != 3 ) {
                 PrintCorrectFormat();
                 close(sock);
                 return 0;
             }
             strcat( SendBuffer, "erase " );
             strcat( SendBuffer, argv[ 2 ] );
         }
    else {
             PrintCorrectFormat();
             close(sock);
             return 0;
         }


    if ( write(sock, SendBuffer, strlen(SendBuffer) ) < 0 ) {
        perror( "send error" );
    }

    memset( SendBuffer, '\0', 512 );
    read( sock, SendBuffer, 512 );
    if (strlen( SendBuffer )) {
        printf("%s\n", SendBuffer);
    }

    close(sock);

    return 0;
}
