CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES=Logger.cpp
FILES+=Automobile.cpp
FILES+=TravelSimulator.cpp
LIBS=-lpthread

travel: $(FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
server: Server.cpp
	$(CC) $(CFLAGS) Server.cpp -o server $(LIBS)

clean:
	rm -f *.o travel
	
all: travel server
