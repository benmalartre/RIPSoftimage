# ===============================================
# File Module
# ===============================================
from Globals import *
import os
import stat
import shutil


# -----------------------------------------------
# FolderExists
# -----------------------------------------------
def FolderExists(folder):
	folderExists = os.path.isdir(folder)
	return folderExists	


# -----------------------------------------------
# FileExists
# -----------------------------------------------
def FileExists(file):
	fileExists = os.path.isfile(file)
	return fileExists


# -----------------------------------------------
# IsFileReadOnly
# -----------------------------------------------
def IsFileReadOnly(file):
	fileAtt = os.stat(file).st_mode
	if not fileAtt & stat.S_IWRITE:
		return True
	else:
		return False


# -----------------------------------------------
# IsFileValid (size > 0kb)
# -----------------------------------------------
def IsFileValid(file):
	s = os.path.getsize(file)
	if s == 0:
		return False
	return True


# -----------------------------------------------
# CopyFiles
# -----------------------------------------------
def CopyFiles(sourcepath, targetpath, checkexist=False):
	for root, dirs, files in os.walk(sourcepath):
		# figure out where we're going
		dest = targetpath + root.replace(sourcepath, '')

		# loop through all files in the directory
		for f in files:

			# compute current (old) & new file locations
			oldLoc = root + '\\' + f
			newLoc = dest + '\\' + f
			
			if checkexist and FileExists(newLoc):
				buttonPressed = XSIUIToolKit.Msgbox("File "+ f +" already exists, Overwrite?",
													constants.siMsgYesNo | constants.siMsgQuestion,
													"Copy Files" )
				if buttonPressed == constants.siMsgNo:
					continue
					
			try:
				shutil.copy2(oldLoc, newLoc)
			except:
				xsi.LogMessage('Error copying file : '+ f,constants.siError)


# -----------------------------------------------
# CopyOneFile
# -----------------------------------------------
def CopyOneFile(source, target, checkexist=False):
	if checkexist and FileExists(target):
		buttonPressed = XSIUIToolKit.Msgbox("File " + target +" already exists, Overwrite?",
											constants.siMsgYesNo | constants.siMsgQuestion,
											"Copy Files" )
		if buttonPressed == constants.siMsgNo:
			return
	
	try:
		shutil.copy2(source, target)
	except:
		xsi.LogMessage('Error copying file : '+ source, constants.siError)