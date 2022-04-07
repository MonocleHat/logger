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
#define BUFLEN 1024
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
    memset(&listener, 0, sizeof(listener));
    listener.sin_addr.s_addr = inet_addr(IP);
    listener.sin_port = htons(PORT);
    listener.sin_family = AF_INET;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    listener_len = sizeof(listener);
    bind(sockfd, (struct sockaddr *)&listener, sizeof(listener));
    pthread_mutex_init(&logLock, NULL);
    // set address and port of server
    // create pthread_mutex_t
    // start rec thread pass file descriptor to it
    is_running = true;
    pthread_create(&logthread, NULL, recv_func, &sockfd);
    return 0;
}

void SetLogLevel(LOG_LEVEL level)
{
    severity = level;
}

void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message)
{
    char BUF[BUFLEN];
    int len;
    time_t now = time(0);
    char *dt = ctime(&now);
    memset(BUF, 0, BUFLEN);
    char levelStr[][16] = {"DEBUG", "WARNING", "ERROR", "CRITICAL"};
    len = sprintf(BUF, "%s %s %s:%s:%d %s\n", dt, levelStr[level], prog, func, line, message) + 1;
    BUF[BUFLEN - 1] = '\0';
    sendto(sockfd, BUF, len, 0, (struct sockaddr *)&listener, sizeof(listener));
}

void ExitLog()
{
    is_running = false;
}

void *recv_func(void *arg)
{
    struct timeval to;
    to.tv_sec = 1;
    to.tv_usec = 0;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&to, sizeof(to));
    char BUF[BUFLEN];
    int ret, len;
    int servSock = *(int *)arg;
    int levelSet;
    while (is_running)
    {
        recvfrom(servSock,BUF,BUFLEN,0,(struct sockaddr*)&listener,&listener_len);
        pthread_mutex_lock(&logLock);
        if(strncmp("Set Log Level=",BUF,BUFLEN+1)==0){
            std::string newBuf(BUF);
            string found;
            std::string::size_type sz;
            std::size_t finder = newBuf.find_first_of("0123");
            
            while(finder!=std::string::npos){
                found = newBuf[finder];
                levelSet = atoi(found.c_str());
                cout << "DEBUG::LEVEL::" << levelSet << " | found:: " << found << endl;
            }
        }
        pthread_mutex_unlock(&logLock);
    }
    pthread_exit(NULL);
}
