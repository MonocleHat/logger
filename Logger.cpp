#include <iostream>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "Logger.h"
using namespace std;
#define PORT 9999
#define BUFLEN 4096
#define IP "127.0.0.1" // change me
// Globals
LOG_LEVEL severity;
int sockfd, len;
struct sockaddr_in listener;
socklen_t listener_len;
pthread_mutex_t logLock;
pthread_t logthread;
bool is_running;

void *recv_func(void *arg);
int InitializeLog()
{
    // create socket
    // set address and port of server
    memset(&listener, 0, sizeof(listener));
    listener.sin_addr.s_addr = inet_addr(IP);
    listener.sin_port = htons(PORT);
    listener.sin_family = AF_INET;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    listener_len = sizeof(listener);
    bind(sockfd, (struct sockaddr *)&listener, sizeof(listener));
    pthread_mutex_init(&logLock, NULL);
    is_running = true;
    pthread_create(&logthread, NULL, recv_func, &sockfd); //start the thread
    return 0;
}

void SetLogLevel(LOG_LEVEL level)
{
   
    severity = level;
    
}

void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message)
{
    
    if (level == severity){ //if the passed level of the message is == to our current severity, log message to server
    pthread_mutex_lock(&logLock);
  
    char BUF[BUFLEN];
    int len;
    time_t now = time(0);
    char *dt = ctime(&now);
    memset(BUF, 0, BUFLEN);
    char levelStr[][16] = {"DEBUG", "WARNING", "ERROR", "CRITICAL"};
    len = sprintf(BUF, "%s %s %s:%s:%d %s\n", dt, levelStr[level], prog, func, line, message) + 1; //construct message
    BUF[BUFLEN - 1] = '\0';
    sendto(sockfd, BUF, len, 0, (struct sockaddr *)&listener, sizeof(listener)); //send message
    pthread_mutex_unlock(&logLock);
    }

}

void ExitLog()
{
    is_running = false;
    close(sockfd);
}

void *recv_func(void *arg)
{
    struct timeval to;
    to.tv_sec = 1;
    to.tv_usec = 0;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&to, sizeof(to)); //create a socket option timeout
    char BUF[BUFLEN];
    int ret, len;
    int servSock = *(int *)arg;
    LOG_LEVEL levelSet = DEBUG;
    int contain=0;
    while (is_running)
    {
        pthread_mutex_lock(&logLock);
        recvfrom(servSock,BUF,BUFLEN,0,(struct sockaddr*)&listener,&listener_len)-1; //listen for server messages and set the severity accordingly
          
            std::string newBuf(BUF);
            string found;
            std::string::size_type sz;
            std::size_t finder = newBuf.find_first_of("0123");
            bool findFLAG = false;
            while(finder!=std::string::npos && !findFLAG){ //if 0123 is found in the passed data, 
                found = newBuf[finder]; //assign the number to a container string
                contain = atoi(found.c_str()); //convert container to an int
                levelSet = (LOG_LEVEL)contain; //cast and set level to the contained int
                SetLogLevel(levelSet);//set level
                findFLAG = true;//to break the loop CRITICAL DO NOT REMOVE
            }
            
        
        pthread_mutex_unlock(&logLock); //unlock and sleep
        sleep(1);
    }
    pthread_exit(NULL);
}

