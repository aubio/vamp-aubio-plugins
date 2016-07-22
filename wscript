#! /usr/bin/env python

# script to build vamp-aubio-plugin with waf (https://waf.io)

import sys, os, platform

local_aubio_include  = 'contrib/aubio-dist/include'
local_aubio_lib      = 'contrib/aubio/build/src'
local_vamp_include   = 'contrib/vamp-plugin-sdk-2.6'
local_vamp_lib_i686  = 'contrib/vamp-plugin-sdk-2.6-binaries-i686-linux'
local_vamp_lib_amd64 = 'contrib/vamp-plugin-sdk-2.6-binaries-amd64-linux'
local_vamp_lib_osx   = 'contrib/vamp-plugin-sdk-2.6-binaries-osx'
local_vamp_lib_win32 = 'contrib'


def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx')
    local_aubio_stlib    = 'libaubio.a'
    local_vamp_stlib     = 'libvamp-sdk.a'

    if sys.platform.startswith('linux'):
        if platform.machine() == 'x86_64':
            local_vamp_lib = local_vamp_lib_amd64
        elif platform.machine() == 'x86_64':
            local_vamp_lib = local_vamp_lib_i686
    elif sys.platform == 'darwin':
        local_vamp_lib = local_vamp_lib_osx
    elif sys.platform == 'win32':
        local_vamp_lib = local_vamp_lib_win32
        local_vamp_stlib = 'VampPluginSDK.lib'
        local_aubio_stlib = 'aubio.lib'

    local_aubio_stlib = os.path.join(local_aubio_lib, local_aubio_stlib)
    local_vamp_stlib = os.path.join(local_vamp_lib, local_vamp_stlib)

    if os.path.isdir(local_aubio_include) and os.path.isfile(local_aubio_stlib):
        conf.env.append_value('CXXFLAGS', '-I../'+local_aubio_include)
        conf.env.append_value('SHLIB_MARKER', os.path.join('..',local_aubio_stlib))
    else:
        conf.check_cfg (package='aubio', uselib_store='AUBIO',
                args=['--cflags', '--libs'], mandatory=True)

    if os.path.isdir(local_vamp_include):
        conf.env.append_value('CXXFLAGS', '-I../'+local_vamp_include)
        local_vamp_lib = os.path.join(local_vamp_lib, 'libvamp-sdk.a')
        if os.path.isfile(local_vamp_stlib):
            conf.env.append_value('SHLIB_MARKER', os.path.join('..',local_vamp_stlib))
        #conf.check(lib = 'vamp-sdk', mandatory = False)
    else:
        conf.check_cfg (package='vamp-sdk', uselib_store = 'VAMP',
                args=['--cflags','--libs'], mandatory=True)

    if sys.platform.startswith('linux'):
        conf.env['CXXFLAGS'] += ['-Wall', '-Wextra', '-O3', '-msse', '-msse2',
                '-mfpmath=sse', '-ftree-vectorize']
        conf.env.append_value('LINKFLAGS', '-Wl,-z,defs')
        # add plugin.map
        conf.env.append_value('LINKFLAGS', '-Wl,--version-script=../vamp-plugin.map')

def build(bld):
    # Host Library
    plugin_sources = bld.path.ant_glob('plugins/*.cpp')
    plugin_sources += bld.path.ant_glob('*.cpp')

    # rename libvamp-aubio to vamp-plugin binary name
    if sys.platform.startswith('linux'):
        bld.env['cxxshlib_PATTERN'] = '%s.so'
    elif sys.platform.startswith('darwin'):
        bld.env['cxxshlib_PATTERN'] = '%s.dylib'

    bld.program(source = plugin_sources,
               includes = '.',
               target = 'vamp-aubio',
               name = 'vamp-aubio',
               use = ['VAMP', 'AUBIO'],
               features = 'cxx cxxshlib'
               )

    #for k in bld.env.keys():
    #    print ("%s : %s", k, bld.env[k] )
