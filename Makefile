CXX=g++
RM=rm -f
OBJS=fclones main.o fclones.o md5.o

main: main.o fclones.o md5.o
	$(CXX) -o fclones -g main.o fclones.o md5.o -g -L/usr/local/boost/lib -lboost_system -lboost_filesystem -lboost_program_options --std=c++11	

main.o: main.cpp main.h
	$(CXX) -c main.cpp -g -I/usr/local/boost/include --std=c++11

fclones.o: fclones.cpp fclones.h
	$(CXX) -c fclones.cpp -g -I/usr/local/boost/include --std=c++11

md5.o: md5.cpp
	$(CXX) -c -g md5.cpp -lc

all: fclones

clean:
	$(RM) $(OBJS)
