BUILD_DIR=build
include $(N64_INST)/include/n64.mk

N64_CFLAGS := $(filter-out -Werror -O2,$(N64_CFLAGS)) -I$(BUILD_DIR) -Os
N64_CXXFLAGS := $(filter-out -Werror -O2,$(N64_CXXFLAGS)) -I$(BUILD_DIR) -Os
OPTIONS :=

GAME ?= wolf

ifdef NOWAIT
    OPTIONS += NOWAIT=true
else
    OPTIONS += NOWAIT=false
endif
ifdef WARP
    OPTIONS += DEVWARP=$(WARP)
endif
ifdef SKILL
    OPTIONS += DEVSKILL=$(SKILL)
endif

ifeq ($(GAME),spear)
  ROMTITLE := "Spear64"
  ROMNAME := spear64
  OPTIONS += CARMACIZED SPEAR GOODTIMES
  assets := audiohed.sod audiot.sod gamemaps.sod maphead.sod \
            vgadict.sod vgahead.sod vgagraph.sod vswap.sod
else ifeq ($(GAME),speardemo)
  ROMTITLE := "SpearDemo64"
  ROMNAME := speardemo64
  OPTIONS += CARMACIZED SPEAR SPEARDEMO
  assets := audiohed.sdm audiot.sdm gamemaps.sdm maphead.sdm \
            vgadict.sdm vgagraph.sdm vgahead.sdm vswap.sdm
else ifeq ($(GAME),wolfdemo)
  ROMTITLE := "WolfDemo64"
  ROMNAME := wolfdemo64
  OPTIONS += CARMACIZED UPLOAD
  assets := audiohed.wl1 audiot.wl1 gamemaps.wl1 maphead.wl1 \
            vgadict.wl1 vgagraph.wl1 vgahead.wl1 vswap.wl1
else ifeq ($(GAME),wolf)
  ROMTITLE := "Wolfenstein64"
  ROMNAME := wolf64
  OPTIONS += CARMACIZED GOODTIMES
  assets := audiohed.wl6 audiot.wl6 gamemaps.wl6 maphead.wl6 vgadict.wl6 \
            vgagraph.wl6 vgahead.wl6 vswap.wl6
else
    $(error Unknown game $(GAME))
endif

OPTIONS += GAMETITLE=$(ROMTITLE) ROMNAME="$(ROMNAME)"

$(shell mkdir -p $(BUILD_DIR))

hash := \#
define nl


endef

CONFIG_H := $(BUILD_DIR)/config.h
C_OPTIONS = $(foreach d,$(OPTIONS),$(hash)define $(subst =, ,$(d))$(nl))

ifeq (,$(wildcard $(CONFIG_H)))
    $(file > $(CONFIG_H),$(C_OPTIONS))
endif
ifneq ($(strip $(shell cat $(CONFIG_H))),$(strip $(C_OPTIONS)))
    $(file > $(CONFIG_H),$(C_OPTIONS))
endif

src := id_ca.c id_in.c id_pm.c id_sd.c id_us.c id_vh.c id_vl.c \
       signon.c wl_act1.c wl_act2.c wl_agent.c wl_atmos.c wl_cloudsky.c \
       wl_debug.c wl_draw.c wl_game.c wl_inter.c wl_main.c wl_menu.c \
       wl_parallax.c wl_plane.c wl_play.c wl_scale.c wl_shade.c wl_state.c \
       wl_text.c wl_utils.c dbopl.cpp n64_main.c

src := $(addprefix src/,$(src))
assets_conv := $(addprefix filesystem/$(GAME)/,$(assets))

all: $(ROMNAME).z64

filesystem/$(GAME)/%: data/%
	@mkdir -p $(dir $@)
	@echo "    [DATA] $@"
	cp "$<" "$@"

$(assets_conv): $(CONFIG_H)

$(BUILD_DIR)/$(ROMNAME).dfs: $(assets_conv)
$(BUILD_DIR)/$(ROMNAME).elf: $(src:%.c=$(BUILD_DIR)/%.o) $(src:%.cpp=$(BUILD_DIR)/%.o)

$(ROMNAME).z64: N64_ROM_TITLE=$(ROMTITLE)
$(ROMNAME).z64: N64_ROM_SAVETYPE=sram1m
$(ROMNAME).z64: N64_ROM_REGIONFREE=1
$(ROMNAME).z64: $(BUILD_DIR)/$(ROMNAME).dfs

clean:
	rm -rf $(BUILD_DIR) filesystem/ $(ROMNAME).z64

-include $(wildcard $(BUILD_DIR)/src/*.d)

.PHONY: all clean
