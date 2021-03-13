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


void *chat_receive_and_refresh(void *arg)
{
    struct chat_refresh_thread_arg *myarg = (struct chat_refresh_thread_arg *)arg;
    WINDOW *chat_win = myarg->chat_win;
    int sock = myarg->sock;
    int chat_line_counter = myarg->chat_line_counter;

    char recvmsgbuffer[MESSAGE_BUFFER_SIZE]; //1K message buffer
    int bytesrecv;

    while (1) {
        if ((bytesrecv = recv(sock, recvmsgbuffer, sizeof(recvmsgbuffer), 0))== -1) {
            perror("recv failed");
            exit(1);
        }

        struct chat_message recv_message;
        strncpy(recv_message.username, recvmsgbuffer, sizeof(recv_message.username));
        //offset the source by the length of the username buffer
        strncpy(recv_message.message, recvmsgbuffer + sizeof(recv_message.username), sizeof(recv_message.message));
        strncpy(recv_message.color, recvmsgbuffer + sizeof(recv_message.username)+sizeof(recv_message.message),
            sizeof(recv_message.color));

        // recvmsgbuffer[sizeof(struct chat_message)+2] = '\0';

        print_to_chat(chat_win, recv_message.username, recv_message.message, &chat_line_counter,
                      atoi(recv_message.color));
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

    //setup panels
//    WINDOW *my_wins[2];
    PANEL *my_panels[3]; //chat and game
    //start curses
    initscr();
    raw();                //no stdin buffering
    noecho();             //no stdin echoing
    keypad(stdscr, TRUE); //enable fn and arrow keys
    my_startcolor();

    int terminal_height, terminal_width;
    getmaxyx(stdscr, terminal_height, terminal_width);

    WINDOW *msg_win = create_newwin(MSG_HEIGHT, terminal_width, terminal_height - MSG_HEIGHT, 0);
    WINDOW *chat_win = create_newwin(terminal_height - MSG_HEIGHT, terminal_width, 0, 0);
//    WINDOW *game = create_newwin(terminal_height, terminal_width, 0, 0);
//
//    my_panels[0] = new_panel(msg_win);
//    my_panels[1] = new_panel(chat_win);
//    my_panels[2] = new_panel(game);




    idlok(msg_win,TRUE);
    idlok(chat_win,TRUE);
    scrollok(chat_win, TRUE);
    scrollok(msg_win, TRUE);



    char yourmsg[MESSAGE_BUFFER_SIZE - MAX_USERNAME_LENGTH]; //ex. 1024-30=994
    // int chat_line_counter = 1;

    //create refresh threading
    //TODO: error check pthread_create
    struct chat_refresh_thread_arg myarg;
    myarg.chat_win = chat_win;
    myarg.sock = sock;
    myarg.chat_line_counter = 1;

    pthread_create(&chat_refresh_thread_id, NULL, chat_receive_and_refresh, (void *)&myarg);

    wattron(msg_win, COLOR_PAIR(1));
    wattron(msg_win, A_BOLD);
    mvwprintw(msg_win, 1, 1, "%s:", username);
    wattroff(msg_win, COLOR_PAIR(1));
    wattroff(msg_win, A_BOLD);

    while (1) {
        echo();
        //move cursor after name and get message
        //string length no more than name + colon - 4
//        mvwgetnstr(msg_win, 1, strlen(username) + 2, yourmsg, terminal_width - strlen(username) + 1 - MSG_BOX_PADDING);
        mvwgetnstr(msg_win, 1, strlen(username) + 2, yourmsg, 1000);
        noecho();
        clear_msg_window(msg_win, username);

        //TODO: make this be sent as a char buffer, not a struct
        struct chat_message cur_msg;
        memset(&cur_msg, 0, sizeof(cur_msg));
        //initialize this struct cur_msg which will be sent
        strncpy(cur_msg.username, username, sizeof(cur_msg.username));
        strncpy(cur_msg.message, yourmsg, sizeof(cur_msg.message));

        if (send(sock, (void *)&cur_msg, sizeof(cur_msg), 0) == -1) {
            perror("send failed");
            exit(1);
        }

    }
    

    pthread_cancel(chat_refresh_thread_id);
    pthread_join(chat_refresh_thread_id, NULL);
    delwin(chat_win);
    delwin(msg_win);
    //end curses
    endwin();
    close(sock);
    return 0;
}
