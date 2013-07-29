vodtest: vodtest.o utils.o
	g++ vodtest.o utils.o -o vodtest -lpthread
vodtest.o: vodtest.cpp utils.h
	g++ -c vodtest.cpp -o vodtest.o
utils.o: utils.cpp utils.h
	g++ -c utils.cpp -o utils.o