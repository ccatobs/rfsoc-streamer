CC = gcc

EXE = stream_min_example
OBJS = stream_min_example.o
LIBS = ../src/RfsocTransmitter.o ../src/RfsocBuilder.o \
       -Wl,--copy-dt-needed-entries \
       -lspt3g-core -pthread -lpython3.12 -lstdc++

CFLAGS = -g --std=c++17 -pthread \
        -I ../include -I ${HOME}/git/spt3g_software/core/include/core -I ${HOME}/git/spt3g_software/core/include \
        -I /usr/include/python3.12 -I ${HOME}/CCAT/streamer_test/lib/python3.12/site-packages/numpy/_core/include \
	-I ${HOME}/CCAT/streamer_test/lib/python3.12/site-packages/pybind11/include 

#LFLAGS = -L /home/ubuntu/CCAT/streamer_test/lib/python3.12/site-packages/spt3g.libs
LFLAGS = -L /home/ubuntu/git/spt3g_software/build/lib/

all: $(EXE)

stream_min_example.o: stream_min_example.cxx
	$(CC) -c -o $@ $(CFLAGS) $^

stream_min_example: $(OBJS)
	$(CC) -o stream_min_example $(OBJS) $(LFLAGS) $(LIBS) # -Wl,-rpath-link=${HOME}/so3g/lib


clean:
	rm -f $(OBJS) $(EXE)

EXE2 = inspect_g3
OBJS2 = inspect_g3.o

inspect_g3.o: inspect_g3.cxx
	$(CC) -c -o $@ $(CFLAGS) $^

inspect_g3: $(OBJS2)
	$(CC) -o inspect_g3 $(OBJS2) $(LFLAGS) $(LIBS) # -Wl,-rpath-link=${HOME}/so3g/lib


EXE3 = cppexample
OBJS3 = cppexample.o

cppexample.o: cppexample.cxx
	$(CC) -c -o $@ $(CFLAGS) $^

cppexample: $(OBJS3)
	$(CC) -o cppexample $(OBJS3) $(LFLAGS) $(LIBS)


