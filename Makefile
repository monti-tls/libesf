# This file is part of libesf.
# 
# Copyright (c) 2019, Alexandre Monti
# 
# libesf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# libesf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with libesf.  If not, see <http://www.gnu.org/licenses/>.
#

# Tools
CCP    ?= g++
FMT     = clang-format
UUIDGEN = dbus-uuidgen
# UUIDGEN = cat /proc/sys/kernel/random/uuid

# Directories
SRC_DIR  = src
INC_DIR  = inc
TMP_DIR  = obj
BIN_DIR  = bin
DOX_DIR  = doxygen
C_EXT    = cpp
S_EXT    = S
H_EXT    = h

# User config
include Makefile.inc

ifeq ($(PRODUCT),)
    $(error "Makefile.inc should define PRODUCT")
endif

ifeq ($(VERSION),)
    $(error "Makefile.inc should define VERSION")
endif

# Configuration
BUILD_ID      := $(shell $(UUIDGEN))

DEFINES       += -DLESF_USER_PROGRAM=\"$(PRODUCT)\"
DEFINES       += -DLESF_USER_BUILD_ID=\"$(BUILD_ID)\"
DEFINES       += -DLESF_USER_VERSION=\"$(VERSION)\"

# Mandatory CC flags
CC_FLAGS += -std=c++11
CC_FLAGS += -Wall -Wextra
CC_FLAGS += $(DEFINES)
CC_FLAGS += -I$(INC_DIR) -I$(SRC_DIR)

# Format flags
FMT_FLAGS = -i -style=file

# Additional macros
define \n


endef

# Sources management
C_SUB  = $(shell find $(SRC_DIR) -type d 2>/dev/null)
H_SUB  = $(shell find $(INC_DIR) -type d 2>/dev/null)

C_SRC  = $(wildcard $(addsuffix /*.$(C_EXT),$(C_SUB)))
C_OBJ  = $(patsubst $(SRC_DIR)/%.$(C_EXT),$(TMP_DIR)/%.o,$(C_SRC))
C_DEP  = $(patsubst $(SRC_DIR)/%.$(C_EXT),$(TMP_DIR)/%.d,$(C_SRC))

S_SRC  = $(wildcard $(addsuffix /*.$(S_EXT),$(C_SUB)))
S_OBJ  = $(patsubst $(SRC_DIR)/%.$(S_EXT),$(TMP_DIR)/%.o,$(S_SRC))
S_DEP  = $(patsubst $(SRC_DIR)/%.$(S_EXT),$(TMP_DIR)/%.d,$(S_SRC))

C_FMT  = $(foreach d,$(C_SUB),$(patsubst $(d)/%.$(C_EXT),$(d)/fmt-%,$(wildcard $(d)/*.$(C_EXT))))
H_FMT  = $(foreach d,$(H_SUB),$(patsubst $(d)/%.$(H_EXT),$(d)/fmt-%,$(wildcard $(d)/*.$(H_EXT))))

# Generated files
LD_SCRIPT     = $(TMP_DIR)/$(PRODUCT).ld
VER_INFO_FILE = $(TMP_DIR)/$(PRODUCT).lesf_verinfo.$(C_EXT)
VER_INFO_OBJ  = $(patsubst %.$(C_EXT),%.o,$(VER_INFO_FILE))

# Product files
BINARY        = $(BIN_DIR)/$(PRODUCT)

# Top-level
.NOTPARALLEL: all
all: binary subdirs

.PHONY: binary
binary: $(LD_SCRIPT) $(BINARY)

.PHONY: doxygen
doxygen:
	@mkdir -p $(DOX_DIR)
	@doxygen Doxyfile

.PHONY: clean
clean:
	@rm -rf $(BIN_DIR) $(TMP_DIR) $(DOX_DIR)
	$(foreach sub,$(SUBDIRS),@$(MAKE) --no-print-directory -C $(sub) clean${\n})

.PHONY: format
format: $(C_FMT) $(H_FMT)

.PHONY: subdirs
subdirs:
	$(foreach sub,$(SUBDIRS),@$(MAKE) --no-print-directory -C $(sub)${\n})

.PHONY: tar
tar: clean $(DIST)

.PHONY: dist
dist: binary
	@[ -z "$(DIST_PREFIX)" ] && { echo "dist: DIST_PREFIX not set"; exit 1; } || true
	@mkdir -p $(DIST_PREFIX)/$(DIST_DIR)
	@echo "$(INDENT)(CP)      $$(basename $(BINARY)) -> $(DIST_PREFIX)/$(DIST_DIR)"
	@cp $(BINARY) $(DIST_PREFIX)/$(DIST_DIR)

# Special targets

define ld_script_contents
SECTIONS
{
    .lesf_verinfo(lesf_verinfo_data) :
    {
        KEEP (*$(VER_INFO_OBJ) (.rodata*, .data*, .sdata*))
    }
}
INSERT AFTER .text;
endef

export ld_script_contents
$(LD_SCRIPT):
	@mkdir -p $(@D)
	@echo "$(INDENT)(GEN)     $@"
	@echo "$$ld_script_contents" >> $@

define ver_info_file_contents
static char __attribute__((section(".lesf_verinfo"))) const build_date[] = __DATE__;
static char __attribute__((section(".lesf_verinfo"))) const build_time[] = __TIME__;
static char __attribute__((section(".lesf_verinfo"))) const user_build_id[] = LESF_USER_BUILD_ID;
static char __attribute__((section(".lesf_verinfo"))) const user_program[] = LESF_USER_PROGRAM;
static char __attribute__((section(".lesf_verinfo"))) const user_version[] = LESF_USER_VERSION;
endef

export ver_info_file_contents
$(VER_INFO_FILE):
	@echo "$(INDENT)(GEN)     $@"
	@echo "$$ver_info_file_contents" >> $@

# Dependencies
-include $(C_DEP)

# Translation
$(BINARY): $(VER_INFO_OBJ) $(C_OBJ) $(S_OBJ)
	@mkdir -p $(@D)
	@echo "$(INDENT)(LD)      $@"
	@$(CCP) -o $@ $^ $(LD_FLAGS)

$(TMP_DIR)/%.o: $(SRC_DIR)/%.$(C_EXT)
	@mkdir -p $(@D)
	@echo "$(INDENT)(CC)      $<"
	@$(CCP) $(CC_FLAGS) -MMD -c $< -o $@

$(TMP_DIR)/%.o: $(TMP_DIR)/%.$(C_EXT)
	@mkdir -p $(@D)
	@echo "$(INDENT)(CC)      $<"
	@$(CCP) $(CC_FLAGS) -MMD -c $< -o $@

$(TMP_DIR)/%.o: $(SRC_DIR)/%.$(S_EXT)
	@mkdir -p $(@D)
	@echo "$(INDENT)(CC)      $<"
	@$(CCP) $(CC_FLAGS) -MMD -c $< -o $@

# Format
fmt-%: %.$(C_EXT)
	@$(FMT) $(FMT_FLAGS) $<
	@echo "$(INDENT)(FMT)     $<"

fmt-%: %.$(H_EXT)
	@$(FMT) $(FMT_FLAGS) $<
	@echo "$(INDENT)(FMT)     $<"
