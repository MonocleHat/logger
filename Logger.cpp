#include <iostream>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "Logger.h"
using namespace std;
#define PORT 9999
#define BUFLEN 512
#define IP "127.0.0.1" // change me
//Globals
LOG_LEVEL severity;
int sockfd, len; 
struct sockaddr_in listener;
pthread_mutex_t logLock;
pthread_t logthread;
bool is_running;

void *recv_func(void *arg);
int InitializeLog(){
    //create socket
    memset(&listener, 0,sizeof(listener));
    listener.sin_addr.s_addr = inet_addr(IP);
    listener.sin_port = htons(PORT);
    listener.sin_family = AF_INET;
    sockfd = socket(AF_INET,SOCK_DGRAM,0);

    bind(sockfd,(struct sockaddr*)&listener,sizeof(listener));
    pthread_mutex_init(&logLock, NULL);
    //set address and port of server
    //create pthread_mutex_t
    //start rec thread pass file descriptor to it
    is_running = true;
    pthread_create(&logthread, NULL, recv_func, &sockfd);
}

void SetLogLevel(LOG_LEVEL level){
    severity = level; 
}

void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message){
    char BUF[BUFLEN];
    int len;
    time_t now = time(0);
    char *dt = ctime(&now);
    memset(BUF,0,BUFLEN);
    char levelStr[][16]={"DEBUG","WARNING","ERROR","CRITICAL"};
    len = sprintf(BUF, "%s %s %s:%s:%d %s\n",dt,levelStr[level],prog,func,line,message)+1;
    BUF[BUFLEN-1]='\0';
    sendto(sockfd,BUF,len,0,(struct sockaddr*)&listener,sizeof(listener));
}

void ExitLog(){
    is_running = false;
}

void *recv_func(void *arg){
    char BUF[BUFLEN];
    int ret, len;
    int servSock = *(int *)arg;
    while(is_running){
        
    }

}