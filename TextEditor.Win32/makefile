CXX = g++

ifdef OLD_TOOLS
	CXX = ./Tools/i686-8.1.0-release-win32-dwarf-rt_v6-rev0/bin/g++.exe
endif

UNICODE = -DUNICODE -D_UNICODE -municode
UNICODE_BUILD_PATH = UNICODE
WINDOWS_VERSION = 0x0500
ARCH=native
BIT=64

ifdef NO_UNICODE
	UNICODE =
	UNICODE_BUILD_PATH = ANSI
endif

SOURCE_DIR = ./TextEditor.Win32
INCLUDE_DIR = $(SOURCE_DIR)
RESOURCES_DIR = $(SOURCE_DIR)/Resources
BIN_DIR = ./bin
BUILD_DIR = $(BIN_DIR)/$(ARCH)/$(WINDOWS_VERSION)/$(UNICODE_BUILD_PATH)/$(BIT)

CXXFLAGS = -Wall -Wextra -pedantic -O3 -std=c++14 $(UNICODE) -DWINDOWS_VERSION=$(WINDOWS_VERSION) -march=$(ARCH) -mtune=$(ARCH) -m$(BIT) -mwindows
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
	cp $(RESOURCES_DIR)/$(WINDOWS_VERSION)/* $(BUILD_DIR)

clean:
	rm -rf $(BIN_DIR)