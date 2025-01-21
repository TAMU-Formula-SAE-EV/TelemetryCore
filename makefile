UNAME := $(shell uname)

TARGET_EXEC ?= telemetrycore 

BUILD_DIR ?= ./bin
SRC_DIRS ?= ./src ./lib

C_V ?= 17
CPP_V ?= 20

SRCS := $(shell find $(SRC_DIRS) -name *.cc -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c$(C_V) -g -O3 -Wno-format-security -Wall -Wextra -pedantic-errors -Weffc++ -Wno-unused-parameter
CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c++$(CPP_V) -g -O3 -Wno-format-security -Wall -Wextra -pedantic-errors -Weffc++ -Wno-unused-parameter 

ifeq ($(UNAME),MINGW64_NT-10.0-19043)
	LDFALGS ?= -L/c/MinGW/msys/1.0/lib/libdl.a
else
ifeq ($(UNAME),MINGW32_NT-6.2)
	LDFLAGS ?= -L/lib/libdl.a
else
	LDFLAGS ?= -ldl
endif
endif

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) 

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cc.o: %.cc
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p