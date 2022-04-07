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
    int sockfd;
    char BUF[BUFLEN];
    
    memset(&servaddr, 0, sizeof(servaddr));
    signal(SIGINT, shutdownHandler); //signal handler
    //creation of the UDP socket using the constant IP and PORT values defined above
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(PORT);
    servlen = sizeof(servaddr);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); //creating our socket
     struct timeval to;
    to.tv_sec = 1;
    to.tv_usec = 0;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&to, sizeof(to)); //setting a timeout
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)); //binding the socket to the IP
    //Introducing threading
    pthread_mutex_init(&lockserv,NULL); //Creating the mutex
    pthread_create(&servthread,NULL,recv_func,&sockfd); //create a thread and pass socket fd to it

    //main menu creation
    int menuopt = -1;
    int loglvl;
    int len;
    is_running = true;
    while(menuopt!=0 && is_running == true){ //while the menu option is not 0, and we are running, get input
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
            case 1: //change log level
                    cout << "Enter new log level (0-3):";
                    cin >> loglvl;
                    if(loglvl > 3 || loglvl < 0){
                        cout << "INVALID SELECTION! returning to menu" << endl;
                        sleep(2); //if invalid log, sleep then return

                    }else{
                        pthread_mutex_lock(&lockserv);
                      
                        memset(BUF,0,BUFLEN);
                        len = sprintf(BUF,"Set Log Level=%d", loglvl)+1;
                        sendto(sockfd,BUF,len,0,(struct sockaddr*)&servaddr,servlen); //send changed log level to logger
                        pthread_mutex_unlock(&lockserv); //unlock 
                        
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
//Open a file in RD mode
    int readFD = open("serv.log",O_RDONLY);
    int num_read=0;
    pthread_mutex_lock(&lockserv);
    do{
        num_read = read(readFD,BUF,BUFLEN); //read all data while there is data to be read from file
        cout << BUF << endl;
    }while(num_read>0);
  
    pthread_mutex_unlock(&lockserv);
 
    if (close(readFD)==-1){
        perror("close");
    }
   		cout<<endl<<"Press any key to continue: ";
        cin>>contchck; //may have to enter something other than enterkey.
}

void *recv_func(void *arg){
    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; //rw-rw-rw-
    int writeFD = open("serv.log",O_CREAT | O_WRONLY | O_TRUNC ,filePerms);//open file, create if nonexistent, truncate it
    int numtowrite;
    char BUF[BUFLEN];
    int ret, len;
    int *servSock = (int *) arg;
   
    while(is_running){

        pthread_mutex_lock(&lockserv);//lock
        len = recvfrom(*servSock, BUF,BUFLEN,0,(struct sockaddr*)&servaddr,&servlen)-1;//recieve the log data 
        if(len<0){
            pthread_mutex_unlock(&lockserv);
            sleep(1);
        }else{
            write(writeFD,BUF,len); //writing log data to file
            pthread_mutex_unlock(&lockserv);
        }
    }
    cout << "exiting thread" << endl;
    pthread_exit(NULL);
}
