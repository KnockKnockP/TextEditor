CXX = gcc

ifdef OLD_TOOLS
    CXX = ./Tools/gcc_win98/bin/gcc.exe
endif

UNICODE = -DUNICODE -D_UNICODE -municode
UNICODE_BUILD_PATH = UNICODE
CRT_MAIN = -UCRT_MAIN
WINDOWS_VERSION = 0x0600
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

LINKS = $(BUILD_DIR)/resource.o -lcomctl32 -ldwmapi -luxtheme
ifdef USE_CRT_MAIN
    CRT_MAIN = -DCRT_MAIN -ffreestanding
    LINKS += -nostdlib -lmsvcrt -lkernel32 -luser32 -lgdi32 -limm32 -lcomdlg32 -ladvapi32
endif

CXXFLAGS = -Wall -Wextra -pedantic -O3 -Ofast -std=c2x $(UNICODE) $(CRT_MAIN) -D_WIN32_WINNT=$(WINDOWS_VERSION) -march=$(ARCH) -mtune=$(ARCH) -m$(BIT) -mwindows

TARGET = $(BUILD_DIR)/TextEditor.exe
SRCS = $(wildcard $(SOURCE_DIR)/*.c)
OBJS = $(SRCS:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(TARGET) RESOURCES

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(INCLUDE_DIR)

$(TARGET): $(OBJS) $(BUILD_DIR)/resource.o
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LINKS)

$(BUILD_DIR)/resource.o: $(SOURCE_DIR)/resource.rc | $(BUILD_DIR)
	windres $(SOURCE_DIR)/resource.rc -o $(BUILD_DIR)/resource.o

RESOURCES:
	cp $(RESOURCES_DIR)/* $(BUILD_DIR)

clean:
	rm -rf $(BIN_DIR)