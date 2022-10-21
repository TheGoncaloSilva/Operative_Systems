CXXFLAGS = -Wall -ggdb -pthread

LDFLAGS = -lpthread -lm

TARGETS = client-server

all:	$(TARGETS)

client-server:	client-server.o service.o fifo.o buffer.o process.o delays.o utils.o
		g++ -o $@ $^ $(LDFLAGS)


fifo.o buffer.o process.o service.o service.o utils.o delays.o: %.o:		%.h

clean:
		rm -f *.o *~

cleanall:	clean
		rm -f $(TARGETS)