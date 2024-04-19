CXX = g++

CXXFLAGS = -Wall -std=c++11

TARGET = alphaBetaGomoku

# pve, evp, pvp
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp evp.cpp

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
	rm alphaBetaGomoku

