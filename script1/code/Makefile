CC = g++
CXXFLAGS = -Wall -ggdb -pthread

LDFLAGS = -pthread -lm

TARGET = ull-main

.PHONY: all clean cleanall	# Ignorar ficheiros com os nomes clean e cleanall caso estejam criados

all:	$(TARGET)

$(TARGET):	main.o ull.o 
		g++ -o $@ $^ $(LDFLAGS)

$(TARGET):	ull.h

clean:
		rm -f *.o *~

cleanall:	clean
		rm -f $(TARGET)

