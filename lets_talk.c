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

pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
bool term_signal = 1;
char buffer[1000];
int encryption_key = 16;
struct thread_params {
    int receiver_socketfd;
    int sender_socketfd;
    struct sockaddr_storage receiveraddr;
    struct addrinfo * sender_servinfo, *sender_p, *receiver_p;
    List * sending_list;
    List * receiving_list;
};

void *receive_msg(void * ptr) {
    struct thread_params *params = ptr;
    int i;
    while(term_signal) {
        char buf[1024];
        socklen_t addr_len;
        int numbytes;
        addr_len = sizeof params->receiveraddr;
        if ((numbytes = recvfrom(params->receiver_socketfd, buf, 1024 , 0,
            (struct sockaddr *)&(params->receiveraddr), &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        buf[numbytes] = '\0';
        
        // for(i = 0; i < strlen(buf); i++) {
        //     if(buf[i] != '\n' && buf[i] != '\0') {
        //         buf[i] -= encryption_key;
        //     }
        // }
        
        List_add(params->receiving_list, (char *) buf);
        if(strcmp(buf, "!exit\n") == 0) {
            term_signal = 0;
        }
        if(strcmp(buf, "!status\n") == 0) {
            int numbytes2;
            char * msg = "Online";
            if ((numbytes2 = sendto(params->receiver_socketfd, msg, strlen(msg), 0,
                (struct sockaddr *)&(params->receiveraddr), addr_len)) == -1) {
                perror("talker: sendto");
                exit(1);
            }
            
        }
        
    }
    pthread_exit(NULL);

}
void *print_msg(void * ptr) {
    struct thread_params *params = ptr;
    bool val = 1;
    while(val) {
        if(List_count(params->receiving_list)) {
            char * msg = List_remove(params->receiving_list);
            printf("%s", msg);
            
        }
    }
    pthread_exit(NULL);

}
void *send_msg(void * ptr) {
    struct thread_params *params = ptr;
    int i;
    while(term_signal) {
        if(List_count(params->sending_list)) {
            char * msg = List_remove(params->sending_list);
            if(strcmp(msg, "!exit\n") == 0) {
                term_signal = 0;
            }
            
            // for(i = 0; i < strlen(msg); i++) {
            //     if(msg[i] != '\n' && msg[i] != '\0') {
            //         msg[i] += encryption_key;
            //     }
            // }
            int numbytes;
            if ((numbytes = sendto(params->sender_socketfd, msg, strlen(msg), 0,
                (params->sender_p)->ai_addr, (params->sender_p)->ai_addrlen)) == -1) {
                printf("in sender\n");
                perror("talker: sendto");
                exit(1);
            }
            // for(i = 0; i < strlen(msg); i++) {
            //     if(msg[i] != '\n' && msg[i] != '\0') {
            //         msg[i] -= encryption_key;
            //     }
            // }
            if(strcmp(msg, "!status\n") == 0) {
                char buf[1024];
                socklen_t addr_len;
                int numbytes2;
                addr_len = (params->sender_p)->ai_addrlen;
                if ((numbytes2 = recvfrom(params->sender_socketfd, buf, 1024 , 0,
                    (params->sender_p)->ai_addr, &addr_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }

                buf[numbytes2] = '\0';
                printf("%s\n", buf);
            }     

            

        }
    }
    pthread_exit(NULL);

}
void *input_msg(void * ptr) {
    struct thread_params *params = ptr;
    printf("Welcome to lets-talk! Please type your message now\n");
    bool val = 1;
    while(val) {
        if(fgets(buffer, 1000, stdin)){
            List_add(params->sending_list, (char *)buffer);
        }
    } 
    pthread_exit(NULL);
}

int main (int argc, char ** argv) 
{
    //RECEIVER
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    struct sockaddr_storage their_addr;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    //SENDER
    int sender_sockfd;
    struct addrinfo sender_hints, *sender_servinfo, *sender_p;
    int sender_rv;

    memset(&sender_hints, 0, sizeof sender_hints);
    sender_hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    sender_hints.ai_socktype = SOCK_DGRAM;

    if ((sender_rv = getaddrinfo(argv[2], argv[3], &sender_hints, &sender_servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(sender_p = sender_servinfo; sender_p != NULL; sender_p = sender_p->ai_next) {
        if ((sender_sockfd = socket(sender_p->ai_family, sender_p->ai_socktype,
                sender_p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (sender_p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }
   
    List * send_list;
    send_list = List_create();
    
    List * receive_list;
    receive_list = List_create();

    struct thread_params params;
    params.receiver_p = p;
    params.receiveraddr = their_addr;
    params.receiver_socketfd = sockfd;
    params.sender_socketfd = sender_sockfd;
    params.sender_servinfo = sender_servinfo;
    params.sender_p = sender_p;
    params.sending_list = send_list;
    params.receiving_list = receive_list;

    pthread_t receiving_thr, printer_thr, sending_thr, keyboard_thr;

    //create threads for receiving and printing messages
    pthread_create(&keyboard_thr, NULL, input_msg, &params);
    pthread_create(&sending_thr, NULL, send_msg, &params);
    pthread_create(&receiving_thr, NULL, receive_msg, &params);
    pthread_create(&printer_thr, NULL, print_msg, &params);

    // wait for a termination signal from one of the threads
    while(term_signal);
    pthread_cancel(keyboard_thr);
    pthread_cancel(sending_thr);
    pthread_cancel(receiving_thr);
    pthread_cancel(printer_thr);


    return 0;
}