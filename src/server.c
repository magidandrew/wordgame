#include "server.h"
#include "chat_message.h"
#include "globals.h"
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

#define NAME_COLORS 6

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: %s <port>\n", argv[0]);
        exit(1);
    }

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

                    //send disconnected message to all clients
                    //---
                    //create disconnected message in buffer
                    //FIXME: absolute garbage spaghetti code. this needs to be reworked. im a donkey
                    struct chat_message disconnect_message;
//                    snprintf(buffer, sizeof(buffer), "<%s HAS DISCONNECTED>", client_username);
                    snprintf(disconnect_message.message, sizeof(disconnect_message.message), "<%s HAS DISCONNECTED>", client_username);
                    int sizeofusername = sizeof(disconnect_message.username);
                    int sizeofcolor = sizeof(disconnect_message.color);
                    memset(disconnect_message.username, 0, sizeofusername);
                    memset(disconnect_message.color, 0, sizeofcolor);

                    for (int i = 0; i < max_clients; i++) {
                        if (client_socket[i] != 0) {
                            if (send(client_socket[i], &disconnect_message, sizeof(struct chat_message), 0) == -1) {
                                fprintf(stderr, "send failed to socket\n");
                                exit(1);
                            }
                        }
                    }


                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                }

                //work with message
                else {
                    //this is to get the name and message for logging purposes
                    struct chat_message recv_message;
                    strncpy(recv_message.username, buffer, sizeof(recv_message.username));
                    //offset the source by the length of the username buffer
                    strncpy(recv_message.message, buffer + sizeof(recv_message.username), sizeof(recv_message.message));
                    //end logging

                    //color info
                    char usercolor[sizeof(short)];
                    sprintf(usercolor, "%d",(i%NAME_COLORS)+2);
                    // recv_message.color = (i % 4) + 1; //4 because there are 4 init colors and +1 because colors numbered 1-5
                    strncpy(buffer+sizeof(recv_message.username)+sizeof(recv_message.message), usercolor, sizeof(usercolor));

                    //add color info to buffer
                    //2 places for double-digits users. 0-99
                    //FIXME: adjust buffers to account for the last 2
                    // snprintf(buffer+sizeof(struct chat_message)-2, 2, "%d%c", (i%4)+1, '\0');
                    
                    //send received bytes to all other connections
                    for (int i = 0; i < max_clients; i++) {
                        if (client_socket[i] != 0) {
                            if (send(client_socket[i], buffer, bytesrecv, 0) == -1) {
                                fprintf(stderr, "send failed to socket\n");
                                exit(1);
                            }
                            //print out sent message
                            log_time_server();
                            printf("{%s} sent {%s} to %d\n", recv_message.username, recv_message.message, client_socket[i]);
                        }
                    } //end for-loop
                }
            }
        } //end some io operation
    }
}
