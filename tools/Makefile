C_CRAP=-I/Users/trent/local/include -g
LD_CRAP=-L/Users/trent/local/lib -lallegro_primitives -lallegro_image -lallegro -lallegro_main

all: earclip

earclip: earclip.o dllist.o
	g++ earclip.o dllist.o -o earclip $(LD_CRAP)

earclip.o: earclip.cpp testing.h
	g++ $(C_CRAP) -c earclip.cpp

dllist.o: dllist.cpp dllist.h
	g++ $(C_CRAP) -c dllist.cpp

clean:
	rm -f earclip.o dllist.o
