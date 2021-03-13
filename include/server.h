#ifndef _SERVER_H_
#define _SERVER_H_
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>

char *hostToIP(char *hostArg)
{
    struct hostent *he;
    char *serverName = hostArg;
    // get server ip from server name
    if ((he = gethostbyname(serverName)) == NULL) {
        perror("gethostname failed");
        exit(1);
    }
    char *serverIP = inet_ntoa(*(struct in_addr *)he->h_addr);
    return serverIP;
}

void log_time_server()
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("[%d-%02d-%02d %02d:%02d:%02d]: ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

#endif
