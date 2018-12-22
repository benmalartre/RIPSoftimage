# -------------------------------------------------------------------
# Globals
# -------------------------------------------------------------------
import win32com as win
import win32com.server as winServer

from win32com.client import constants
from win32com.client.dynamic import Dispatch
from win32com.client import Dispatch as ComObject
from win32com.server.util import wrap

xsi = Dispatch('XSI.Application').Application
XSIFactory = Dispatch('XSI.Factory')
XSIMath = Dispatch('XSI.Math')
XSIUIToolKit = Dispatch('XSI.UIToolKit')
XSIUtils = Dispatch('XSI.Utils')
null = None
false = 0
true = 1
