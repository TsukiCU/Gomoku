CXX = g++

CXXFLAGS = -Wall -std=c++11 -pthread

TARGET = alphaBetaGomoku

#SOURCES = ../src/gomoku.cpp ../src/players.cpp ../src/tcp.cpp ../src/display.cpp tcp_main.cpp
SOURCES = ../src/gomoku.cpp ../src/players.cpp ../src/tcp.cpp ../src/display.cpp network.cpp 

# OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS = $(notdir $(SOURCES:.cpp=.o))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

%.o: ../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
# rm -f $(TARGET) $(OBJECTS)
	rm -f *.o
	rm -f alphaBetaGomoku
