CXX = g++

CXXFLAGS = -Wall -std=c++11 -pthread

TARGET = alphaBetaGomoku

SOURCES = ../src/gomoku.cpp ../src/players.cpp ../src/tcp.cpp tcp_main.cpp ../src/pve.cpp
# SOURCES = ../src/gomoku.cpp ../src/players.cpp ../src/tcp.cpp tcp_main_display.cpp ../src/display.cpp
# SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp selfplay.cpp
# SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp pve_display.cpp ../src/display.cpp

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
	rm -f $(TARGET) $(OBJECTS)
