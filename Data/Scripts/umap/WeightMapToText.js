var g = Selection(0);
var wm = Dictionary.GetObject(g+".polymsh.cls.BlendMapCls.BlendMap");

var cls = wm.Parent;
var subcomponent = cls.CreateSubComponent();
var elemarray = subcomponent.ElementArray.toArray();

var elem = wm.Elements.Array.toArray();

var fso = XSIFactory.CreateActiveXObject("Scripting.FileSystemObject");
var tmp = XSIUtils.Environment("TEMP");
logmessage(tmp);
var filepath = XSIUtils.BuildPath(tmp,"FrancoeurScarf.txt");
var file = fso.CreateTextFile(filepath,true);

file.WriteLine("-------------------------------");
file.WriteLine(elemarray);
file.WriteLine("-------------------------------");
file.WriteLine(elem);
file.WriteLine("-------------------------------");
file.Close();

/*
var str1 = "PaintWeights(\"";
var str2 = "\", Array(";
var str3 = "), Array(";
var str4 = "), 0, 0, null);";
*/

