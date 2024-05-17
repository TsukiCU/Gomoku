CXX = g++

CXXFLAGS = -Wall -std=c++11 -pthread
LDFLAGS = -lusb-1.0

TARGET = alphaBetaGomoku

DIRECTORIES := . display game input network

SOURCES := $(wildcard $(addsuffix /*.cpp,$(DIRECTORIES)))

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET) kmod

game: $(TARGET)

kmod:
	make -C vga_kmod

clean:
	rm -f $(TARGET) $(OBJECTS)
	make clean -C vga_kmod

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(CXXFLAGS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all game kmod clean
