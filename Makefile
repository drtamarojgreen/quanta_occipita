CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = quanta_occipita
SRC      = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRC) src/cli.hpp src/bootstrapper.hpp src/console.hpp src/licenses.hpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: all clean install
