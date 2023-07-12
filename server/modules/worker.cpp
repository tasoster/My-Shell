#include "../include/mutual.h"

extern Info info;
extern std::map<std::string, std::string> voterParty;
extern int terminate;
extern pthread_mutex_t mapMtx;
extern pthread_mutex_t trmMtx;


// ====================================================================================================

void strtrim(char* str);

// Worker thread function
void* workerThread(void* arg) {

    Buffer* buffer = (Buffer*)(arg);

    // worker loop
    while (1) {

        // Get client socket from buffer if non-empty
        pthread_mutex_lock(&buffer->mtx);
        while (buffer->count == 0) {
            pthread_mutex_lock(&trmMtx);    // Check if terminate flag is set
            if (terminate) {                // Terminate flag is set, exit thread normally
                pthread_mutex_unlock(&buffer->mtx);
                pthread_mutex_unlock(&trmMtx);
                pthread_exit(NULL);
            }
            pthread_mutex_unlock(&trmMtx);
            pthread_cond_wait(&buffer->full, &buffer->mtx);
        }

        // Get client socket from buffer
        int clientSocket = buffer->connections[buffer->out];
        buffer->out = (buffer->out + 1) % buffer->bufferSize;
        buffer->count--;
        pthread_cond_signal(&buffer->empty);
        pthread_mutex_unlock(&buffer->mtx);

        // No need of buffer now. Handle client request
        const char* namePrompt = "SEND NAME PLEASE: ";
        write(clientSocket, namePrompt, strlen(namePrompt));

        // store a string of the form "firstName lastName"
        char voterInfo[512];
        memset(voterInfo, 0, sizeof(voterInfo));
        read(clientSocket, voterInfo, sizeof(voterInfo));
        strtrim(voterInfo);

        // Extract first and last name from the received string
        char firstName[256];
        char lastName[256];
        memset(firstName, 0, sizeof(firstName));
        memset(lastName, 0, sizeof(lastName));

        int scanResult = sscanf(voterInfo, "%s %s", firstName, lastName);
        if (scanResult != 2) {  // Invalid input format, notify client and close connection
            const char* errorMsg = "Invalid input format. Please provide first name and last name.\n";
            write(clientSocket, errorMsg, strlen(errorMsg));
            close(clientSocket);
            continue;   // move on to next client
        }

        // Combine first name and last name into a single string
        char fullName[512];
        sprintf(fullName, "%s %s", firstName, lastName);

        /* Check if the current voter has already voted */
        pthread_mutex_lock(&mapMtx);
        std::map<std::string, std::string>::iterator it = voterParty.find(fullName);
        if (it != voterParty.end()) {       // Voter has already voted. notify client and close connection
            const char* errorMsg = "ALREADY VOTED\n";
            write(clientSocket, errorMsg, strlen(errorMsg));
            close(clientSocket);
            pthread_mutex_unlock(&mapMtx);
            continue;
        }
        pthread_mutex_unlock(&mapMtx);

        // Voter has not voted yet. Ask for vote
        const char* votePrompt = "SEND VOTE PLEASE: ";
        write(clientSocket, votePrompt, strlen(votePrompt));

        // Read vote from client
        char vote[256];
        memset(vote, 0, sizeof(vote));
        read(clientSocket, vote, sizeof(vote));
        strtrim(vote);

        // Write to pollLog file. 
        pthread_mutex_lock(&info.logMtx);
        fprintf(info.pollLogFile, "%s %s\n", fullName, vote);
        fflush(info.pollLogFile);
        pthread_mutex_unlock(&info.logMtx);

        // update server map
        pthread_mutex_lock(&mapMtx);
        voterParty[fullName] = vote;
        pthread_mutex_unlock(&mapMtx);

        // finally send confirmation message to client
        char confirmationMsg[512];
        snprintf(confirmationMsg, sizeof(confirmationMsg), "VOTE for Party %s RECORDED\n", vote);
        write(clientSocket, confirmationMsg, strlen(confirmationMsg));

        // time to close connection and move on to next client
        close(clientSocket);
    }

    pthread_exit(NULL);
}


// Trim leading and trailing whitespace from a string
void strtrim(char* str) {
    char* start = str;
    while (isspace(*start))
        start++;

    char* end = str + strlen(str) - 1;
    while (end > start && isspace(*end))
        end--;
    *(end + 1) = '\0';

    if (str != start)
        memmove(str, start, end - start + 2);
}