#
# Copyright 2010-2013 Fabric Engine Inc. All rights reserved.
#

import os, sys, platform, copy

Import(
  'parentEnv',
  'FABRIC_DIR',
  'STAGE_DIR',
  'FABRIC_BUILD_OS',
  'FABRIC_BUILD_TYPE',
  'FABRIC_BUILD_ARCH',
  'MODO_BIN_DIR',
  'MODO_LIB_DIR',
  'MODO_INCLUDE_DIR',
  'MODO_VERSION',
  'sharedCapiFlags',
  'ADDITIONAL_FLAGS',
  'commandsFlags',
  'astWrapperFlags',
  'codeCompletionFlags'
  )

env = parentEnv.Clone()

qtMOCBuilder = Builder(
  action = [[os.path.join(MODO_BIN_DIR, 'moc'), '-o', '$TARGET', '$SOURCE']],
  prefix = 'moc_',
  suffix = '.cc',
  src_suffix = '.h',
)
env.Append(BUILDERS = {'QTMOC': qtMOCBuilder})

modoFlags = {
  'CPPPATH': [
      MODO_INCLUDE_DIR,
      os.path.join(os.path.join(os.path.split(str(MODO_BIN_DIR))[0], 'include')),
      env.Dir('lib'),
      env.Dir('plugin')
    ],
  'LIBPATH': [
    MODO_LIB_DIR
  ],
}

modoFlags['LIBS'] = []
if FABRIC_BUILD_OS == 'Windows':
  modoFlags['CPPDEFINES'] = ['NT_PLUGIN']
  modoFlags['LIBS'].extend(['QtCore4', 'QtGui4', 'QtOpenGL4'])
if FABRIC_BUILD_OS == 'Linux':
  modoFlags['CPPDEFINES'] = ['LINUX']
  modoFlags['LIBS'].extend(['QtCore', 'QtGui', 'QtOpenGL'])
if FABRIC_BUILD_OS == 'Darwin':
  modoFlags['CPPDEFINES'] = ['OSMac_']
  qtCoreLib = File(os.path.join(MAYA_LIB_DIR, 'QtCore'))
  qtGuiLib = File(os.path.join(MAYA_LIB_DIR, 'QtGui'))
  qtOpenGLLib = File(os.path.join(MAYA_LIB_DIR, 'QtOpenGL'))
  modoFlags['LIBS'].extend([
    qtCoreLib,
    qtGuiLib,
    qtOpenGLLib,
    File(os.path.join(MAYA_LIB_DIR, 'QtGui'))
    ])

env.MergeFlags(modoFlags)

# services flags
if len(commandsFlags.keys()) > 0:
  env.MergeFlags(commandsFlags)
  env.MergeFlags(codeCompletionFlags)
else:
  if FABRIC_BUILD_OS == 'Windows':
    env.Append(LIBS = ['FabricServices-MSVC-'+env['MSVC_VERSION']])
  else:
    env.Append(LIBS = ['FabricServices'])
  env.Append(LIBS = ['FabricSplitSearch'])


# build the ui libraries for modo
uiLibPrefix = 'uiModo'+str(MODO_VERSION)

uiDir = env.Dir('#').Dir('Native').Dir('FabricUI')
if os.environ.has_key('FABRIC_UI_DIR'):
  uiDir = env.Dir(os.environ['FABRIC_UI_DIR'])
uiSconscript = uiDir.File('SConscript')
if not os.path.exists(uiSconscript.abspath):
  print "Error: You need to have FabricUI checked out to "+uiSconscript.dir.abspath

env.Append(CPPPATH = [os.path.join(os.environ['FABRIC_DIR'], 'include')])
env.Append(LIBPATH = [os.path.join(os.environ['FABRIC_DIR'], 'lib')])
env.Append(CPPPATH = [os.path.join(os.environ['FABRIC_DIR'], 'include', 'FabricServices')])
env.Append(CPPPATH = [uiSconscript.dir])

if FABRIC_BUILD_TYPE == 'Debug':
  env.Append(CPPDEFINES = ['_DEBUG'])
  env.Append(CPPDEFINES = ['_ITERATOR_DEBUG_LEVEL=2'])

uiLibs = SConscript(uiSconscript, exports = {
  'parentEnv': env, 
  'uiLibPrefix': uiLibPrefix, 
  'qtDir': os.path.join(MODO_INCLUDE_DIR, 'Qt'),
  'qtMOC': os.path.join(MODO_BIN_DIR, 'moc'),
  'qtFlags': {

  },
  'fabricFlags': sharedCapiFlags,
  'buildType': FABRIC_BUILD_TYPE,
  'buildOS': FABRIC_BUILD_OS,
  'buildArch': FABRIC_BUILD_ARCH,
  'stageDir': env.Dir('#').Dir('stage').Dir('lib'),
  },
  duplicate=0,
  variant_dir = env.Dir('FabricUI')
  )

# import the modo specific libraries
Import(uiLibPrefix+'Flags')

# ui flags
env.MergeFlags(locals()[uiLibPrefix + 'Flags'])

env.MergeFlags(sharedCapiFlags)
env.MergeFlags(ADDITIONAL_FLAGS)

if FABRIC_BUILD_OS == 'Linux':
  env.Append(LIBS=['boost_filesystem', 'boost_system'])

target = 'FabricModo'

modoModule = None
libSources = env.Glob(os.path.join(str(MODO_INCLUDE_DIR), 'common', '*.cpp'))
libSources += env.Glob('src/*.cpp')
libSources += env.QTMOC(env.File('src/_class_FabricDFGWidget.h'))

libFabricModo = env.StaticLibrary('libFabricModo', libSources)
env.Append(LIBS = [libFabricModo])

pluginSources = env.Glob('plugin/*.cpp')

if FABRIC_BUILD_OS == 'Darwin':
  # a loadable module will omit the 'lib' prefix name on Os X
  spliceAppName = 'FabricModo'+MODO_VERSION
  target += '.bundle'
  env.Append(SHLINKFLAGS = ','.join([
    '-Wl',
    '-current_version',
    '.'.join([env['FABRIC_VERSION_MAJ'],env['FABRIC_VERSION_MIN'],env['FABRIC_VERSION_REV']]),
    '-compatibility_version',
    '.'.join([env['FABRIC_VERSION_MAJ'],env['FABRIC_VERSION_MIN'],'0']),
    '-install_name',
    '@rpath/Splice/Applications/'+spliceAppName+'/plugins/'+spliceAppName+".bundle"
    ]))
  modoModule = env.LoadableModule(target = target, source = pluginSources)
else:
  libSuffix = '.so'
  if FABRIC_BUILD_OS == 'Windows':
    libSuffix = '.mll'
  if FABRIC_BUILD_OS == 'Linux':
    exportsFile = env.File('Linux.exports').srcnode()
    env.Append(SHLINKFLAGS = ['-Wl,--version-script='+str(exportsFile)])
    env[ '_LIBFLAGS' ] = '-Wl,--start-group ' + env['_LIBFLAGS'] + ' -Wl,--end-group'
  modoModule = env.SharedLibrary(target = target, source = pluginSources, SHLIBSUFFIX=libSuffix, SHLIBPREFIX='')

moduleFileModoVersion = MODO_VERSION

modoFiles = []
modoFiles.append(env.Install(STAGE_DIR, modoModule))
modoFiles.append(env.Install(STAGE_DIR, libFabricModo))

# for xpm in ['FE_tool']:
#   modoFiles.append(env.Install(os.path.join(STAGE_DIR.abspath, 'ui'), os.path.join('Module', 'ui', xpm+'.xpm')))
installedModule = env.Install(os.path.join(STAGE_DIR.abspath, 'plug-ins'), modoModule)
modoFiles.append(installedModule)

# # also install the FabricCore dynamic library
# if FABRIC_BUILD_OS == 'Linux':
#   env.Append(LINKFLAGS = [Literal('-Wl,-rpath,$ORIGIN/../../../lib/')])
# if FABRIC_BUILD_OS == 'Darwin':
#   env.Append(LINKFLAGS = [Literal('-Wl,-rpath,@loader_path/../../..')])
# # if FABRIC_BUILD_OS == 'Windows':
# #   FABRIC_CORE_VERSION = FABRIC_SPLICE_VERSION.rpartition('.')[0]
# #   modoFiles.append(
# #     env.Install(
# #       os.path.join(STAGE_DIR.abspath, 'plug-ins'),
# #       os.path.join(FABRIC_DIR, 'lib', 'FabricCore-' + FABRIC_CORE_VERSION + '.dll')
# #       )
# #     )
# #   modoFiles.append(
# #     env.Install(
# #       os.path.join(STAGE_DIR.abspath, 'plug-ins'),
# #       os.path.join(FABRIC_DIR, 'lib', 'FabricCore-' + FABRIC_CORE_VERSION + '.pdb')
# #       )
# #     )

alias = env.Alias('splicemodo', modoFiles)
spliceData = (alias, modoFiles)
Return('spliceData')
