# paths
INCLUDE = ./include
MODULES = ./modules

# compiler
CC = gcc

# Compile options. 
CFLAGS =  -Werror -g -I$(INCLUDE)
LDFLAGS = -lpthread 

# files .o
OBJS = pollSwayer.o

# executable
EXEC = pollSwayer

# arguments
ARGS = linux01.di.uoa.gr 5634 inputFile

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	./$(EXEC) $(ARGS)

valgrind: $(EXEC)
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./$(EXEC) $(ARGS)
