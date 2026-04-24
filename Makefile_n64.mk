V=1
SOURCE_DIR=source
PLATFORM_DIR=platform
BUILD_DIR=build
PROJECT_NAME=moonchild

include $(N64_INST)/include/n64.mk

all: $(PROJECT_NAME).z64
.PHONY: all


# Assets

DFS_FILES := $(patsubst assets/%,filesystem/%,$(wildcard assets/**/*.* assets/moonchild/assets/*.*))
DFS_FILES := $(patsubst filesystem/audio/%.wav,filesystem/audio/%.wav64,$(DFS_FILES))
DFS_FILES := $(patsubst filesystem/audio/%.mp3,filesystem/audio/%.wav64,$(DFS_FILES))
DFS_FILES := $(patsubst filesystem/movies/%.mp4,filesystem/movies/%.h264,$(DFS_FILES))

# Audio
filesystem/audio/%.wav64: assets/audio/%.wav
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV) --wav-compress 3 -o filesystem/audio $<

# Movies
filesystem/movies/%.h264: assets/movies/%.mp4
	@mkdir -p "$(dir $@)"
	@echo "    [MOVIE] $@"
	@$(N64_VIDEOCONV) -o filesystem/movies --codec h264 --quality 70 --quick "$<";

# Music
filesystem/audio/%.wav64: assets/audio/%.mp3
	@mkdir -p $(dir $@)
	@echo "    [MUSIC] $@"
	@$(N64_AUDIOCONV) --wav-compress 3 -o filesystem/audio $<

# Other files
filesystem/moonchild/%: assets/moonchild/%
	@mkdir -p $(dir $@)
	@echo "    [OTHER] $@"
	@cp -f $< $@


# Sources

N64_CXXFLAGS += -Iinclude -I$(SOURCE_DIR) -I$(SOURCE_DIR)/framewrk -I$(SOURCE_DIR)/moonchild -I$(PLATFORM_DIR) \
	-Wall -Wextra -Wno-error \
	-Wformat-signedness -fno-common \
	-Wshadow -Wdouble-promotion -Wformat-security -Wformat-overflow -Wformat-truncation \
	-fpermissive -Wno-unused -DPLATFORM_N64

SRCS := $(wildcard $(SOURCE_DIR)/**/*.cpp) $(wildcard $(SOURCE_DIR)/*.cpp) $(wildcard $(PLATFORM_DIR)/n64/*.cpp)

$(BUILD_DIR)/$(SOURCE_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "    [CC] $<"
	@$(N64_CC) -c $(N64_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/$(PLATFORM_DIR)/%.o: $(PLATFORM_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "    [CC] $<"
	@$(N64_CC) -c $(N64_CXXFLAGS) -o $@ $<

$(PROJECT_NAME).z64: N64_ROM_TITLE="Moon Child"
$(PROJECT_NAME).z64: $(BUILD_DIR)/$(PROJECT_NAME).dfs

$(BUILD_DIR)/$(PROJECT_NAME).dfs: $(DFS_FILES)
$(BUILD_DIR)/$(PROJECT_NAME).elf: $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

clean:
	rm -rf $(BUILD_DIR)/* *.z64 filesystem/*
.PHONY: clean

test:
	@$(N64_MKSPRITE) -f CI8 "test.png"
.PHONY: test

-include $(wildcard $(BUILD_DIR)/*.d)