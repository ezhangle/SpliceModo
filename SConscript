#
# Copyright (c) 2010-2016, Fabric Software Inc. All rights reserved.
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
  qtCoreLib = File(os.path.join(MODO_LIB_DIR, 'QtCore.framework/QtCore'))
  qtGuiLib = File(os.path.join(MODO_LIB_DIR, 'QtGui.framework/QtGui'))
  qtOpenGLLib = File(os.path.join(MODO_LIB_DIR, 'QtOpenGL.framework/QtOpenGL'))
  modoFlags['LIBS'].extend([
    qtCoreLib,
    qtGuiLib,
    qtOpenGLLib,
    File(os.path.join(MODO_LIB_DIR, 'QtGui.framework/QtGui'))
    ])

env.MergeFlags(modoFlags)

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

if FABRIC_BUILD_OS == 'Windows':
  env.Append(CCFLAGS = ['/wd4244'])
  env.Append(CCFLAGS = ['/wd4700'])
elif FABRIC_BUILD_OS == 'Linux':
  env.Append(CCFLAGS = ['-Wno-missing-braces'])
elif FABRIC_BUILD_OS == 'Darwin':
  env.Append(CCFLAGS = ['-Wno-#warnings'])
  env.Append(CCFLAGS = ['-Wno-missing-braces'])
  env.Append(CCFLAGS = ['-Wno-tautological-compare'])
  env.Append(CCFLAGS = ['-Wno-unused-private-field'])
  env.Append(CCFLAGS = ['-Wno-dangling-else'])
  env.Append(CCFLAGS = ['-Wno-deprecated-writable-strings'])
  env.Append(CCFLAGS = ['-Wno-unknown-pragmas'])
  env.Append(CCFLAGS = ['-Wno-parentheses'])

target = 'FabricModo'

modoModule = None

env.VariantDir(env.Dir('common'+MODO_VERSION), 
  os.path.join(os.path.split(str(MODO_INCLUDE_DIR))[0], 'common')
)

pluginSources = env.Glob('src/*.cpp')
pluginSources += env.QTMOC(env.File('src/_class_FabricDFGWidget.h'))

# ignore all warnings from Modo code
commonEnv = env.Clone()
if FABRIC_BUILD_OS == 'Linux':
  if '-Wall' in commonEnv['CCFLAGS']:
    commonEnv['CCFLAGS'].remove('-Wall')
  if '-Werror' in commonEnv['CCFLAGS']:
    commonEnv['CCFLAGS'].remove('-Werror')

commonSources = env.Glob(os.path.join(env.Dir('common'+MODO_VERSION).abspath, '*.cpp'))
for commonSource in commonSources:
  fileName = os.path.split(commonSource.abspath)[1]
  if fileName == 'clean.cpp':
    continue
  pluginSources += commonEnv.SharedObject(commonSource)

if FABRIC_BUILD_OS == 'Darwin':
  # a loadable module will omit the 'lib' prefix name on Os X
  spliceAppName = 'FabricModo'+MODO_VERSION
  target += '.lx'
  env.Append(SHLINKFLAGS = ','.join([
    '-Wl',
    '-current_version',
    '.'.join([env['FABRIC_VERSION_MAJ'],env['FABRIC_VERSION_MIN'],env['FABRIC_VERSION_REV']]),
    '-compatibility_version',
    '.'.join([env['FABRIC_VERSION_MAJ'],env['FABRIC_VERSION_MIN'],'0']),
    '-install_name',
    '@rpath/Splice/Applications/'+spliceAppName+'/plugins/'+spliceAppName+".lx"
    ]))
  modoModule = env.LoadableModule(target = target, source = pluginSources, SHLIBSUFFIX='.lx')
else:
  if FABRIC_BUILD_OS == 'Linux':
    exportsFile = env.File('Linux.exports').srcnode()
    env.Append(SHLINKFLAGS = ['-Wl,--version-script='+str(exportsFile)])
    env.Append(LINKFLAGS = [Literal('-Wl,-rpath,$ORIGIN/../../../lib/')])
    # env[ '_LIBFLAGS' ] = '-Wl,--start-group ' + env['_LIBFLAGS'] + ' -Wl,--end-group'
  modoModule = env.SharedLibrary(target = target, source = pluginSources, SHLIBSUFFIX='.lx')

installDir = None
if FABRIC_BUILD_OS == 'Linux':
  installDir = STAGE_DIR.Dir('lnx64')
elif FABRIC_BUILD_OS == 'Windows':
  installDir = STAGE_DIR.Dir('win64')
else:
  installDir = STAGE_DIR.Dir('mac64')

installedModule = env.Install(installDir, modoModule)

FABRIC_CORE_VERSION = '2.2'

modoFiles = []
modoFiles.append(installedModule)
modoFiles += env.Install(STAGE_DIR, env.Dir('src').File('index.cfg'))
modoFiles += env.Install(STAGE_DIR, env.Dir('src').File('startup_script.py'))
modoFiles += env.Install(STAGE_DIR, env.Dir('src').File('btn_FabricCanvasExportGraph.pl'))
modoFiles += env.Install(STAGE_DIR, env.Dir('src').File('btn_FabricCanvasImportGraph.pl'))
modoFiles += env.Install(STAGE_DIR, env.Dir('src').File('btn_FabricCanvasOpenCanvas.pl'))

if FABRIC_BUILD_OS == 'Windows':
  modoFiles += env.Install(installDir, os.path.join(FABRIC_DIR, 'bin', 'FabricCore-' + FABRIC_CORE_VERSION + '.dll'))
  modoFiles += env.Install(installDir, os.path.join(FABRIC_DIR, 'bin', 'FabricCore-' + FABRIC_CORE_VERSION + '.pdb'))
  modoFiles += env.Install(installDir, os.path.join(FABRIC_DIR, 'bin', 'FabricSplitSearch.dll'))
  modoFiles += env.Install(installDir, os.path.join(FABRIC_DIR, 'bin', 'FabricSplitSearch.pdb'))
  modoFiles += env.Install(installDir, os.path.join(FABRIC_DIR, 'bin', 'tbb.dll'))
  modoFiles += env.Install(installDir, os.path.join(FABRIC_DIR, 'bin', 'tbbmalloc.dll'))

if FABRIC_BUILD_OS == 'Darwin':
  env.Append(LINKFLAGS = [Literal('-Wl,-rpath,@loader_path/../../..')])

alias = env.Alias('splicemodo', modoFiles)
spliceData = (alias, modoFiles)
Return('spliceData')
