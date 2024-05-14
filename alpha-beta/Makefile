CXX = g++

CXXFLAGS = -Wall -std=c++11 -pthread
LDFLAGS = -lusb-1.0

TARGET = alphaBetaGomoku

# May 12th
# SOURCES = ../src/gomoku.cpp ../src/players.cpp ../src/tcp.cpp ../src/display.cpp network.cpp 
SOURCES = ../src/display.cpp ../src/gomoku.cpp ../src/input.cpp ../src/players.cpp\
../src/tcp.cpp ../src/touchpad.cpp ../src/xboxcont.cpp gomokuAI.cpp game.cpp main.cpp

# legacy
# SOURCES = ../src/gomoku.cpp ../src/players.cpp ../src/tcp.cpp tcp_main_display.cpp ../src/display.cpp
# SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp pve_display.cpp ../src/display.cpp ../src/xboxcont.cpp ../src/touchpad.cpp
# SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp pve.cpp ../src/xboxcont.cpp


# OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS = $(notdir $(SOURCES:.cpp=.o))

all: $(TARGET)

$(TARGET): $(OBJECTS)
#$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS)
#$(CXX) $(OBJECTS) $(CXXFLAGS) $(LDFLAGS) -o $@
	$(CXX) $(OBJECTS) $(CXXFLAGS) -o $@ $(LDFLAGS)

%.o: %.cpp
#$(CXX) $(CXXFLAGS) -c $<

	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: ../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
# rm -f $(TARGET) $(OBJECTS)
	rm -f *.o
	rm -f alphaBetaGomoku
