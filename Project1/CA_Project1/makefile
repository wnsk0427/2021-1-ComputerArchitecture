runfile : main.o format.o
	g++ -o runfile main.o format.o -std=c++14

main.o : main.cpp
	g++ -c main.cpp -std=c++14

format.o : format.h format.cpp
	g++ -c format.h format.cpp -std=c++14

clean :
	rm main.o
	rm format.o
	rm format.h.gch
	rm runfile
