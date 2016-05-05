#!/usr/bin/env python
# encoding: utf-8

import os

print "start executing setup_script.py"

lx.eval("dialog.setup style:{warning}")
lx.eval("dialog.title {Fabric Engine}")
fabricDir = os.getenv('FABRIC_DIR', "")

if fabricDir == "":

	errMsg1 = "ERROR:\nThe environment variable FABRIC_DIR\nis not set on your system!"
	errMsg2 = "Please set the FABRIC_DIR environment\nvariable before running this script."
	print errMsg1
	print errMsg2
	lx.eval("dialog.msg {" + errMsg1 + "}")
	lx.eval("dialog.open")
	lx.eval("dialog.msg {" + errMsg2 + "}")
	lx.eval("dialog.open")
	
else:

	print "FABRIC_DIR set to " + fabricDir

	userPath = ""
	ps = lx.service.Platform()
	for i in range(ps.PathCount()):
		if ps.PathNameByIndex(i) == 'user':
			userPath = ps.PathByIndex(i)

	if userPath == "":
	
		errMsg = "Unable to find the user path!"
		print errMsg
		lx.eval("dialog.msg {" + errMsg + "}")
		lx.eval("dialog.open")
		
	else:

		userPath = userPath + "/Kits/FabricLoader"
		if not os.path.exists(userPath):
			os.makedirs(userPath)

		filename = userPath + "/index.cfg"
		f = open(filename, "w")
		print f

		f.write('<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n')
		f.write('<configuration kit=\"FabricLoader\" version=\"1.0\">\n')
  		f.write('  <import>' + fabricDir + '/DCCIntegrations</import>\n')
		f.write('</configuration>\n')

		msg  = " \n"
		msg += "The FabricLoader kit has been successfully\n"
		msg += "installed in your Modo user folder.\n"
		msg += " \n"
		msg += "Please re-start Modo finalize the setup.\n"
		msg += " \n"
		msg += "Have fun with Fabric for Modo!\n"
		msg += " \n"
		lx.eval("dialog.setup style:{info}")
		lx.eval("dialog.msg {" + msg + "}")
		lx.eval("dialog.open")

print "end executing setup_script.py"












