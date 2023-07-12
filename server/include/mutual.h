#include <iostream>
#include <string>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <cctype>
#include <map>

struct Info {
    int portnum;
    int numWorkerthreads;
    int bufferSize;
    char* pollLog;
    char* pollStats;
    pthread_t* threads;
    int serverSocket;
    pthread_mutex_t logMtx;
    FILE* pollLogFile;
    FILE* pollStatsFile;
};

struct Buffer {
    int* connections;
    int bufferSize;
    int count;
    int in;
    int out;
    pthread_mutex_t mtx;
    pthread_cond_t full;
    pthread_cond_t empty;
};

void* workerThread(void*);

void initializeBuffer(Buffer*, int);

void destroyBuffer(Buffer*);

void fillStats(FILE*, std::map<std::string, std::string>);