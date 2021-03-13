# Import the Softimage constants module
from win32com.client import constants

kitchen_set = r'C:/Users/graph/Documents/bmal/src/USD_ASSETS/Kitchen_set/Kitchen_set.usd'
root = Application.ActiveSceneRoot
prim = root.AddPrimitive('UsdPrimitive')
prim.Parameters('filename').Value = kitchen_set