
CC = SDK:gcc/bin/gcc
LD = SDK:gcc/bin/gcc


OBJ = main.o gui_mgba.o gui_resources.o gui_build.o


BIN = mgbaGUI


OS := $(shell uname)

ifeq ($(strip $(OS)),AmigaOS)
	AMIGADATE = $(shell c:date LFORMAT %d.%m.%Y)
	#YEAR = $(shell c:date LFORMAT %Y)
else
	AMIGADATE = $(shell date +"%-d.%m.%Y")
	#YEAR = $(shell date +"%Y")
endif

DEBUG = -DDEBUG


INCPATH = -I. -Iincludes

CFLAGS = $(DEBUG) $(INCPATH) -Wall -D__AMIGADATE__=\"$(AMIGADATE)\" -gstabs

LDFLAGS = 

LIBS = 
#	add any extra linker libraries you want here

.PHONY: all all-before all-after clean clean-custom realclean

all: all-before $(BIN) all-after

all-before: mgbagui_strings.h
#	You can add rules here to execute before the project is built


all-after:
#	You can add rules here to execute after the project is built

clean: clean-custom
	rm -v $(OBJ)

realclean:
	rm -v $(OBJ) $(BIN) $(BIN).debug

$(BIN): $(OBJ) $(LIBS)
#	You may need to move the LDFLAGS variable in this rule depending on its contents
	@echo "Linking $(BIN)"
	@$(LD) -o $(BIN).debug $(OBJ) $(LDFLAGS) $(LIBS)
#	strip $(BIN).debug -o $(BIN)
	copy $(BIN).debug $(BIN) FORCE

###################################################################
##  Standard rules
###################################################################

# A default rule to make all the objects listed below
# because we are hiding compiler commands from the output

mgbagui_strings.h: mgbagui.cd
	APPDIR:CatComp mgbagui.cd CFILE mgbagui_strings.h


.c.o:
	@echo "Compiling $<"
	@$(CC) -c $< -o $*.o $(CFLAGS)

main.o: main.c

gui_mgba.o: gui_mgba.c includes/includes.h mgbagui_rev.h mgbagui_strings.h

gui_resources.o: gui_resources.c includes/includes.h mgbagui_rev.h mgbagui_strings.h

gui_build.o: gui_build.c includes/gui_leftgadgets.h includes/includes.h mgbagui_rev.h mgbagui_strings.h


###################################################################