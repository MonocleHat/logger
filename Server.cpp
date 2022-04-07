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
void shutdownHandler(int sig)
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
    signal(SIGINT, shutdownHandler);
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
    int menuopt = -1;
    int loglvl;
    int len;
    bool logFLAG = false;
    is_running = true;
    while(menuopt!=0 && is_running == true){
        system("clear");
        cout << "-=-=-=-=-=-=-=-=-" << endl;
        cout << "|A2 Server 1.0.2|" << endl;
        cout << "-=-=-=-=-=-=-=-=-" << endl;
        cout << "1. Set Log Level" << endl;
        cout << "2. Dump Logs" << endl;
        cout << "0. Quit" << endl;
        cout << "Selection: ";
        cin >> menuopt;
        switch (menuopt){
            case 1:
                    cout << "Enter new log level (0-3):";
                    cin >> loglvl;
                    if(loglvl > 3 || loglvl < 0){
                        cout << "INVALID SELECTION! returning to menu" << endl;
                        sleep(2);
                    }else{
                        pthread_mutex_lock(&lockserv);
                        //cout << "INLOG" << endl;
                        memset(BUF,0,BUFLEN);
                        len = sprintf(BUF,"Set Log Level=%d", loglvl)+1;
                        sendto(sockfd,BUF,len,0,(struct sockaddr*)&servaddr,servlen);
                       // cout << "DEBUG::" << len << " || BUF: " << BUF << endl;
                        pthread_mutex_unlock(&lockserv);
                        
                    }
                break;
            case 2:
                fileReader();
                break;
            case 0:
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
    close(sockfd);
    return 0;
}

void fileReader(){
    char BUF[BUFLEN];
    char contchck;
//read from the file provided
//Open a file in RDWR mode, create if doesnt exist, set rw perms for user/owner
    int readFD = open("serv.log",O_RDONLY);
    int num_read=0;
    pthread_mutex_lock(&lockserv);
    do{
        num_read = read(readFD,BUF,BUFLEN);
        cout << BUF << endl;
    }while(num_read>0);
  
    pthread_mutex_unlock(&lockserv);
 
    if (close(readFD)==-1){
        perror("close");
    }
   		cout<<endl<<"Press any key to continue: ";
        cin>>contchck;
}

void *recv_func(void *arg){
    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    int writeFD = open("serv.log",O_CREAT | O_WRONLY | O_TRUNC ,filePerms);
    int numtowrite;
    char BUF[BUFLEN];
    int ret, len;
    int *servSock = (int *) arg;
   
    while(is_running){

        pthread_mutex_lock(&lockserv);
        len = recvfrom(*servSock, BUF,BUFLEN,0,(struct sockaddr*)&servaddr,&servlen)-1;
        if(len<0){
            pthread_mutex_unlock(&lockserv);
            sleep(1);
        }else{
            write(writeFD,BUF,len);
            pthread_mutex_unlock(&lockserv);
        }
    }
    cout << "exiting thread" << endl;
    pthread_exit(NULL);
}
