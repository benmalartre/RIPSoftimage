import win32com.client
from win32com.client import constants

import sys

null = None
false = 0
true = 1

def XSILoadPlugin( in_reg ):
    in_reg.Author = "benmalartre"
    in_reg.Name = "Use Python Path"
    in_reg.Major = 1
    in_reg.Minor = 0

    in_reg.RegisterCommand("SetPythonModulePath","SetPythonModulePath")

    wkgrLocations = Application.Workgroups
    for wkgr in wkgrLocations:
        wkgrLibFolder = XSIUtils.BuildPath(wkgr,"Application\\PythonModules")
        if wkgrLibFolder not in sys.path:
            sys.path.append(wkgrLibFolder)
    userLocation = Application.InstallationPath(constants.siUserPath)
    userLibFolder = XSIUtils.BuildPath(userLocation,"Application\\PythonModules")
    if userLibFolder not in sys.path:
        sys.path.append(userLibFolder)
    Application.LogMessage(sys.path, constants.siInfo)

    return true

