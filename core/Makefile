# (C)2004-2008 SourceMod Development Team
# Makefile written by David "BAILOPAN" Anderson

SRCDS_BASE = ~/srcds
HL2SDK_OB = ../../hl2sdk-ob
HL2SDK_L4D = ../../hl2sdk-l4d

#####################################
### EDIT BELOW FOR OTHER PROJECTS ###
#####################################

OBJECTS = metamod.cpp 				\
		  metamod_util.cpp 			\
		  metamod_console.cpp 		\
		  metamod_oslink.cpp 		\
		  metamod_plugins.cpp 		\
		  sourcehook/sourcehook.cpp \
		  sourcehook/sourcehook_hookmangen.cpp \
		  provider/console.cpp 		\
		  provider/provider_ep2.cpp \
		  provider/vsp_listener.cpp

##############################################
### CONFIGURE ANY OTHER FLAGS/OPTIONS HERE ###
##############################################

C_OPT_FLAGS = -DNDEBUG -O3 -funroll-loops -pipe -fno-strict-aliasing
C_DEBUG_FLAGS = -D_DEBUG -DDEBUG -g -ggdb3
C_GCC4_FLAGS = -fvisibility=hidden
CPP_GCC4_FLAGS = -fvisibility-inlines-hidden
CPP = gcc-4.1

override ENGSET = false
ifeq "$(ENGINE)" "orangebox"
	HL2SDK = $(HL2SDK_OB)
	HL2PUB = $(HL2SDK_OB)/public
	HL2LIB = $(HL2SDK_OB)/lib/linux
	INCLUDE += -I$(HL2SDK_OB)/public/game/server
	SRCDS = $(SRCDS_BASE)/orangebox
	BINARY = metamod.2.ep2.so
	override ENGSET = true
endif
ifeq "$(ENGINE)" "left4dead"
	HL2SDK = $(HL2SDK_L4D)
	HL2PUB = $(HL2SDK_L4D)/public
	HL2LIB = $(HL2SDK_L4D)/lib/linux
	INCLUDE += -I$(HL2SDK_L4D)/public/game/server
	SRCDS = $(SRCDS_BASE)/l4d_demo
	BINARY = metamod2.l4d.so
	override ENGSET = true
endif

LINK += $(HL2LIB)/tier1_i486.a vstdlib_i486.so tier0_i486.so -static-libgcc

	
INCLUDE += -I. -I.. -I$(HL2PUB) -I$(HL2PUB)/engine -I$(HL2PUB)/mathlib -I$(HL2PUB)/vstdlib \
	-I$(HL2PUB)/tier0 -I$(HL2PUB)/tier1 -I. -Isourcehook \

CFLAGS += -D_LINUX -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp \
	-Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca \
	-Dstrcmpi=strcasecmp -Wall -Werror -Wno-uninitialized -mfpmath=sse -msse -DHAVE_STDINT_H -m32
CPPFLAGS += -Wno-non-virtual-dtor -fno-exceptions -fno-rtti

################################################
### DO NOT EDIT BELOW HERE FOR MOST PROJECTS ###
################################################

ifeq "$(DEBUG)" "true"
	BIN_DIR = Debug.$(ENGINE)
	CFLAGS += $(C_DEBUG_FLAGS)
else
	BIN_DIR = Release.$(ENGINE)
	CFLAGS += $(C_OPT_FLAGS)
endif

GCC_VERSION := $(shell $(CPP) -dumpversion >&1 | cut -b1)
ifeq "$(GCC_VERSION)" "4"
	CFLAGS += $(C_GCC4_FLAGS)
	CPPFLAGS += $(CPP_GCC4_FLAGS)
endif

OBJ_LINUX := $(OBJECTS:%.cpp=$(BIN_DIR)/%.o)
OBJ_LINUX := $(OBJ_LINUX:%.c=$(BIN_DIR)/%.o)

$(BIN_DIR)/%.o: %.cpp
	$(CPP) $(INCLUDE) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(BIN_DIR)/%.o: %.c
	$(CPP) $(INCLUDE) $(CFLAGS) -o $@ -c $<

all: check
	mkdir -p $(BIN_DIR)
	mkdir -p $(BIN_DIR)/provider
	mkdir -p $(BIN_DIR)/sourcehook
	ln -sf $(SRCDS)/bin/vstdlib_i486.so vstdlib_i486.so
	ln -sf $(SRCDS)/bin/tier0_i486.so tier0_i486.so
	$(MAKE) -f Makefile metamod

check:
	if [ "$(ENGSET)" == "false" ]; then \
		echo "You must supply ENGINE=left4dead or ENGINE=orangebox"; \
		exit 1; \
	fi

metamod: check $(OBJ_LINUX)
	$(CPP) $(INCLUDE) $(OBJ_LINUX) $(LINK) -m32 -shared -ldl -lm -o$(BIN_DIR)/$(BINARY)

debug:
	$(MAKE) -f Makefile all DEBUG=true

default: all

clean: check
	rm -rf $(BIN_DIR)/*.o
	rm -rf $(BIN_DIR)/sourcehook/*.o
	rm -rf $(BIN_DIR)/provider/*.o
	rm -rf $(BIN_DIR)/$(BINARY)
