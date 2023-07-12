#include "../include/mutual.h"

void initializeBuffer(Buffer* buffer, int bufferSize) {
    buffer->connections = (int*)malloc(sizeof(int) * bufferSize);
    buffer->bufferSize = bufferSize;
    buffer->count = 0;
    buffer->in = 0;
    buffer->out = 0;
    pthread_mutex_init(&buffer->mtx, NULL);
    pthread_cond_init(&buffer->full, NULL);
    pthread_cond_init(&buffer->empty, NULL);
}

void destroyBuffer(Buffer* buffer) {
    free(buffer->connections);
    pthread_mutex_destroy(&buffer->mtx);
    pthread_cond_destroy(&buffer->full);
    pthread_cond_destroy(&buffer->empty);
}


// gets called when sigint is sent to main process. Time to fill the stats file based on the voterParty map
void fillStats(FILE* pollStats, std::map<std::string, std::string> voterParty) {
    
    // count the number of votes for each party
    std::map<std::string, int> partyVotes;
    std::map<std::string, std::string>::iterator it;
    for (it = voterParty.begin(); it != voterParty.end(); it++) {
        if (partyVotes.find(it->second) == partyVotes.end()) 
            partyVotes[it->second] = 1;
        else 
            partyVotes[it->second]++;
    }

    // fill the pollStats file
    std::map<std::string, int>::iterator it2;
    for (it2 = partyVotes.begin(); it2 != partyVotes.end(); it2++) 
        fprintf(pollStats, "%s %d\n", it2->first.c_str(), it2->second);
    fprintf(pollStats, "TOTAL %ld\n", voterParty.size());

}