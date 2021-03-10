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


char buffer[1000];
struct thread_params {
    List * sending_list;
    List * receiving_list;
    int argc;
    char ** argv;
};

void *receive_msg(void * ptr) {
    // struct thread_params *params = ptr;
    // printf("%d\n", List_count(params->receiving_list));
    // printf("%d\n", atoi(params->argv[1]));
    // int sockfd;
    // char buffer[1024];
    // struct sockaddr_in servaddr, cliaddr;

    // // Creating socket file descriptor 
    // if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
    //     perror("socket creation failed"); 
    //     exit(EXIT_FAILURE); 
    // } 
      
    // memset(&servaddr, 0, sizeof(servaddr)); 
    // memset(&cliaddr, 0, sizeof(cliaddr)); 

    // // Filling server information 
    // servaddr.sin_family    = AF_INET; // IPv4 
    // servaddr.sin_addr.s_addr = INADDR_ANY; 
    // servaddr.sin_port = htons(atoi(params->argv[1])); 
      
    // // Bind the socket with the server address 
    // if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
    //         sizeof(servaddr)) < 0 ) 
    // { 
    //     perror("bind failed"); 
    //     exit(EXIT_FAILURE); 
    // } 
      
    // int len, n; 
  
    // len = sizeof(cliaddr);  //len is value/resuslt 
  
    // n = recvfrom(sockfd, (char *)buffer, 1024,  
    //             MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
    //             &len); 
    // buffer[n] = '\0'; 

    // List_add(params->receiving_list, (char *)buffer);
    printf("receive\n");
}
void *print_msg(void * ptr) {
    printf("print\n");
}
void *send_msg(void * ptr) {
    struct thread_params *params = ptr;
    do {
        char * msg = List_remove(params->sending_list);
        printf("%s", msg);
    } while(true);
    
}
void *input_msg(void * ptr) {
    struct thread_params *params = ptr;
    printf("Welcome to lets-talk! Please type your message now\n");
    do {
        if(fgets(buffer, 1000, stdin)){
            List_add(params->sending_list, (char *)buffer);
        }
    } while(true);
}

int main (int argc, char ** argv) 
{
    List * send_list;
    send_list = List_create();
    
    List * receive_list;
    receive_list = List_create();


    struct thread_params params;
    params.argc = argc;
    params.argv = argv;
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