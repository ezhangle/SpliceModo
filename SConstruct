#
# Copyright 2010-2014 Fabric Software Inc. All rights reserved.
#

import os, sys, platform

if not platform.system().lower().startswith('win'):
  # todo: other operating systems
  raise Exception("This extension only builds for Windows.")

thirdpartyDirs = {
  'FABRIC_DIR': "Should point to Fabric Engine's installation folder.",
  'MODO_SDK_DIR': "Should point to the root of the Oculus SDK."
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


# env.Append(CPPPATH = [os.path.join(os.environ['FABRIC_DIR'], 'include')])
# env.Append(CPPPATH = [os.path.join(os.environ['OCULUS_DIR'], 'LibOVR', 'Include')])
# env.Append(LIBPATH = [os.path.join(os.environ['OCULUS_DIR'], 'LibOVR', 'Lib', 'x64', 'VS2010')])
# env.Append(CPPPATH = [os.environ['BOOST_INCLUDE_DIR']])
# env.Append(LIBPATH = [os.environ['BOOST_LIBRARY_DIR']])

# alias = SConscript('src/SConscript', variant_dir = 'build', exports = {'parentEnv': env, 'STAGE_DIR': env.Dir('stage')}, duplicate=0)

# env.Default(alias)
