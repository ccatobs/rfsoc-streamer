CC = gcc

OBJ = RfsocBuilder.o RfsocTransmitter.o

CFLAGS = -I ../include -I /usr/local/include/spt3g/core -I /usr/local/include/spt3g \
       	 -I /home/ubuntu/so3g/include -I /usr/include/python3.8 \
       	 -I /home/ubuntu/.local/lib/python3.8/site-packages/numpy/core/include

all: $(OBJ)

RfsocBuilder.o: RfsocBuilder.cpp
	$(CC) -c -o $@ $(CFLAGS) $^

RfsocTransmitter.o: RfsocTransmitter.cpp
	$(CC) -c -o $@ $(CFLAGS) $^

clean:
	rm -f *.o

