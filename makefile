UNAME := $(shell uname)


MKDIR_P = mkdir -p
BUILD_DIR ?= ./bin
SRC_DIRS ?= ./src ./lib

TARGET_EXEC ?= telemetrycore 


C_V ?= 17
CPP_V ?= 17



SRCS := $(shell find $(SRC_DIRS) -name *.cc -or -name *.c -or -name *.s)

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)


INC_DIRS := $(shell find $(SRC_DIRS) -type d)

INC_FLAGS := $(addprefix -I,$(INC_DIRS)) 

CFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c$(C_V) -g  -Wno-format-security -Wall -Wextra -pedantic-errors -Weffc++ -Wno-unused-parameter
# Adding _WEBSOCKETPP_MINGW_THREAD_ to the CFLAGS to make it deffined in the build and make windows stick with it 
CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c++$(CPP_V) -g -O3 -Wno-format-security -Wall -Wextra -pedantic-errors -Weffc++ -Wno-unused-parameter -D_WEBSOCKETPP_CPP11_THREAD_
CXXFLAGS += -Wno-effc++ -Wno-template-id-cdtor #only show errors and remove warnings for now, delete when done



LDFLAGS := -ldl

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

