CXX=g++
CFLAGS=-O3
RM=rm -f
OBJS=fclones *.o fclones_test testing/* 

main: main.o
	$(CXX) -o fclones $(CFLAGS) main.o fclones.o clone.o md5.o -L/usr/local/boost/lib -lboost_system -lboost_filesystem -lboost_program_options --std=c++11	

main.o: main.cpp fclones.o
	$(CXX) -c main.cpp $(CFLAGS) -I/usr/local/boost/include --std=c++11

fclones.o: fclones.cpp clone.o md5.o
	$(CXX) -c fclones.cpp $(CFLAGS) -I/usr/local/boost/include --std=c++11

clone.o: clone.cpp
	$(CXX) -c clone.cpp $(CFLAGS) -I/usr/local/boost/include --std=c++11	

md5.o: md5.cpp
	$(CXX) -c $(CFLAGS) md5.cpp -lc

all: fclones

clean:
	@$(RM) $(OBJS); rmdir testing 2>/dev/null || true

test: fclones.o
	$(CXX) -o fclones_test fclones_test.cpp fclones.o clone.o md5.o -I/usr/local/boost/include -L/usr/local/boost/lib -lboost_system -lboost_filesystem -lboost_program_options -lboost_unit_test_framework --std=c++11

