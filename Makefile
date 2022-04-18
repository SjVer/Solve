########################################################################
####################### Makefile Template ##############################
########################################################################

# Compiler settings - Can be customized.
CC = clang++

MUTE = write-strings varargs # sign-compare unused-function comment dangling-gsl unknown-warning-option c++17-extensions
DEFS = 
CXXFLAGS = -Wall $(addprefix -Wno-,$(MUTE)) $(addprefix -D,$(DEFS))

# Makefile settings - Can be customized.
APPNAME = solve
EXT = .cpp
SRCDIR = src
HEADERDIR = include
BINDIR = bin
OBJDIR = $(BINDIR)/obj

############## Do not change anything from here downwards! #############
SRC = $(wildcard $(SRCDIR)/*$(EXT))
OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)
APP = $(BINDIR)/$(APPNAME)
DEP = $(OBJ:$(OBJDIR)/%.o=%.d)

PCH = $(HEADERDIR)/pch
PCHFLAGS = $(CXXFLAGS) -x c++-header $(PCH)
# INC_PCH_FLAG = -include $(PCH)
INC_PCH_FLAG = -include-pch $(PCH).gch

DEBUGDEFS = -DDEBUG -ggdb

OBJCOUNT_NOPAD = $(shell v=`echo $(OBJ) | wc -w`; echo `seq 1 $$(expr $$v)`)
OBJCOUNT = $(foreach v,$(OBJCOUNT_NOPAD),$(shell printf '%02d' $(v)))

PCHS = pch

PCH_OUT_DIR = $(BINDIR)/pch
PCH_SRC = $(addprefix $(HEADERDIR)/,$(PCHS))
PCH_OUT = $(PCH_SRC:$(HEADERDIR)/%=$(PCH_OUT_DIR)/%.gch)
PCHFLAGS = $(CXXFLAGS) -x c++-header
INC_PCH_FLAG = $(addprefix -include-pch ,$(PCH_OUT))

# UNIX-based OS variables & settings
RM = rm
MKDIR = mkdir
DELOBJ = $(OBJ)
SHELL := /bin/bash

########################################################################
####################### Targets beginning here #########################
########################################################################

.MAIN: $(APP)
all: $(APP)
.DEFAULT_GOAL := $(APP)

# Builds the app
$(APP): $(OBJ) | makedirs
	@printf "[final] compiling final product $(notdir $@)..."
	@$(CC) $(CXXFLAGS) -I$(HEADERDIR)/$(TARGET) -o $@ $^ $(LDFLAGS)
	@printf "\b\b done!\n"

# Building rule for .o files and its .c/.cpp in combination with all .h
# $(OBJDIR)/%.o: $(SRCDIR)/%$(EXT) | makedirs
$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT) | makedirs $(PCH_OUT)
	@printf "[$(word 1,$(OBJCOUNT))/$(words $(OBJ))] compiling $(notdir $<) into $(notdir $@)..."
	@$(CC) $(CXXFLAGS) -I$(HEADERDIR)/$(TARGET) $(INC_PCH_FLAG) -I $(HEADERDIR) -o $@ -c $<
	@printf "\b\b done!\n"
	$(eval OBJCOUNT = $(filter-out $(word 1,$(OBJCOUNT)),$(OBJCOUNT)))

# Builds phc's
pchs: $(PCH_OUT)
$(PCH_OUT_DIR)/%.gch: $(HEADERDIR)/% | makedirs
	@printf "[pchs] compiling $(notdir $<)..."
	@$(CC) $(PCHFLAGS) $^ -o $@
	@printf "\b\b done!\n"


############################################################################

# Cleans complete project
.PHONY: clean
clean:
	@$(RM) -rf $(BINDIR)

.PHONY: makedirs
makedirs:
	@$(MKDIR) -p $(BINDIR)
	@$(MKDIR) -p $(OBJDIR)
	@$(MKDIR) -p $(PCH_OUT_DIR)

.PHONY: remake
remake: clean $(APP)

############################################################################

.PHONY: test
test: $(APP)
	@printf "============= Running \"$(APP)\" =============\n\n"
	@$(APP) test/test.slv $(args)

.PHONY: valgrind
valgrind: debug $(APP)
	@printf "============ Running \"valgrind $(APP) test/test.slv\" ============\n\n"
	@valgrind $(APP) test/test.slv $(args)

############################################################################

.PHONY: printdebug
printdebug:
	@echo "debug mode set!"

debug: CXXFLAGS += $(DEBUGDEFS)
debug: printdebug
debug: $(APP)

############################################################################

git:
	@cd wiki && $(MAKE) --no-print-directory git || true

	git add --all
	git commit -m $$(test "$(msg)" && echo '$(msg)' || echo upload)
	git push origin main

newfile:
	@test $(name) || ( echo "basename not given! ('make newfile name=BASENAME')"; false )
	touch $(SRCDIR)/$(name).cpp
	touch $(HEADERDIR)/$(name).hpp