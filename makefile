MKDIR_P = cmd.exe /c mkdir


TARGET_EXEC ?= telemetrycore 

BUILD_DIR ?= ./bin
SRC_DIRS ?= ./src ./lib

C_V ?= 17
CPP_V ?= 17


# Exclude serial osx 		note: can't we just wrap serialosx header in an ifdef __APPLE__? -jus
SRCS := $(shell find $(SRC_DIRS) -type f \( -name "*.cc" -or -name "*.c" -or -name "*.s" \) ! -path "*/lib/serialosx/*")


OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)


INC_DIRS := $(SRC_DIRS) $(shell powershell -Command "Get-ChildItem -Path ./lib -Recurse -Directory | Select-Object -ExpandProperty FullName")
# Define the include directories (INC_DIRS) by searching for all subdirectories within the specified source directories.


INC_FLAGS := $(addprefix -I,$(INC_DIRS)) 

CFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c$(C_V) -g -O3 -Wno-format-security -Wall -Wextra -pedantic-errors -Weffc++ -Wno-unused-parameter
# Adding _WEBSOCKETPP_MINGW_THREAD_ to the CFLAGS to make it deffined in the build and make windows stick with it 
CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c++$(CPP_V) -g -O3 -Wno-format-security -Wall -Wextra -pedantic-errors -Weffc++ -Wno-unused-parameter -D_WEBSOCKETPP_CPP11_THREAD_
CXXFLAGS += -Wno-effc++ -Wno-template-id-cdtor #only show errors and remove warnings for now, delete when done



LDFLAGS := -lws2_32 -lmswsock # taking out ldl and adding winsock2 library


$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) 

# assembly
$(BUILD_DIR)/%.s.o: %.s
	#$(MKDIR_P) $(dir $@)
	-$(MKDIR_P) $(subst /,\,$(dir $@))
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	#$(MKDIR_P) $(dir $@)
	-$(MKDIR_P) $(subst /,\,$(dir $@))
	$(CC) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cc.o: %.cc
	#$(MKDIR_P) $(dir $@)
	-$(MKDIR_P) $(subst /,\,$(dir $@))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean


clean:
	cmd.exe /c if exist $(subst /,\,$(BUILD_DIR)) rmdir /s /q $(subst /,\,$(BUILD_DIR))


-include $(DEPS)
