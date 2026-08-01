#
# Copyright (c) 2012-2026 Daniele Bartolini et al.
# SPDX-License-Identifier: MIT
#

EMPTY :=
SPACE := $(EMPTY) $(EMPTY)
escape_spaces = $(subst $(SPACE),\$(SPACE),$(1))

PROJECT_ROOT := ../..

LINGUAS  := LINGUAS
POTFILES := POTFILES

TARGETDIR ?= /tmp

POT_SOURCES := $(sed '/^\#/d; /^$$/d' "$(POTFILES)" || echo "")
POT_TARGET  := crown-editor.pot

.PHONY: all
all: pot install

.PHONY: pot
pot: $(POT_TARGET)

$(POT_TARGET): $(POTFILES) $(addprefix $(PROJECT_ROOT)/,$(POT_SOURCES))
	@cd "$(PROJECT_ROOT)" && xgettext -L Vala --from-code=UTF-8 -f tools/po/POTFILES -k_ -kN_ -kC_:1c,2 -kNC_:1c,2 -o tools/po/crown-editor.pot
	@echo "  Generated $(notdir $@)"

LANGUAGES := $(shell sed '/^\#/d; /^$$/d' "$(LINGUAS)")
MO_TARGETDIR := $(call escape_spaces,$(TARGETDIR))
MO_FILES     := $(foreach lang,$(LANGUAGES),$(MO_TARGETDIR)/po/$(lang)/LC_MESSAGES/crown-editor.mo)

$(MO_TARGETDIR)/po/%/LC_MESSAGES/crown-editor.mo: %.po
	@mkdir -p "$(TARGETDIR)/po/$*/LC_MESSAGES"
	@msgfmt -o "$@" "$<"
	@echo "$*.po"

.PHONY: install
install: $(MO_FILES)
