#include "chat_message.h"
#include "chat_refresh_thread_arg.h"
#include "color_init.h"
#include "globals.h"
#include "gui.h"
#include "server.h"
#include <arpa/inet.h>
#include <errno.h>
#include <panel.h>
#include <ncurses.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h> //TODO: check which packages i need and don't
#include <time.h>
#include <ctype.h>


void sanitize(char *word)
{
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
        word[i] = tolower(word[i]);
        if (isspace(word[i]) != 0) {
            word[i] = '\0';
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    //networking
    // const char *ip = argv[2];
    const char *ip = hostToIP(argv[1]);
    unsigned short port = atoi(argv[2]);

    //multithreading for chat receiving and printing messages
    pthread_t chat_refresh_thread_id;

    // Create a socket for TCP connection

    int sock; // socket descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(1);
    }

    // Construct a server address structure

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr)); // must zero out the structure
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port); // must be in network byte order

    // Establish a TCP connection to the server
    if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect failed");
        exit(1);
    }

    // char recvmsgbuffer[MESSAGE_BUFFER_SIZE]; //1K message buffer
    // int bytesrecv;

    //username info
    char username[MAX_USERNAME_LENGTH];
    // char *username = (char *)malloc(sizeof(char) * MAX_USERNAME_LENGTH);
    printf("Enter your username. Max %d characters: ", MAX_USERNAME_LENGTH);
    memset(username, 0, sizeof(username)); //FIXME: Why do i need memset here??
    fgets(username, sizeof(username), stdin);
    //replace newline with null terminator
    username[strlen(username) - 1] = '\0';
    // free(username);

    //send username to server
    if (send(sock, username, sizeof(username), 0) == -1) {
        fprintf(stderr, "send failed to socket\n");
        exit(1);
    }

    //setup vars
    int bytesrecv;
    char recvmsgbuffer[MESSAGE_BUFFER_SIZE];
    char newword[100];
    //END SETUP. GAME BEGINS.
    while(1){
        //request from server to see who's turn it is

        if ((bytesrecv = recv(sock, recvmsgbuffer, sizeof(recvmsgbuffer), 0)) == -1) {
            perror("recv failed");
            exit(1);
        }

        //one is player
        if(recvmsgbuffer[0] == '1'){
            printf("Find a noun that starts with the letter \'%c\': ", recvmsgbuffer[strlen(recvmsgbuffer) - 1]);
            //get input from user
            fgets(newword, sizeof(newword), stdin);
            sanitize(newword);

            //send input to server
            if (send(sock, newword, sizeof(newword), 0) == -1) {
                fprintf(stderr, "send failed to socket\n");
                exit(1);
            }

            //another attempt or confirmation
            if ((bytesrecv = recv(sock, recvmsgbuffer, sizeof(recvmsgbuffer), 0)) == -1) {
                perror("recv failed");
                exit(1);
            }

            //print to screen
            printf("%s", recvmsgbuffer + 1);
        }

        //two is spectator
        else if(recvmsgbuffer[0] == '2'){
            //receive word or score
//            if ((bytesrecv = recv(sock, recvmsgbuffer, sizeof(recvmsgbuffer), 0)) == -1) {
//                perror("recv failed");
//                exit(1);
//            }
            printf("%s", recvmsgbuffer + 1);
        }

        //+1 to make up for user code
//        printf("\\33[2K" "%s", (recvmsgbuffer + 1));

        else{
            break;
        }
    }

    close(sock);
    return 0;
}
