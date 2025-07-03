CC = gcc

OBJ = RfsocBuilder.o RfsocTransmitter.o

CFLAGS = -g --std=c++17 -pthread -I ../include -I ${HOME}/git/spt3g_software/core/include/core -I ${HOME}/git/spt3g_software/core/include \
         -I /usr/include/python3.12 \
         -I ${HOME}/CCAT/streamer_test/lib/python3.12/site-packages/numpy/core/include/ \
         -I ${HOME}/CCAT/streamer_test/lib/python3.12/site-packages/pybind11/include/

all: $(OBJ)

RfsocBuilder.o: RfsocBuilder.cxx
	$(CC) -c -o $@ $(CFLAGS) $^

RfsocTransmitter.o: RfsocTransmitter.cxx
	$(CC) -c -o $@ $(CFLAGS) $^

clean:
	rm -f *.o

