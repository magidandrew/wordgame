#include "server.h"
#include "chat_message.h"
#include "globals.h"
#include "mylist.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h> //TODO: DO I NEED THIS?
#include <ctype.h>

int letterValues[26] = {LETTER_A, LETTER_B, LETTER_C, LETTER_D, LETTER_E, LETTER_F, LETTER_G, LETTER_H, LETTER_I,
                        LETTER_J, LETTER_K, LETTER_L, LETTER_M, LETTER_N, LETTER_O, LETTER_P, LETTER_Q, LETTER_R,
                        LETTER_S, LETTER_T, LETTER_U, LETTER_V, LETTER_W, LETTER_X, LETTER_Y, LETTER_Z};

int compareStringNodeData(const void *word1, const void *word2)
{
    return strcmp((char *)word1, (char *)word2);
}

void freeUsedWords(struct List *list)
{
    while (!isEmptyList(list)) {
        free(popFront(list));
    }
}

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

int wordToScore(char *word)
{
    int score = 0;
    int len = strlen(word);
    for(int i=0; i<len; i++){
        //convert ascii to 0-25 and lookup that letter value if it's a letter
        if(isalpha(word[i])) {
            score += letterValues[(int) (word[i] - 97)];
        }
    }
    return score;
}

char randomLetter(){
    return (char) (rand()%26) + 97;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: %s <port>\n", argv[0]);
        exit(1);
    }

    //setup for game
    puts("Parsing dictionary and initialising word lookup...");
    FILE *dictionary = fopen("../allnouns.txt", "r");
    if (dictionary == NULL){
        perror("dictionary not found");
        exit(1);
    }

    //populate dictionary
    struct List allwords;
    initList(&allwords);
    char dictionaryBuf[100];
    while (fgets(dictionaryBuf, sizeof(dictionaryBuf), dictionary) != NULL){
        sanitize(dictionaryBuf);
        char *word = (char *) malloc(sizeof(char) * strlen(dictionaryBuf) + 1);
        strcpy(word, dictionaryBuf);
        //note: addBack too damn expensive
        addFront(&allwords, (void *) word);
    }

    struct List usedwords;
    initList(&usedwords);

    //array with all scores
    int scores[MAX_CLIENTS];


    //server setup
    int opt = 1; //set to TRUE
    int master_socket, addrlen, new_socket, client_socket[MAX_CLIENTS],
            max_clients = MAX_CLIENTS, activity, bytesrecv, sd;
    int max_sd;
    int port = atoi(argv[1]);
    struct sockaddr_in address;
    char client_username[MAX_USERNAME_LENGTH];

    char buffer[MESSAGE_BUFFER_SIZE]; //data buffer of 1K

    //set of socket descriptors
    fd_set playerfds;

    //a message
    // char *weclome_message = "Welcome to wordgame by Andrew. Waiting in lobby...\n";

    //init all client_socket[] to 0
    memset((int *)&client_socket, 0, sizeof(client_socket));

    //create a master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(1);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0) {
        perror("setsockopt failure");
        exit(1);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    //bind the socket to localhost the given port
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(1);
    }
    log_time_server();
    printf("Listener on port %d\n", port);

    //try to specify maximum of 3 pending connections for the master socket
    //TODO: adjust size for more than 3 connections
    if (listen(master_socket, 3) == -1) {
        perror("listen failed");
        exit(1);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    log_time_server();
    puts("Waiting for connections ...");

    while (1) {
        //clear the socket set
        FD_ZERO(&playerfds);

        //add master socket to set
        FD_SET(master_socket, &playerfds);
        max_sd = master_socket;

        //add child sockets to set
        for (int i = 0; i < max_clients; i++) {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if (sd > 0) {
                FD_SET(sd, &playerfds);
            }

            //highest file descriptor number, need it for the select function
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select(max_sd + 1, &playerfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &playerfds)) {
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("accept failure");
                exit(1);
            }

            //add new socket to array of sockets
            for (int i = 0; i < max_clients; i++) {
                //if position is empty
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;

                    //recieve username from client
                    if ((bytesrecv = recv(new_socket, buffer, sizeof(buffer), 0)) == -1) {
                        perror("recv failed");
                    }

                    strncpy(client_username, buffer, MAX_USERNAME_LENGTH);

                    log_time_server();
                    //log new connection
                    printf("New connection-->name:{%s}, socket no: %d, socket fd: %d, ip: %s, port: %d\n", client_username,
                           i, new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    break;
                }
            }
        }

        //else it's some IO operation on some other socket
        for (int i = 0; i < max_clients; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &playerfds)) {
                //Check if it was for closing, and also read the incoming message
                if ((bytesrecv = recv(sd, buffer, sizeof(buffer), 0)) == 0) {
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    log_time_server();
                    //TODO: implement functions that alert everyone when someone has joined or left the chat
                    printf("Disconnected-->name:{%s}, socket no: %d, socket fd: %d, ip: %s, port: %d\n", client_username,
                           i, sd, inet_ntoa(address.sin_addr), ntohs(address.sin_port));


                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                }

                    //work with message
                else {

                    } //end for-loop

                }
            }

        //game logic
        int currentPlayer;

        for (int i = 0; i < max_clients; i++) {
            if (client_socket[i] != 0 && client_socket[i] != currentPlayer) {
                //pick a new player
                currentPlayer = client_socket[i];
            }
        }

        snprintf(buffer, sizeof(buffer), "ello love");
        puts("trying to send message");
        if (send(currentPlayer, buffer, sizeof(buffer), 0) == -1) {
            fprintf(stderr, "send failed to socket\n");
            exit(1);
        }

    }

    //free used words
    removeAllNodes(&usedwords);
    freeUsedWords(&allwords);

    //close file descriptor
    fclose(dictionary);
}


