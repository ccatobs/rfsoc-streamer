CC = gcc

OBJ = RfsocBuilder.o RfsocTransmitter.o

CFLAGS = -g -pthread -I ../include -I /home/ubuntu/git/spt3g_software/core/include/core -I /home/ubuntu/git/spt3g_software/core/include \
         -I /home/ubuntu/git/so3g/include -I /usr/include/python3.10 \
         -I /home/ubuntu/streamer_test1/lib/python3.10/site-packages/numpy/core/include/ \
         -D_GLIBCXX_USE_CXX11_ABI=0

all: $(OBJ)

RfsocBuilder.o: RfsocBuilder.cpp
	$(CC) -c -o $@ $(CFLAGS) $^

RfsocTransmitter.o: RfsocTransmitter.cpp
	$(CC) -c -o $@ $(CFLAGS) $^

clean:
	rm -f *.o

