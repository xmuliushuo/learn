vodtest: vodtest.o utils.o
	g++ -Wall vodtest.o utils.o -o vodtest
vodtest.o: vodtest.cpp utils.h
	g++ -Wall -c vodtest.cpp -o vodtest.o
utils.o: utils.cpp utils.h
	g++ -Wall -c utils.cpp -o utils.o