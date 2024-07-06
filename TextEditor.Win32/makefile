#Shamelessly ChatGPT ahh makefile.

SOURCE_DIR = ./TextEditor.Win32
INCLUDE_DIR = $(SOURCE_DIR)
RESOURCES_DIR = $(SOURCE_DIR)/Resources
BUILD_DIR = ./bin

CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -O3 -std=c++14
LINKS = -lgdi32 -static

TARGET = $(BUILD_DIR)/TextEditor.exe
SRCS = $(wildcard $(SOURCE_DIR)/*.cpp)
OBJS = $(SRCS:$(SOURCE_DIR)/%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all

all: $(TARGET) RESOURCES

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LINKS) -I$(INCLUDE_DIR)

RESOURCES:
	cp $(RESOURCES_DIR)/* $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)