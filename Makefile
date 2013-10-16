CXX=g++
RM=rm -f
OBJS=fclones.o md5.o

fclones: fclones.cpp fclones.h md5.o
	$(CXX) -o fclones md5.o fclones.cpp -I/usr/local/boost/include -L/usr/local/boost/lib -lboost_system -lboost_filesystem --std=c++11

md5.o: md5.cpp
	$(CXX) -c md5.cpp -lc

all: fclones

clean:
	$(RM) $(OBJS)
