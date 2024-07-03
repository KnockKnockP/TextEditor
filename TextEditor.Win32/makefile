BUILD_DIR := ./build
SRC_DIR := ./TextEditor.Win32
SRCS := #(shell find $(SRC_DIR) -name '*.cpp')
OBJS:= $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
CPPFLAGS := -Wall -Wextra -pedantic -O3

$(BUILD_DIR)/TextEditor.exe: $(OBJS)
	$(CXX) $(OBJS) -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)