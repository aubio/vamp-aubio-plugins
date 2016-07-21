# Makefile for vamp-aubio. This file gets included from platform specific makefiles.
#
# To build on OSX, use:
#
#   $ make -f Makefile.osx
#
# To build on linux, use:
#
#   $ make -f Makefile.linux
#
# To build with mingw32, use:
#
#   $ make -f Makefile.mingw

# Defaults, overridden from the platform-specific Makefile
VAMPSDK_DIR	?= ./contrib/vamp-plugin-sdk-2.6
PLUGIN_EXT	?= .so
CXX 		?= g++
CC 		?= gcc

CFLAGS		:= $(ARCHFLAGS) $(CFLAGS)
CXXFLAGS	:= $(CFLAGS) -I$(VAMPSDK_DIR) $(CXXFLAGS)

LDFLAGS		:= $(ARCHFLAGS) -L$(VAMPSDK_DIR) $(LDFLAGS)
PLUGIN_LDFLAGS	:= $(LDFLAGS) $(PLUGIN_LDFLAGS)

# Location of our plugins
#
PLUGINDIR	= plugins

PLUGIN_OBJECTS	= libmain.o $(patsubst %.cpp,%.o,$(wildcard $(PLUGINDIR)/*.cpp))
PLUGIN_HEADERS	= $(patsubst %.cpp,%.h,$(wildcard $(PLUGINDIR)/*.cpp))
PLUGIN_TARGET	= vamp-aubio$(PLUGIN_EXT)

# use local aubio, can be build with ./scripts/get_aubio.sh
CXXFLAGS := $(CXXFLAGS) -I./contrib/aubio-dist/include

all:		$(PLUGIN_TARGET)

$(PLUGIN_TARGET):	$(PLUGIN_OBJECTS) $(PLUGIN_HEADERS)
		$(CXX) -o $@ $(PLUGIN_OBJECTS) $(PLUGIN_LDFLAGS) $(PLUGIN_LIBS)

clean:
		rm -f $(PLUGIN_OBJECTS)

distclean:	clean
		rm -f $(PLUGIN_TARGET) *~ */*~

