#
# Copyright (c) 2012-2026 Daniele Bartolini et al.
# SPDX-License-Identifier: MIT
#

PO_DIR       := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PROJECT_ROOT := $(abspath $(PO_DIR)../../)

LINGUAS  := $(PO_DIR)LINGUAS
POTFILES := $(PO_DIR)POTFILES

TARGETDIR ?= /tmp

POT_SOURCES := $(sed '/^\#/d; /^$$/d' "$(POTFILES)" || echo "")
POT_TARGET  := $(PO_DIR)crown-editor.pot

.PHONY: all
all: pot install

.PHONY: pot
pot: $(POT_TARGET)

$(POT_TARGET): $(POTFILES) $(addprefix $(PROJECT_ROOT)/,$(POT_SOURCES))
	@cd "$(PROJECT_ROOT)" && xgettext -L Vala --from-code=UTF-8 -f tools/po/POTFILES -k_ -kN_ -kC_:1c,2 -kNC_:1c,2 -o tools/po/crown-editor.pot
	@echo "  Generated $(notdir $@)"

LANGUAGES := $(shell sed '/^\#/d; /^$$/d' "$(LINGUAS)")
MO_FILES  := $(foreach lang,$(LANGUAGES),$(TARGETDIR)/po/$(lang)/LC_MESSAGES/crown-editor.mo)

$(TARGETDIR)/po/%/LC_MESSAGES/crown-editor.mo: $(PO_DIR)%.po
	@mkdir -p "$(dir $@)"
	@msgfmt -o "$@" "$<"
	@echo "$(notdir $<)"

.PHONY: install
install: $(MO_FILES)
