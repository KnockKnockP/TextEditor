CXX = gcc
WINDRES = windres
WINDRESFLAGS =
UICC = "/c/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/x86/uicc.exe"
I686_TOOLS = ./Tools/i686-8.1.0-release-win32-dwarf-rt_v6-rev0/bin
OLD_TOOLS_DIR = ./Tools/gcc_win98/bin

ifdef OLD_TOOLS
    CXX = $(OLD_TOOLS_DIR)/gcc.exe
else ifeq ($(BIT),32)
    CXX = $(I686_TOOLS)/gcc.exe
endif

UNICODE = -DUNICODE -D_UNICODE -municode
UNICODE_BUILD_PATH = UNICODE
CRT_MAIN = -UCRT_MAIN
WINDOWS_VERSION = 0x0600
ARCH=native
BIT=64

ifeq ($(BIT),32)
    WINDRESFLAGS += -F pe-i386
endif

ifdef NO_UNICODE
    UNICODE =
    UNICODE_BUILD_PATH = ANSI
endif

SOURCE_DIR = ./TextEditor.Win32
INCLUDE_DIR = $(SOURCE_DIR)
RESOURCES_DIR = $(SOURCE_DIR)/Resources
BIN_DIR = ./bin
BUILD_DIR = $(BIN_DIR)/$(ARCH)/$(WINDOWS_VERSION)/$(UNICODE_BUILD_PATH)/$(BIT)

LINKS = $(BUILD_DIR)/resource.o $(BUILD_DIR)/ribbon.o -lcomctl32 -ldwmapi -luxtheme -lole32 -luuid
ifdef USE_CRT_MAIN
    CRT_MAIN = -DCRT_MAIN -ffreestanding
    LINKS += -nostdlib -lmsvcrt -lkernel32 -luser32 -lgdi32 -limm32 -lcomdlg32 -ladvapi32
endif

CXXFLAGS = -Wall -Wextra -pedantic -O3 -Ofast -std=gnu++98 -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit $(UNICODE) $(CRT_MAIN) -D_WIN32_WINNT=$(WINDOWS_VERSION) -march=$(ARCH) -mtune=$(ARCH) -m$(BIT) -mwindows

TARGET = $(BUILD_DIR)/TextEditor.exe
SRCS = $(wildcard $(SOURCE_DIR)/*.cpp)
OBJS = $(SRCS:$(SOURCE_DIR)/%.cpp=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(TARGET) RESOURCES

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(INCLUDE_DIR)

$(TARGET): $(OBJS) $(BUILD_DIR)/resource.o $(BUILD_DIR)/ribbon.o
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LINKS)

$(BUILD_DIR)/resource.o: $(SOURCE_DIR)/resource.rc | $(BUILD_DIR)
	$(WINDRES) $(WINDRESFLAGS) $(SOURCE_DIR)/resource.rc -o $(BUILD_DIR)/resource.o

$(BUILD_DIR)/ribbon.o: $(SOURCE_DIR)/ribbon.rc | $(BUILD_DIR)
	$(UICC) $(SOURCE_DIR)/ribbon.xml ribbon.bml /header:ribbon.h /res:ribbon.rc
	mv ./ribbon.bml ./ribbon.h ./ribbon.rc ./$(SOURCE_DIR)/
	$(WINDRES) $(WINDRESFLAGS) $(SOURCE_DIR)/ribbon.rc -o $(BUILD_DIR)/ribbon.o

RESOURCES:
	cp $(RESOURCES_DIR)/* $(BUILD_DIR)

clean:
	rm -rf $(BIN_DIR)
