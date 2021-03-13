from win32com.client import constants
from Globals import *
import os
import stat
import shutil


def FolderExists(folder):
	""" Does folder exists on disk
	:return bool: folder exists
	"""
	return os.path.isdir(folder)


def FileExists(filename):
	""" Does folder exists on disk
	:return bool: file exists
	"""
	return os.path.isfile(filename)


def IsFileReadOnly(filename):
	""" Is read-only file
	:return bool: is read-only file
	"""
	return not (os.stat(filename).st_mode & stat.S_IWRITE)


def IsFileValid(filename):
	""" Is file valid
	:return bool: is file valid (>0kb)
	"""
	return os.path.getsize(filename) > 0


def CopyFiles(sourcePath, targetPath, checkExist=False):
	""" Copy files from source path to target path
	:param str sourcePath: source directory
	:param str targetPath: target directory
	:param bool checkExist: check for existing file and ask for overwrite
	"""
	for root, dirs, files in os.walk(sourcePath):
		# figure out where we're going
		destination = '{}{}'.format(targetPath, root.replace(sourcePath, ''))

		# loop through all files in the directory
		for _file in files:
			# compute current & new file locations
			old_location = os.path.join(root, _file)
			new_location = os.path.join(destination, _file)
			
			if checkExist and FileExists(new_location):
				response = XSIUIToolKit.Msgbox(
					'File "{}" already exists, Overwrite?'.format(_file),
					constants.siMsgYesNo | constants.siMsgQuestion,
					'Copy Files'
				)
				if response == constants.siMsgNo:
					continue
					
			try:
				shutil.copy2(old_location, new_location)
			except RuntimeError:
				XSI.LogMessage('Error copying file "{}"'.format(_file), constants.siError)


def CopyOneFile(source, target, checkExist=False):
	""" Copy one file from source path to target path
	:param str source: source filename
	:param str target: target filename
	:param bool checkExist: check for existing file and ask for overwrite
	"""
	if checkExist and FileExists(target):
		response = XSIUIToolKit.Msgbox(
			'File "{}" already exists, Overwrite?'.format(target),
			constants.siMsgYesNo | constants.siMsgQuestion,
			'Copy Files'
		)
		if response == constants.siMsgNo:
			return
	
	try:
		shutil.copy2(source, target)
	except RuntimeError:
		XSI.LogMessage('Error copying file : ' + source, constants.siError)
