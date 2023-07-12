#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_STR_LENGTH 256

// struct to pass arguments to the thread
struct ThreadArgs {
    char fullName[MAX_STR_LENGTH];
    char vote[MAX_STR_LENGTH];
    char serverName[MAX_STR_LENGTH];
    char portNum[MAX_STR_LENGTH];
};

// Send a vote to the server
void sendVote(const char* fullName, const char* vote, const char* serverName, const char* portNum) {
    int port, sock;
    char buf[MAX_STR_LENGTH];
    struct sockaddr_in server;
    struct hostent* rem;

    // Get host name
    if ((rem = gethostbyname(serverName)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }

    // Get port number
    port = atoi(portNum);

    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        pthread_exit(NULL);
    }

    // Expect to receive "send name" from server
    memset(buf, 0, sizeof(buf));
    read(sock, buf, sizeof(buf));

    // Send name to server
    sprintf(buf, "%s\n", fullName);
    write(sock, buf, strlen(buf));

    // Expect to receive "send vote" or "already voted" from server
    memset(buf, 0, sizeof(buf));
    read(sock, buf, sizeof(buf));

    // Send vote to server
    sprintf(buf, "%s\n", vote);
    write(sock, buf, strlen(buf));

    // Expect to receive "vote recorded" from server
    memset(buf, 0, sizeof(buf));
    read(sock, buf, sizeof(buf));

    // Communication ends
    close(sock);
}

void* voteThread(void* arg) {
    struct ThreadArgs* args = (struct ThreadArgs*)arg;
    sendVote(args->fullName, args->vote, args->serverName, args->portNum);
    free(args);  // Free the allocated memory for ThreadArgs
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: ./pollSwayer <serverName> <portNum> <inputFile>\n");
        return 1;
    }

    // Get command line arguments
    const char* serverName = argv[1];
    const char* portNum = argv[2];
    const char* inputFile = argv[3];

    // Open the input file
    FILE* file = fopen(inputFile, "r");
    if (file == NULL) {
        perror("Error opening input file");
        return 1;
    }

    // Count number of lines in file. Will create that many threads
    char line[MAX_STR_LENGTH];
    int numThreads = 0;
    while (fgets(line, sizeof(line), file))
        numThreads++;
    fseek(file, 0, SEEK_SET);       // Reset the file pointer to the beginning

    // Allocate an array of threads
    pthread_t* threads = (pthread_t*)malloc(numThreads * sizeof(pthread_t));

    // Create threads. one for each line in input file
    int id = 0;
    while (fgets(line, sizeof(line), file)) {
        char firstName[MAX_STR_LENGTH];
        char lastName[MAX_STR_LENGTH];
        char vote[MAX_STR_LENGTH];
        // get all three strings from line
        if (sscanf(line, "%s %s %s", firstName, lastName, vote) == 3) {
            // form the arguments to pass to the thread
            struct ThreadArgs* args = (struct ThreadArgs*)malloc(sizeof(struct ThreadArgs));
            snprintf(args->fullName, 2 * MAX_STR_LENGTH, "%s %s", firstName, lastName);
            strcpy(args->vote, vote);
            strcpy(args->serverName, serverName);
            strcpy(args->portNum, portNum);

            // create the thread
            if (pthread_create(&threads[id], NULL, voteThread, (void*)args) != 0) {
                fprintf(stderr, "Error creating thread\n");
                free(args);
            } else
                id++;
        }
    }

    fclose(file);
    // Wait for each thread to join
    for (int i = 0; i < id; i++)
        pthread_join(threads[i], NULL);
    free(threads);

    return 0;
}
