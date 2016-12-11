# Compiler vars
CXX = g++
CXXFLAGS = -g -std=c++11 -O2
INC = -I./include

SRC = ./src
TESTSRC = ./test
BIN = ./bin

SRCS = $(wildcard $(SRC)/*.cpp)
OBJS = $(SRCS:.cpp=.o)
EXEC = $(BIN)/chart-tidy

$(EXEC): $(OBJS)
	$(CXX) $(INC) $(CXXFLAGS) -o $(EXEC) $(OBJS)

%.o: %.cpp
	$(CXX) -c $(INC) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(EXEC) $(OBJS)

