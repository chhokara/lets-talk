#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include "list.c"
#include "list.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#define AI_PASSIVE 0x0001

pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
char buffer[1000];
struct thread_params {
    struct sockaddr * receiverinfo;
    struct sockaddr * senderinfo;
    List * sending_list;
    List * receiving_list;
};

void *receive_msg(void * ptr) {
    struct thread_params *params = ptr;
    do {
        pthread_mutex_lock(&Mutex);
        char buff[1024];
        int n;
        socklen_t len; 
  
        len = sizeof(params->cliaddr);  //len is value/resuslt 
  
        n = recvfrom(params->clisockfd, (char *)buff, 1024,  
                MSG_WAITALL, ( struct sockaddr *) &(params->cliaddr), 
                    &len); 
        buff[n] = '\0'; 
        List_add(params->receiving_list, (char *)buff);
        pthread_mutex_unlock(&Mutex);
    } while(1);
    
    // printf("receiving thread\n");
}
void *print_msg(void * ptr) {
    struct thread_params *params = ptr;
    do {
        pthread_mutex_lock(&Mutex);
        if(List_count(params->receiving_list)) {
            char * msg = List_remove(params->receiving_list);
            printf("%s", msg);
        }
        pthread_mutex_unlock(&Mutex);
    }while(1);
    
}
void *send_msg(void * ptr) {
    struct thread_params *params = ptr;
    do {
        pthread_mutex_lock(&Mutex);
        if(List_count(params->sending_list)) {
            char * msg = List_remove(params->sending_list);
            sendto(params->servsockfd, (const char *)msg, strlen(msg), 
                MSG_CONFIRM, (const struct sockaddr *) &(params->servaddr),  
                    sizeof(params->servaddr));
            close(params->servsockfd);  
        }
        pthread_mutex_unlock(&Mutex);
    } while(1);
    
}
void *input_msg(void * ptr) {
    struct thread_params *params = ptr;
    printf("Welcome to lets-talk! Please type your message now\n");
    do {
        pthread_mutex_lock(&Mutex);
        if(fgets(buffer, 1000, stdin)){
            List_add(params->sending_list, (char *)buffer);
        }
        pthread_mutex_unlock(&Mutex);
    } while(1);
}

int main (int argc, char ** argv) 
{
    int sender_status;
    int sender_socketfd;
    struct addrinfo sender_hints, *senderinfo;  // will point to the results

    memset(&sender_hints, 0, sizeof sender_hints); // make sure the struct is empty
    sender_hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    sender_hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    // get ready to connect
    sender_status = getaddrinfo(argv[2], argv[1], &sender_hints, &senderinfo);

    sender_socketfd = socket(senderinfo->ai_family, senderinfo->ai_socktype, senderinfo->ai_protocol);
    int receiver_status;
    int receiver_socketfd;
    struct addrinfo receiver_hints, *receiverinfo;  // will point to the results

    memset(&receiver_hints, 0, sizeof receiver_hints); // make sure the struct is empty
    receiver_hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    receiver_hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    if ((receiver_status = getaddrinfo(argv[2], argv[3], &receiver_hints, &receiverinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(receiver_status));
        exit(1);
    }

    receiver_socketfd = socket(receiverinfo->ai_family, receiverinfo->ai_socktype, receiverinfo->ai_protocol);

    // bind it to the port we passed in to getaddrinfo():
    bind(receiver_socketfd, receiverinfo->ai_addr, receiverinfo->ai_addrlen);

    List * send_list;
    send_list = List_create();
    
    List * receive_list;
    receive_list = List_create();

    struct thread_params params;
    params.senderinfo = senderinfo->ai_addr;
    params.receiverinfo = receiverinfo->ai_addr;
    params.sending_list = send_list;
    params.receiving_list = receive_list;

    pthread_t receiving_thr, printer_thr, sending_thr, keyboard_thr;

    //create threads for receiving and printing messages
    pthread_create(&keyboard_thr, NULL, input_msg, &params);
    pthread_create(&sending_thr, NULL, send_msg, &params);
    pthread_create(&receiving_thr, NULL, receive_msg, &params);
    pthread_create(&printer_thr, NULL, print_msg, &params);

    pthread_join(keyboard_thr, NULL);
    pthread_join(sending_thr, NULL);
    pthread_join(receiving_thr, NULL);
    pthread_join(printer_thr, NULL);

    return 0;
}