CXXFLAGS = -Wall -ggdb -pthread

LDFLAGS = -lpthread -lm

TARGETS = client-server

all:	$(TARGETS)

client-server:	client-server.o service.o classes.o fifo.o process.o  utils.o thread.o
		g++ -o $@ $^ $(LDFLAGS)


classes.o fifo.o process.o service.o service.o thread.o utils.o: %.o:		%.h

clean:
		rm -f *.o *~

cleanall:	clean
		rm -f $(TARGETS)