runfile : main.o console.o
	g++ -o runfile main.o console.o -std=c++14

main.o : main.cpp
	g++ -c main.cpp -std=c++14

console.o : console.h console.cpp
	g++ -c console.h console.cpp -std=c++14

clean :
	rm main.o
	rm console.o
	rm console.h.gch
	rm runfile
