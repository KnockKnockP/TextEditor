CXX = gcc

ifdef OLD_TOOLS
	CXX = ./Tools/i686-8.1.0-release-win32-dwarf-rt_v6-rev0/bin/gcc.exe
endif

UNICODE = -DUNICODE -D_UNICODE -municode
UNICODE_BUILD_PATH = UNICODE
CRT_MAIN = -UCRT_MAIN
WINDOWS_VERSION = 0x0500
ARCH=native
BIT=64

ifdef NO_UNICODE
	UNICODE =
	UNICODE_BUILD_PATH = ANSI
endif

LINKS = -static
ifdef USE_CRT_MAIN
	CRT_MAIN = -DCRT_MAIN -ffreestanding
	LINKS = -nostdlib -lmsvcrt -lkernel32 -luser32 -lgdi32
endif

SOURCE_DIR = ./TextEditor.Win32
INCLUDE_DIR = $(SOURCE_DIR)
RESOURCES_DIR = $(SOURCE_DIR)/Resources
BIN_DIR = ./bin
BUILD_DIR = $(BIN_DIR)/$(ARCH)/$(WINDOWS_VERSION)/$(UNICODE_BUILD_PATH)/$(BIT)

CXXFLAGS = -Wall -Wextra -pedantic -O3 -Ofast -std=c11 $(UNICODE) $(CRT_MAIN) -DWINDOWS_VERSION=$(WINDOWS_VERSION) -march=$(ARCH) -mtune=$(ARCH) -m$(BIT) -mwindows

TARGET = $(BUILD_DIR)/TextEditor.exe
SRCS = $(wildcard $(SOURCE_DIR)/*.c)
OBJS = $(SRCS:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o)

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