var oCPSet = Selection(0);
// Edam shot path
oCPSet.AddParameter("EdamSequence",siString);
oCPSet.AddParameter("EdamShot",siString);
// Alternative Folder
oCPSet.AddParameter("OutputFolder",siString);
// OutputFolder Selection
oCPSet.AddParameter3("OutputFolderSelection",siInt4,0,0,2,false,false);
// Presets Management
oCPSet.AddParameter3("Presets", siString);
oCPSet.AddParameter3("NewPresetName", siString);