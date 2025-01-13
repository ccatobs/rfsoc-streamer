CC = gcc

EXE = stream_min_example
OBJS = stream_min_example.o
LIBS = ../src/RfsocTransmitter.o ../src/RfsocBuilder.o \
       -Wl,--copy-dt-needed-entries \
       -lspt3g-core -lso3g -lboost_system -lboost_filesystem -lboost_python310 -pthread -lpython3.10 -lstdc++

CFLAGS = --std=c++11 -g -pthread \
        -I ../include -I ${HOME}/git/spt3g_software/core/include/core -I ${HOME}/git/spt3g_software/core/include \
        -I ${HOME}/git/so3g/include -I /usr/include/python3.10 -I ${HOME}/streamer_test/lib/python3.10/site-packages/numpy/core/include/

LFLAGS = -L ${HOME}/so3g/lib -L ${HOME}/so3g/so3g


all: $(EXE)

stream_min_example.o: stream_min_example.cxx
	$(CC) -c -o $@ $(CFLAGS) $^

stream_min_example: $(OBJS)
	$(CC) -o stream_min_example $(OBJS) $(LFLAGS) $(LIBS) -Wl,-rpath-link=${HOME}/streamer_test/lib/python3.10/site-packages/so3g.libs


clean:
	rm -f $(OBJS) $(EXE)

EXE2 = cppexample
OBJS2 = cppexample.o

cppexample.o: cppexample.cxx
	$(CC) -c -o $@ $(CFLAGS) $^

cppexample: $(OBJS2)
	$(CC) -o cppexample $(OBJS2) $(LFLAGS) $(LIBS) -Wl,-rpath-link=${HOME}/streamer_test/lib/python3.10/site-packages/so3g.libs
