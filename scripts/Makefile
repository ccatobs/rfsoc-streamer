CC = gcc

EXE = stream_min_example
OBJS = stream_min_example.o
LIBS = ../src/RfsocTransmitter.o ../src/RfsocBuilder.o \
       -lso3g -lspt3g-core \
       -lboost_system -lboost_python38 \
       -pthread -lpython3.8 -lstdc++

CFLAGS = -I ../include -I /usr/local/include/spt3g/core -I /usr/local/include/spt3g \
       	 -I /home/ubuntu/so3g/include -I /usr/include/python3.8 \
       	 -I /home/ubuntu/.local/lib/python3.8/site-packages/numpy/core/include

LFLAGS = -L /usr/local/lib/python3.8/dist-packages/so3g

all: $(EXE)

stream_min_example.o: stream_min_example.cxx
	$(CC) -c -o $@ $(CFLAGS) $^

stream_min_example: $(OBJS)
	$(CC) -o stream_min_example $(LFLAGS) $(OBJS) $(LIBS)

clean:
	rm -f $(EXE)

