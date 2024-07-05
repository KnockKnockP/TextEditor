#Shamelessly ChatGPT ahh makefile.

SRC_DIR = ./TextEditor.Win32
BUILD_DIR = ./bin

CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -O3 -std=c++14
LINKS = -lgdi32 -static

TARGET = $(BUILD_DIR)/TextEditor.exe
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	@echo "1"
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LINKS)

$(OBJS): $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LINKS)

clean:
	rm -rf $(BUILD_DIR)