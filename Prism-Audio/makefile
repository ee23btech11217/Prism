# makefile

INCLUDES = -I./ -I./libs/
DEFINES = 

CCX := g++
LD := g++
LINKFLAGS := 
CPPFLAGS := -Wall -Wextra -Iinclude -fdiagnostics-color=always -std=c++17 $(INCLUDES) $(DEFINES) -MMD -MP 

BUILD_DIR := build


DEBUG_LIBS := -lm
DEBUG_CPPFLAGS := $(CPPFLAGS) -g
DEBUG_LINKFLAGS := $(LINKFLAGS) -g
DEBUG_DIR := $(BUILD_DIR)/debug

RELEASE_LIBS := -lm
RELEASE_CPPFLAGS := $(CPPFLAGS) -O3
RELEASE_LINKFLAGS := $(LINKFLAGS)
RELEASE_DIR := $(BUILD_DIR)/release

# Make does not offer a recursive wildcard function, so here's one:
# DO NOT ADD SPACES TO ARGS WHILE CALLING
# leave 2nd arg empty to search from this directory
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

BUILD_EXCLUDE := src/utils/pab_parser.cpp tests/main.cpp src/core/chaotic_mixer.cpp tests/channelTestFul.cpp
RELEASE_EXCLUDE := src/utils/pab_parser.cpp tests/main.cpp src/core/chaotic_mixer.cpp tests/channelTestFul.cpp

DEBUG_SRC_CPP := $(filter-out $(BUILD_EXCLUDE),$(call rwildcard,,*.cpp))
DEBUG_OBJ_CPP := $(patsubst %.cpp, $(DEBUG_DIR)/objs/%.o, $(DEBUG_SRC_CPP))
DEBUG_DEPENDS := $(patsubst %.cpp,$(DEBUG_DIR)/objs/%.d,$(DEBUG_SRC_CPP))

RELEASE_SRC_CPP := $(filter-out $(RELEASE_EXCLUDE),$(call rwildcard,,*.cpp))
RELEASE_OBJ_CPP := $(patsubst %.cpp, $(RELEASE_DIR)/objs/%.o, $(RELEASE_SRC_CPP))
RELEASE_DEPENDS := $(patsubst %.cpp,$(RELEASE_DIR)/objs/%.d,$(RELEASE_SRC_CPP))

# Test configuration
TEST_DIR := tests
# TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_SRCS := $(tests/channelTestFul.cpp)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp, $(DEBUG_DIR)/objs/$(TEST_DIR)/%.o, $(TEST_SRCS))

CORE_TEST_SRCS := src/core/helper.cpp src/core/resources.cpp src/core/channel.cpp src/audioFX/chainFX.cpp tests/channelTestFul.cpp
CORE_TEST_OBJS := $(patsubst %.cpp, $(DEBUG_DIR)/objs/test_%.o, $(CORE_TEST_SRCS))

.PHONY: all debug release test clean clean-tests

all: build_debug

debug: build_debug
release: build_release

build_debug: $(DEBUG_DIR)/prog
build_release: $(RELEASE_DIR)/prog

$(DEBUG_DIR)/prog: $(DEBUG_OBJ_CPP)
	@echo "Linking Project(Debug)"
	@$(LD) -o $@ $^ $(DEBUG_LIBS) $(DEBUG_LINKFLAGS)
	@echo "Created " $@
	@cp $@ $(BUILD_DIR)/prog

-include $(DEBUG_DEPENDS)

$(DEBUG_DIR)/objs/%.o: %.cpp makefile
	@mkdir -p $(@D)
	@echo "[Debug] Compiling: " $<
	@$(CCX) $(DEBUG_CPPFLAGS) -c -o $@ $<


$(RELEASE_DIR)/prog: $(RELEASE_OBJ_CPP)
	@echo "Linking Project(Release)"
	@$(LD) -o $@ $^ $(RELEASE_LIBS) $(RELEASE_LINKFLAGS)
	@echo "Created " $@

-include $(RELEASE_DEPENDS)

$(RELEASE_DIR)/objs/%.o: %.cpp makefile
	@mkdir -p $(@D)
	@echo "[Release] Compiling: " $<
	@$(CCX) $(RELEASE_CPPFLAGS) -c -o $@ $<

clean:
	@rm -r $(BUILD_DIR)


test: clean-tests $(TEST_OBJS) $(CORE_TEST_OBJS)
	@echo "Linking Test Executable"
	@mkdir -p $(BUILD_DIR)
	@$(LD) -o $(BUILD_DIR)/prism_tests $(CORE_TEST_OBJS) $(TEST_OBJS) $(DEBUG_LIBS) $(DEBUG_LINKFLAGS)
	@echo "Running tests..."
	@$(BUILD_DIR)/prism_tests

clean-tests:
	@rm -rf $(DEBUG_DIR)/objs/$(TEST_DIR)
	@rm -f $(CORE_TEST_OBJS)
	@rm -f $(BUILD_DIR)/prism_tests

$(DEBUG_DIR)/objs/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "[Test] Compiling: " $<
	@$(CCX) $(DEBUG_CPPFLAGS) -c -o $@ $<

$(DEBUG_DIR)/objs/test_%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "[Test] Compiling Core: " $<
	@$(CCX) $(DEBUG_CPPFLAGS) -c -o $@ $<