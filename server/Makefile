# paths
INCLUDE = ./include
MODULES = ./modules

# compiler
CC = g++

# Compile options. 
CFLAGS =  -Werror -g -I$(INCLUDE) 
LDFLAGS = -lpthread

# files .o
OBJS = poller.o $(MODULES)/worker.o $(MODULES)/functions.o

# executable
EXEC = poller

# arguments
ARGS = 5634 8 16 pollLog.txt pollStats.txt

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(EXEC) pollLog.txt pollStats.txt

run: $(EXEC)
	./$(EXEC) $(ARGS)

valgrind: $(EXEC)
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./$(EXEC) $(ARGS)
