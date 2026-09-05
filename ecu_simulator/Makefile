TARGET = vehicle_ecu
TEST_TARGET = build/test_powertrain

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude
LDFLAGS = -lncurses -lpthread

SRCDIR = src
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(SRCS:$(SRCDIR)/%.cpp=build/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): tests/test_powertrain.cpp build/PowertrainModel.o | build
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: $(SRCDIR)/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build $(TARGET)

.PHONY: all clean test
