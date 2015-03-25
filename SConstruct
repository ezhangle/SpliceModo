#
# Copyright 2010-2014 Fabric Software Inc. All rights reserved.
#

import os, sys, platform

if not platform.system().lower().startswith('win'):
  # todo: other operating systems
  raise Exception("This extension only builds for Windows.")

thirdpartyDirs = {
  'FABRIC_DIR': "Should point to Fabric Engine's installation folder.",
  'MODO_SDK_DIR': "Should point to the root of the MODO SDK.",
  'QT_DIR': "Should point to the root of Qt folder.",
  'FABRIC_UI_DIR': "Should point to the root of FabricUI checkout.",
}

# help debug print
if GetOption('help'):
  print ''
  print 'Fabric Engine for Modo build script.'
  print '-----------------------------------'
  print 'Required environment variables: '
  for thirdpartyDir in thirdpartyDirs:
    print thirdpartyDir + ': ' + thirdpartyDirs[thirdpartyDir]
  print ''
  Exit()

# for windows for now use Visual Studio 2010. 
# if you upgrade this you will also have to provide
# boost libs for the corresponding VS version
env = Environment(ENV = os.environ, MSVC_VERSION='10.0')

# find the third party libs
for thirdpartyDir in thirdpartyDirs:
  if not os.environ.has_key(thirdpartyDir):
    raise Exception(thirdpartyDir+' env variable not defined. '+thirdpartyDirs[thirdpartyDir])

env.Append(CPPPATH = [os.path.join(os.environ['FABRIC_UI_DIR'], 'stage', 'include')])
env.Append(CPPPATH = [os.path.join(os.environ['FABRIC_UI_DIR'], 'stage', 'include', 'FabricUI')])
env.Append(LIBPATH = [os.path.join(os.environ['FABRIC_DIR'], 'lib')])
env.Append(LIBPATH = [os.path.join(os.environ['FABRIC_UI_DIR'], 'stage', 'lib')])
env.Append(CPPPATH = [os.path.join(os.environ['MODO_SDK_DIR'], 'include')])
env.Append(CPPPATH = [os.path.join(os.environ['FABRIC_DIR'], 'include')])
env.Append(CPPPATH = [os.path.join(os.environ['FABRIC_DIR'], 'include', 'FabricServices')])
env.Append(CPPDEFINES = ['FEC_SHARED', 'FECS_SHARED'])

# Fabric Engine libraries
env.Append(LIBS = ['FabricSplice-2.0', 'FabricCore-2.0'])
if platform.system().lower().startswith('win'):
  env.Append(LIBS = ['FabricServices-MSVC-'+env['MSVC_VERSION']])
else:
  env.Append(LIBS = ['FabricServices'])
env.Append(LIBS = ['FabricSplitSearch'])

qtDir = os.environ['QT_DIR']
qtFlags = {}
qtMOC = None
if platform.system().lower().startswith('win'):
  # if buildType == 'Debug':
  #   suffix = 'd4'
  # else:
  suffix = '4'
  qtFlags['CPPPATH'] = [os.path.join(qtDir, 'include')]
  qtFlags['LIBPATH'] = [os.path.join(qtDir, 'lib')]
  qtFlags['LIBS'] = ['QtCore'+suffix, 'QtGui'+suffix, 'QtOpenGL'+suffix]
  qtMOC = os.path.join(qtDir, 'bin', 'moc.exe')
elif platform.system().lower().startswith('dar'):
  qtFlags['CPPPATH'] = ['/usr/local/include']
  qtFlags['FRAMEWORKPATH'] = ['/usr/local/lib']
  qtFlags['FRAMEWORKS'] = ['QtCore', 'QtGui', 'QtOpenGL']
  qtMOC = '/usr/local/bin/moc'
elif platform.system().lower().startswith('lin'):
  qtFlags['CPPDEFINES'] = ['_DEBUG']
  qtFlags['CPPPATH'] = ['/usr/include']
  qtFlags['LIBPATH'] = ['/usr/lib']
  qtFlags['LIBS'] = ['QtGui', 'QtCore', 'QtOpenGL']
  qtMOC = '/usr/bin/moc-qt4'

# ui related libraries
env.MergeFlags(qtFlags)
env.Append(LIBS = ['FabricUI'])

qtMOCBuilder = Builder(
  action = [[qtMOC, '-o', '$TARGET', '$SOURCE']],
  prefix = 'moc_',
  suffix = '.cc',
  src_suffix = '.h',
)
env.Append(BUILDERS = {'QTMOC': qtMOCBuilder})

def GlobQObjectHeaders(env, filter):
  headers = Flatten(env.Glob(filter))
  qobjectHeaders = []
  for header in headers:
    content = open(header.srcnode().abspath, 'rb').read()
    if content.find('Q_OBJECT') > -1:
      qobjectHeaders.append(header)
  return qobjectHeaders
Export('GlobQObjectHeaders')
env.AddMethod(GlobQObjectHeaders)

def GlobQObjectSources(env, filter):
  headers = env.GlobQObjectHeaders(filter)
  sources = []
  for header in headers:
    sources += env.QTMOC(header)
  return sources
Export('GlobQObjectSources')
env.AddMethod(GlobQObjectSources)

# standard libraries
if sys.platform == 'win32':
  env.Append(LIBS = ['user32', 'advapi32', 'gdi32', 'shell32', 'ws2_32', 'Opengl32', 'glu32'])
else:
  env.Append(LIBS = ['X11', 'GLU', 'GL', 'dl', 'pthread'])

if sys.platform == 'win32':
  env.Append(CCFLAGS = ['/Od', '/Zi']) # 'Z7'
  env['CCPDBFLAGS']  = ['${(PDB and "/Fd%s.pdb /Zi" % File(PDB)) or ""}']

commonAlias = SConscript('common/SConscript', variant_dir = 'build/common', exports = {
  'parentEnv': env, 
  'STAGE_DIR': env.Dir('stage'), 
  'MODO_SDK_DIR': os.environ['MODO_SDK_DIR']
}, duplicate=0)

pluginFiles = SConscript('src/SConscript', variant_dir = 'build/src', exports = {
  'parentEnv': env, 
  'STAGE_DIR': env.Dir('stage'), 
  'MODO_SDK_DIR': os.environ['MODO_SDK_DIR']
}, duplicate=0)

if sys.platform == 'win32':
  pdbFile = env.Dir('#').File('vc'+env['MSVC_VERSION'].replace('.', '')+'.pdb')
  env.Depends(pdbFile, pluginFiles)
  pluginFiles += env.InstallAs(pluginFiles[0].dir.File('FabricModo.pdb'), pdbFile)

pluginAlias = env.Alias('plugin', pluginFiles)

env.Default(pluginAlias)
