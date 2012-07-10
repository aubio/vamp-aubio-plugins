
# Location of our plugins
#
PLUGINDIR	= plugins

# Compile flags
#
#CXXFLAGS	:= -I../ -I../inst/include $(CXXFLAGS) -fPIC -DNDEBUG -O2 -Wall -I.
CXXFLAGS	:= -I../ -I../inst/include $(CXXFLAGS) -fPIC -DDEBUG -g -Wall -I.

# Libraries required for the plugins.
#
#PLUGIN_LIBS	= -L../inst/lib -lvamp-sdk -laubio 
PLUGIN_LIBS	= ../vamp-plugin-sdk/libvamp-sdk.a /usr/local/lib/libaubio.a /usr/lib/libfftw3f.a

# Flags required to tell the compiler to make a dynamically loadable object
#
PLUGIN_LDFLAGS	= -shared -Wl,-Bsymbolic -Wl,--version-script=vamp-plugin.map

# File extension for a dynamically loadable object
#
PLUGIN_EXT	= .so

## For OS/X with g++:
#PLUGIN_LDFLAGS	= -dynamiclib -exported_symbols_list=vamp-plugin.list
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


