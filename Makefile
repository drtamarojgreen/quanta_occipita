CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = quanta_occipita
SRC      = src/main.cpp
COMPONENTS = src/components/validator.hpp src/components/git_manager.hpp src/components/generators.hpp src/components/skeletons.hpp src/components/manager.hpp

all: $(TARGET)

$(TARGET): $(SRC) src/cli.hpp src/bootstrapper.hpp src/console.hpp src/licenses.hpp $(COMPONENTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

test: test_cli test_validator test_structural
	./test_cli
	./test_validator
	./test_structural

test_cli: test/unit/test_cli.cpp src/cli.hpp
	$(CXX) $(CXXFLAGS) -Isrc -o test_cli test/unit/test_cli.cpp

test_validator: test/unit/test_validator.cpp src/components/validator.hpp
	$(CXX) $(CXXFLAGS) -Isrc -o test_validator test/unit/test_validator.cpp

test_structural: test/sdd/test_structural.cpp src/components/generators.hpp
	$(CXX) $(CXXFLAGS) -Isrc -o test_structural test/sdd/test_structural.cpp

clean:
	rm -f $(TARGET) test_cli test_validator test_structural

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: all clean install
