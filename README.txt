Systems Programming - Project 2  

Server (Exercise 1)
===========================================================================================================

~~~ In order to compile and run the server, hit "make run" inside the server directory. ~~~
~~~ Default execution is "./poller 5634 8 16 pollLog.txt pollStats.txt" ~~~

Note: The server code is written in c style code, however g++ has been used
in order to utilize the c++ standard library.


poller.cpp:

    This file contains the main function. It is responsible for collecting command line arguments, 
    and setting up the TCP server. The thread of the main() function is considered the MasterThread. 
    This thread creates worker threads and inserts client sockets to the pool, being in a while loop.
    Notice the first line of the main() function, where the signal handler is set up. "siginHandler" 
    function get's called when SIGINT is received (ctrl+c), and it is responsible for gracefull 
    termination.  

    Structs Info and Buffer are defined in "mutual.h" header file. Info struct contains general information
    about the specific execution (server port, log file name, etc). Buffer struct contains information
    about the buffer (buffer size, buffer count, etc). Both structs contain mutex and condition variables,
    in order to be used for synchronization between threads.

    There is also the voterParty map (has mapMtx mutex), which stores every voter and the party he voted for. 
    This map gets constantly updated by the worker threads for two reasons:
        - For worker threads to check if a voter has already voted
        - For masterThread to count the votes for each party at the end of the program, where pollStats
        file gets formed.
    
    There is also the terminate flag (has terminateMtx mutex), which is used to terminate the worker threads.

    The pollLogFile gets filled live by the worker threads (using logMtx mutex), and pollStats file gets 
    formed at the end of the program.
    

worker.cpp:

    The workerThread function is defined here. It is the routine of each worker thread that main() creates.
    A worker thread is constantly inside a loop and tries to get a client socket from the pool. When it
    gets one, the communication with the client starts. The worker thread sends "SEND NAME PLEASE" to client,
    and expects a string that contains two words (first name, last name). It uses this info to check if the
    voter has already voted (by locking the mapMtx). If he has, it sends "ALREADY VOTED", otherwise asks for 
    the vote. When the vote is received, the worker thread:
        - Writes to pollLogFile by locking the logMtx mutex
        - Updates the voterParty map by locking the mapMtx mutex
        - Sends "VOTE FOR PARTY ... RECORDED" to client and closes the connection.


functions.cpp:

    Contains some useful functions for the program. Here is defined the "fillStats" function which gets
    called by the masterThread at the end of the program. This function fills the pollStats file by counting
    the votes for each party. The necessary information to do that has already been stored in the voterParty
    map by the worker threads.





Client (Exercise 2)
===========================================================================================================

~~~ In order to compile and run the client, hit "make run" inside the client directory. ~~~
~~~ Default execution is "./pollSwayer linux01.di.uoa.gr 5634 inputFile" ~~~

Note: The code is written completely in c.


pollSwayer.c:

    This file contains the main function. It is responsible for collecting command line arguments.
    It counts the lines of the inputFile, and creates a thread for each line. Each thread receives an 
    arguments struct, which contains the strings (fullName, vote etc). The thread is responsible for connecting
    to the server, sending the necessary info, and receiving the response. 

    Notice that each line of the input file is a new different thread, thus a new different connection to the
    server. Also, the loop that creates the threads, doesn't join them right away, since we want to utilize
    parallel execution benefits. Threads are joined at the end of the program, after the loop has finished.




Scripts (Exercise 3)
===========================================================================================================


All three scripts are written in bash. They are well commented and self explanatory. In the scripts 
directory, there are some text files in order to test/run the scripts.

Execution examples:

create_input.sh:
    ./create_input.sh  political_parties.txt 20

tallyVotes.sh:
    ./tallyVotes.sh tallyResultsFile

processLogFile.sh
    ./processLogFile.sh poll-log
