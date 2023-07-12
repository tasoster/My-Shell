#include "./include/mutual.h"

Info info;                                      // general execution info 
Buffer buffer;                                  // buffer for client sockets
std::map<std::string, std::string> voterParty;  // map for voter and party
int terminate = 0;                              // termination flag

pthread_mutex_t mapMtx;                         // mutex for voterParty map
pthread_mutex_t trmMtx;                         // mutex for terminate flag


// Signal handler for SIGINT. The way to exit program gracefully
void sigintHandler(int);


int main(int argc, char** argv) {
    
    signal(SIGINT, sigintHandler);

    if (argc != 6) {
        printf("Usage: ./poller <portnum> <numWorkerthreads> <bufferSize> <poll_log> <poll_stats>\n");
        return 0;
    }

    // Initialize info struct
    info.portnum = atoi(argv[1]);
    info.numWorkerthreads = atoi(argv[2]);
    info.bufferSize = atoi(argv[3]);
    info.pollLog = argv[4];
    info.pollStats = argv[5];

    // Initialize buffer
    initializeBuffer(&buffer, info.bufferSize);

    // Initialize mutexes
    if (pthread_mutex_init(&info.logMtx, NULL) != 0) {
        perror("Error initializing pollLog mutex"); exit(1);}

    if (pthread_mutex_init(&mapMtx, NULL) != 0) {
        perror("Error initializing map mutex"); exit(1);}

    // Initialize pollLog and pollStats files
    info.pollLogFile = fopen(info.pollLog, "a");
    if (info.pollLogFile == NULL) {
        perror("Error opening pollLog file"); exit(1);}

    info.pollStatsFile = fopen(info.pollStats, "w");
    if (info.pollStatsFile == NULL) {
        perror("Error opening pollStats file"); exit(1);}

    // Initialize threads
    info.threads = (pthread_t*)malloc(sizeof(pthread_t) * info.numWorkerthreads);
    for (int i = 0; i < info.numWorkerthreads; i++) 
        pthread_create(&info.threads[i], NULL, workerThread, (void*)&buffer);
    
    // Initialize server socket
    struct sockaddr_in serverAddress;
    info.serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (info.serverSocket < 0) {
        perror("Error opening socket"); exit(1); }

    // Set socket options
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(info.portnum);

    // Bind socket to address
    if (bind(info.serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Error binding"); exit(1);}

    // Listen for connections
    if (listen(info.serverSocket, 128) < 0) {
        perror("Error listening"); exit(1); }

    // Main server loop
    while (1) {

        struct sockaddr_in clientAddress;
        socklen_t clientLength = sizeof(clientAddress);
        
        // Accept connection
        int clientSocket = accept(info.serverSocket, (struct sockaddr*)&clientAddress, &clientLength);
        if (clientSocket < 0) {
            perror("Error accepting connection"); exit(1);}

        // Add client socket to buffer if not full
        pthread_mutex_lock(&buffer.mtx);
        while (buffer.count == buffer.bufferSize) 
            pthread_cond_wait(&buffer.empty, &buffer.mtx);

        buffer.connections[buffer.in] = clientSocket;
        buffer.in = (buffer.in + 1) % buffer.bufferSize;
        buffer.count++;

        // Signal worker threads that buffer is not empty
        pthread_cond_signal(&buffer.full);
        pthread_mutex_unlock(&buffer.mtx);

    }

    return 0;
    
}


// signal handler for SIGINT. The way to exit program gracefully
void sigintHandler(int sigNum) {
    
    printf("\nReceived SIGINT signal. Exiting gracefully...\n");

    // Set the termination flag
    pthread_mutex_lock(&trmMtx);
    terminate = 1;
    pthread_mutex_unlock(&trmMtx);
    pthread_cond_broadcast(&buffer.full); // Broadcast signal to all threads

    // Close server socket
    close(info.serverSocket);

    // Join worker threads
    for (int i = 0; i < info.numWorkerthreads; i++) 
        pthread_join(info.threads[i], NULL);
    printf("Worker threads joined\n");

    // Time to form the pollStats file
    fillStats(info.pollStatsFile, voterParty);

    // Cleanup resourses before ending the program
    destroyBuffer(&buffer);
    fclose(info.pollLogFile);
    fclose(info.pollStatsFile);
    pthread_mutex_destroy(&mapMtx);
    pthread_mutex_destroy(&info.logMtx);
    pthread_mutex_destroy(&trmMtx);
    free(info.threads); 
    exit(0);

}