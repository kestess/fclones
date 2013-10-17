CXX=g++
RM=rm -f
OBJS=fclones main.o fclones.o md5.o

main: main.o
	$(CXX) -o fclones -O3 main.o fclones.o clone.o md5.o -O3 -L/usr/local/boost/lib -lboost_system -lboost_filesystem -lboost_program_options --std=c++11	

main.o: main.cpp fclones.o
	$(CXX) -c main.cpp -O3 -I/usr/local/boost/include --std=c++11

fclones.o: fclones.cpp clone.o md5.o
	$(CXX) -c fclones.cpp -O3 -I/usr/local/boost/include --std=c++11

clone.o: clone.cpp
	$(CXX) -c clone.cpp -O3 -I/usr/local/boost/include --std=c++11	

md5.o: md5.cpp
	$(CXX) -c -O3 md5.cpp -lc

all: fclones

clean:
	$(RM) $(OBJS)
