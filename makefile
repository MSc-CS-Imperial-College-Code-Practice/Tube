exec: main.o tube.o
	  	g++ -Wall -g main.o tube.o -o exec

main.o: main.cpp tube.h
		g++ -Wall -g -c main.cpp

tube.o: tube.cpp tube.h
		  g++ -Wall -g -c tube.cpp 

clean:
	  rm -f *.o exec