CC = gcc

OBJ = RfsocBuilder.o RfsocTransmitter.o

CFLAGS = -pthread -I ../include -I ${HOME}/git/spt3g_software/core/include/core -I ${HOME}/git/spt3g_software/core/include \
         -I ${HOME}/git/so3g/include -I /usr/include/python3.10 \
         -I ${HOME}/streamer_test/lib/python3.10/site-packages/numpy/core/include/ \

all: $(OBJ)

RfsocBuilder.o: RfsocBuilder.cxx
	$(CC) -c -o $@ $(CFLAGS) $^

RfsocTransmitter.o: RfsocTransmitter.cxx
	$(CC) -c -o $@ $(CFLAGS) $^

clean:
	rm -f *.o

