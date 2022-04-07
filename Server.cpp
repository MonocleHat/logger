#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/un.h>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#define IP "127.0.0.1"
#define PORT 9999
#define BUFLEN 4096
using namespace std;

bool is_running;
int readFD, writeFD;
void *recv_func(void *arg);
void fileReader();
pthread_mutex_t lockserv;
pthread_t servthread;
 struct sockaddr_in servaddr;
 socklen_t servlen;
static void shutdownHandler(int sig)
{
    switch (sig)
    {
    case SIGINT:
        is_running = false;
        break;
    }
}

int main()
{
    /* create non blocking UDP socket (AF_INET,SOCK_DGRAM);
     * server's main func binds socket to ip and available port
     * create mutex, appl;y mutexing to shared resources
     */
    int sockfd;
    char BUF[BUFLEN];
    
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(PORT);
    servlen = sizeof(servaddr);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
     struct timeval to;
    to.tv_sec = 1;
    to.tv_usec = 0;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&to, sizeof(to));
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    //Introducing threading
    pthread_mutex_init(&lockserv,NULL);
    pthread_create(&servthread,NULL,recv_func,&sockfd); //create a thread and pass socket fd to it

    //main menu creation
    int opt = -1;
    int loglvl;
    int len;
    bool logFLAG = false;
    is_running = true;
    while(is_running){
        cout << "A2 Server 0.0.4" << endl;
        cout << "-=-=-=-=-=-=-=-" << endl;
        cout << "1. Set Log Level" << endl;
        cout << "2. Dump Logs" << endl;
        cout << "3. Quit" << endl;
        cout << "Selection: ";
        cin >> opt;
        switch (opt){
            case 1:
                while(!logFLAG){
                    cout << "Enter new log level (0-3):";
                    cin >> loglvl;
                    switch(loglvl){
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                            memset(BUF,0,BUFLEN);
                            len = sprintf(BUF,"Set Log Level=%d",loglvl)+1;
                            sendto(sockfd,BUF,len,0,(struct sockaddr *)&servaddr,sizeof(servaddr));
                            
                            logFLAG = true;
                            break;
                        default:
                            cout << "Invalid Log Selection" << endl;
                            break;
                    }
                }
                logFLAG = false;
                break;
            case 2:
                fileReader();
                break;
            case 3:
                cout << "exiting menu" << endl;
                is_running = false;
                break;
            default:
                cout << "INVALID OPTION" << endl;
                is_running = true;
                break;
        }
    }
    pthread_join(servthread,NULL);
}

void fileReader(){
    char BUF[BUFLEN];
//read from the file provided
//Open a file in RDWR mode, create if doesnt exist, set rw perms for user/owner
    int readFD = open("serv.log",O_RDONLY);
    int num_read;
    //pthread_mutex_lock(&lockserv);
    while ((num_read = read(readFD,BUF,BUFLEN))>0){
        cout << BUF << endl;
        cout << "bytes read: " << num_read << endl;
    }
    //pthread_mutex_unlock(&lockserv);
    if (close(readFD)==-1){
        perror("close");
    }
}

void *recv_func(void *arg){
    int writeFD = open("serv.log",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    int numtowrite;
    char BUF[BUFLEN];
    int ret, len;
    int servSock = *(int *) arg;
   
    while(is_running){
        pthread_mutex_lock(&lockserv);
        numtowrite = recvfrom(servSock,BUF,BUFLEN,0,(struct sockaddr*)&servaddr,&servlen);
        //write the contents of BUF to the file
        if(write(writeFD,BUF,numtowrite)!=numtowrite){
            perror("Incomplete Write");
        }
        pthread_mutex_unlock(&lockserv);
        sleep(1);
    }
    pthread_exit(NULL);
}
