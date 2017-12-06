all: main.o GrepParser.o JavaReader.o JavaParser.o Utility.o
	g++ -std=c++11 main.o GrepParser.o JavaReader.o JavaParser.o Utility.o -o runtime_scanner

main.o: main.cpp
	g++ -std=c++11 -g -c main.cpp -o main.o

GrepParser.o: GrepParser.cpp
	g++ -std=c++11 -g -c GrepParser.cpp -o GrepParser.o

JavaReader.o: JavaReader.cpp
	g++ -std=c++11 -g -c JavaReader.cpp -o JavaReader.o

JavaParser.o: JavaParser.cpp
	g++ -std=c++11 -g -c JavaParser.cpp -o JavaParser.o

Utility.o: Utility.cpp
	g++ -std=c++11 -g -c Utility.cpp -o Utility.o

clean:
	rm main.o
	rm GrepParser.o
	rm JavaReader.o
	rm JavaParser.o
	rm Utility.o
