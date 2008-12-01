
# Location of our plugins
#
PLUGINDIR	= plugins

# Compile flags
#
CXXFLAGS	:= $(CXXFLAGS) -fPIC -DNDEBUG -O2 -Wall -I.

# Libraries required for the plugins.  Note that we can (and actively
# want to) statically link libstdc++, because our plugin exposes only
# a C API so there are no boundary compatibility problems.
#
PLUGIN_LIBS	= -Wl,-Bstatic -lvamp-sdk -laubio -lfftw3f -Wl,-Bdynamic

# Flags required to tell the compiler to make a dynamically loadable object
#
PLUGIN_LDFLAGS	= -shared -Wl,-Bsymbolic -static-libgcc -Wl,--version-script=vamp-plugin.map

# File extension for a dynamically loadable object
#
PLUGIN_EXT	= .so

## For OS/X with g++:
#PLUGIN_LDFLAGS	= -dynamiclib
#PLUGIN_EXT	= .dylib


### End of user-serviceable parts

PLUGIN_OBJECTS	= libmain.o $(patsubst %.cpp,%.o,$(wildcard $(PLUGINDIR)/*.cpp))
PLUGIN_HEADERS	= $(patsubst %.cpp,%.h,$(wildcard $(PLUGINDIR)/*.cpp))
PLUGIN_TARGET	= vamp-aubio$(PLUGIN_EXT)

all:		$(PLUGIN_TARGET)

$(PLUGIN_TARGET):	$(PLUGIN_OBJECTS) $(PLUGIN_HEADERS)
		$(CXX) $(LDFLAGS) $(PLUGIN_LDFLAGS) -o $@ $(PLUGIN_OBJECTS) $(PLUGIN_LIBS)

clean:		
		rm -f $(PLUGIN_OBJECTS)

distclean:	clean
		rm -f $(PLUGIN_TARGET) *~ */*~


