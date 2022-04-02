/*Start of code*/
#include <iostream>

enum LOG_LEVEL{
    DEBUG,
    WARNING,
    ERROR,
    CRITICAL
};

//functions

/* Create a non blocking socket for UDP
* set address and port of server
* create mutex to protect any hared resources
* start recieve thread, pass file descriptor to it*/
int InitializeLog();

//Set filter log level, store in a variable global within Logger.cpp
void SetLogLevel(LOG_LEVEL level);

/*Compare the severity of log to the filter log severity - log is discarded if severity is lower
* create timestamp to be added to log message
* apply mutexing to shared resources
* message sent to server via UDP "sendto"*/
void Log(LOG_LEVEL level, const char *prog, const char* func,int line, const char *message);

//change an is_running flag, closing the descriptor 
void ExitLog();