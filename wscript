#! /usr/bin/env python

# script to build vamp-aubio-plugin with waf (https://waf.io)

import sys, os, platform

APPNAME = 'vamp-aubio-plugins'

for l in open('VERSION').readlines(): exec (l.strip())

VERSION = '.'.join ([str(x) for x in [
    VAMP_AUBIO_MAJOR_VERSION,
    VAMP_AUBIO_MINOR_VERSION,
    VAMP_AUBIO_PATCH_VERSION
    ]]) + VAMP_AUBIO_VERSION_STATUS

local_aubio_include  = 'contrib/aubio-dist/include'
local_aubio_lib      = 'contrib/aubio-dist/lib'
local_vamp_include   = 'contrib/vamp-plugin-sdk-2.6'
local_vamp_lib_i686  = 'contrib/vamp-plugin-sdk-2.6-binaries-i686-linux'
local_vamp_lib_amd64 = 'contrib/vamp-plugin-sdk-2.6-binaries-amd64-linux'
# using debian/stable (jessie), the stdc++ abi seems broken. recompile it.
local_vamp_lib_mingw = 'contrib/vamp-plugin-sdk-2.6' #-binaries-win32-mingw'
local_vamp_lib_osx   = 'contrib/vamp-plugin-sdk-2.6-binaries-osx'
local_vamp_lib_win32 = 'contrib'


def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    if sys.platform.startswith('win'):
        # build same binary version as current vamp-plugin-sdk
        # currently Visual Studio 2013 (x86),
        conf.env.MSVC_VERSIONS = ['msvc 12.0']
        conf.env.MSVC_TARGETS = ['x86']
        conf.load('msvc')
    else:
        conf.load('compiler_cxx')
    local_aubio_stlib    = 'libaubio.a'
    local_vamp_stlib     = 'libvamp-sdk.a'

    if sys.platform.startswith('linux'):
        if 'mingw' in conf.env.CXX[0]:
            local_vamp_lib = local_vamp_lib_mingw
        elif platform.machine() == 'x86_64':
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

    if conf.env.CXX_NAME != 'msvc':
        conf.env.CXXFLAGS += ['-g', '-Wall', '-Wextra']

    if sys.platform.startswith('linux'):
        conf.env['CXXFLAGS'] += ['-O3', '-msse', '-msse2', '-mfpmath=sse',
                '-ftree-vectorize']
        if 'mingw' in conf.env.CXX[0]:
            conf.env.append_value('LINKFLAGS', '-Wl,--enable-auto-import')
            conf.env.append_value('LINKFLAGS', '-Wl,--retain-symbols-file=../vamp-plugin.list')
        else:
            conf.env.append_value('LINKFLAGS', '-Wl,-z,defs')
            # add plugin.map
            conf.env.append_value('LINKFLAGS', '-Wl,--version-script=../vamp-plugin.map')
    elif sys.platform == 'win32':
        #conf.env.append_value('CXXFLAGS', '/MD')
        conf.env.append_value('CXXFLAGS', '/W4')
        conf.env.append_value('CXXFLAGS', '/EHsc')
        #conf.env.append_value('CXXFLAGS', '/D_CRT_SECURE_NO_WARNINGS')
        #, '/DWIN32', '/D_WINDOWS', '/D_USRDLL', '/D_WINDLL'
        conf.env.append_value('LINKFLAGS', '/EXPORT:vampGetPluginDescriptor')
        conf.env.append_value('LINKFLAGS', '/NODEFAULTLIB:LIBCMT')
        conf.env.append_value('LINKFLAGS', '/NODEFAULTLIB:LIBPCMT')
    elif sys.platform == 'darwin':
        conf.env.FRAMEWORK += ['Accelerate']

def build(bld):
    # Host Library
    plugin_sources = bld.path.ant_glob('plugins/*.cpp')
    plugin_sources += bld.path.ant_glob('*.cpp')

    # rename libvamp-aubio to vamp-plugin binary name
    if 'mingw' in bld.env.CXX[0]:
        bld.env['cxxshlib_PATTERN'] = '%s.dll'
        install_path = None
    elif sys.platform.startswith('linux'):
        bld.env['cxxshlib_PATTERN'] = '%s.so'
        install_path = '${LIBDIR}/vamp'
    elif sys.platform.startswith('darwin'):
        bld.env['cxxshlib_PATTERN'] = '%s.dylib'
        install_path = '/Library/Audio/Plug-Ins/Vamp'
    elif sys.platform.startswith('win32'):
        if platform.machine() == 'AMD64':
             install_path = 'C:\\Program Files (x86)\\Vamp Plugins'
        else:
             install_path = 'C:\\Program Files\\Vamp Plugins'

    bld.program(source = plugin_sources,
               includes = '.',
               target = 'vamp-aubio',
               name = 'vamp-aubio',
               use = ['VAMP', 'AUBIO', 'CBLAS'],
               features = 'cxx cxxshlib',
               install_path = install_path
               )

    if install_path:
        bld.install_files( install_path, ['vamp-aubio.cat', 'vamp-aubio.n3'])

    #for k in bld.env.keys():
    #    print ("%s : %s", k, bld.env[k] )

def dist(ctx):
    ctx.excl  =  '**/.waf-1* **/*~ **/*.pyc **/*.swp **/.lock-w* **/.git*'
    ctx.excl += ' **/**.tar.bz2'
    ctx.excl += ' **/**.zip'
    ctx.excl += ' **/**.o **/**.so'
    ctx.excl += ' contrib/**'
    ctx.excl += ' build/**'
    ctx.excl += ' dist/**'
    ctx.excl += ' **/.travis.yml'
    ctx.excl += ' **/.appveyor.yml'
