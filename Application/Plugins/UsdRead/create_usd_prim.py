# Import the Softimage constants module
from win32com.client import constants

#asset = r'C:/Users/graph/Documents/bmal/src/USD_ASSETS/Kitchen_set/Kitchen_set.usd'
asset = r'C:/Users/graph/Documents/bmal/src/USD_ASSETS/Apple/pancakes.usdz'

root = Application.ActiveSceneRoot
prim = root.AddPrimitive('UsdPrimitive')
prim.Parameters('filename').Value = asset