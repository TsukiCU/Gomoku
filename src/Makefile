CXX = g++

CXXFLAGS = -Wall -std=c++11

TARGET = gomoku

SOURCES = gomoku.cpp players.cpp gomokuAI.cpp main.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<
