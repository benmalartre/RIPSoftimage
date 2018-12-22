var g = Selection(0);
var wm = Dictionary.GetObject(g+".polymsh.cls.WeightMapCls.francoeur_scarf_weights");

var fso = XSIFactory.CreateActiveXObject("Scripting.FileSystemObject");

var tmp = XSIUtils.Environment("TEMP");
var filepath = XSIUtils.BuildPath(tmp,"FrancoeurScarf.txt");
var file = fso.OpenTextFile(filepath,1);

var s, sIdx, sWeigths;

s = file.ReadLine();
sIdx = file.ReadLine();
s = file.ReadLine();
sWeights = file.ReadLine();

file.Close();

var aIdx = sIdx.split(",");
var aWeights = sWeights.split(",");
PaintWeights(wm,aIdx, aWeights, 0, 0, null);


