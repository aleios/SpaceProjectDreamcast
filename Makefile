KOS_ROMDISK_DIR = romdisk
TARGET = spj.elf
DISC_NAME = spj.cdi
SOURCE_DIR = src
BUILD_DIR := build
SOURCES := $(shell find src/ -type f -name '*.c' | sort)
SRC_OBJS := $(SOURCES:src/%.c=$(BUILD_DIR)/%.o)
SRC_DEPS := $(SOURCES:src/%.c=$(BUILD_DIR)/%.d)
OBJS := $(SRC_OBJS) romdisk.o

# Local ADX
ADX_DIR := externals/libadx
ADX_O := $(ADX_DIR)/libadxl.o
ADX_LIB := $(ADX_DIR)/libADXL.a

CFLAGS += -I$(KOS_BASE)/utils -gz -std=gnu23 -O3 -fvisibility=hidden -flto -MMD -MP -I$(ADX_DIR)/include
LDFLAGS += -flto -L$(ADX_DIR) -lADXL

include $(KOS_BASE)/Makefile.rules

# Asset targets
SPRITE_SRCS := $(shell find assets/sprites -type f -name '*.png' | sort)
SPRITES := $(SPRITE_SRCS:assets/sprites/%.png=romdisk/sprites/%.dt)
romdisk/sprites/%.dt: assets/sprites/%.png
	@mkdir -p $(dir $@)
	@echo "[Texture Convert] $@"
	$(KOS_BASE)/utils/pvrtex/pvrtex -i $< -o $@ -f ARGB1555 -p staging/pv.png

ANIM_SRCS := $(shell find assets/animations -type f -name '*.json' | sort)
ANIMS := $(ANIM_SRCS:assets/animations/%.json=romdisk/animations/%.anim)
romdisk/animations/%.anim: assets/animations/%.json
	@mkdir -p $(dir $@)
	python tools/convert_animation.py $< $@

ENEMYDEF_SRCS := $(shell find assets/defs/enemy -type f -name '*.json' | sort)
ENEMYDEFS := $(ENEMYDEF_SRCS:assets/defs/enemy/%.json=romdisk/defs/enemy/%.dat)
romdisk/defs/enemy/%.dat: assets/defs/enemy/%.json
	@mkdir -p $(dir $@)
	python tools/convert_enemy_def.py $< $@

PROJECTILEDEF_SRCS := $(shell find assets/defs/projectile -type f -name '*.json' | sort)
PROJECTILEDEFS := $(PROJECTILEDEF_SRCS:assets/defs/projectile/%.json=romdisk/defs/projectile/%.dat)
romdisk/defs/projectile/%.dat: assets/defs/projectile/%.json
	@mkdir -p $(dir $@)
	python tools/convert_projectile_def.py $< $@

LEVEL_SRCS := $(shell find assets/levels -type f -name '*.json' | sort)
LEVELS := $(LEVEL_SRCS:assets/levels/%.json=romdisk/levels/%.dat)
romdisk/levels/%.dat: assets/levels/%.json
	@mkdir -p $(dir $@)
	python tools/convert_level.py $< $@

GAMESETTINGS := romdisk/settings.dat
romdisk/settings.dat: assets/settings.json
	@mkdir -p $(dir $@)
	python tools/convert_game_settings.py $< $@

FONT_SRCS := $(shell find assets/fonts -type f -name '*.json' | sort)
FONTS := $(FONT_SRCS:assets/fonts/%.json=romdisk/fonts/%.dat)
romdisk/fonts/%.dat: assets/fonts/%.json
	@mkdir -p $(dir $@)
	python tools/convert_font.py $< $@

ASSETS := $(GAMESETTINGS) $(SPRITES) $(ANIMS) $(ENEMYDEFS) $(PROJECTILEDEFS) $(LEVELS) $(FONTS)

# Build targets
.PHONY: all clean rebuild-cdi rm-elf rm-disc

all: $(ADX_LIB) $(TARGET) romdisk.img

$(ADX_O):
	kos-cc -c -flto -O3 -I$(ADX_DIR)/include $(ADX_DIR)/src/libadx.c -o $(ADX_O)

$(ADX_LIB): $(ADX_O)
	kos-ar rcs $(ADX_LIB) $(ADX_O)

debug: CFLAGS += -g -O0
debug: CFLAGS := $(filter-out -O3 -flto, $(CFLAGS))
debug: LDFLAGS := $(filter-out -flto,$(LDFLAGS))
debug: $(TARGET)

clean: rm-elf
	-rm -rf $(BUILD_DIR)
	-rm -rf $(KOS_ROMDISK_DIR)
	-rm -f romdisk.o romdisk.img

rm-disc:
	-rm -f $(DISC_NAME)

rm-elf:
	-rm -f $(TARGET)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	kos-cc $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) $(ADX_LIB)
	kos-cc -o $(TARGET) $(OBJS) $(LDFLAGS)

$(DISC_NAME): $(ADX_LIB) $(TARGET)
	$(DC_TOOLS_BASE)/mkdcdisc -a aleios -e $(TARGET) -D disc -o $(DISC_NAME)

romdisk.img: $(ASSETS)

run: $(TARGET)
	$(KOS_LOADER) $(TARGET) -f -m disc

dist: $(TARGET)
	-rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)

cdi: CFLAGS += -DCD_BUILD
cdi: $(DISC_NAME)

emu: cdi
	flycast $(DISC_NAME)

-include $(SRC_DEPS)