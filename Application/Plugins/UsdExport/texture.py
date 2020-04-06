from pxr import Gf, Kind, Sdf, Usd, UsdGeom, UsdShade

stage = Usd.Stage.CreateNew("texture.usda")
stage.SetStartTimeCode(1)
stage.SetEndTimeCode(24)
#stage.SetDefaultPrim("/refSphere")
UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.y)

modelRoot = UsdGeom.Xform.Define(stage, "/TexModel")
Usd.ModelAPI(modelRoot).SetKind(Kind.Tokens.component)


refSphere = stage.OverridePrim('/refSphere')
refSphere.GetReferences().AddReference('./Test.usda')

material = UsdShade.Material.Define(stage, '/TexModel/material')

pbrShader = UsdShade.Shader.Define(stage, '/TexModel/material/PBRShader')
pbrShader.CreateIdAttr("UsdPreviewSurface")
pbrShader.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(0.4)
pbrShader.CreateInput("metallic", Sdf.ValueTypeNames.Float).Set(0.0)

material.CreateSurfaceOutput().ConnectToSource(pbrShader, "surface")

stReader = UsdShade.Shader.Define(stage, '/TexModel/material/stReader')
stReader.CreateIdAttr('UsdPrimvarReader_float2')

diffuseTextureSampler = UsdShade.Shader.Define(stage,'/TexModel/material/diffuseTexture')
diffuseTextureSampler.CreateIdAttr('UsdUVTexture')
diffuseTextureSampler.CreateInput('file', Sdf.ValueTypeNames.Asset).Set("texture.jpg")
diffuseTextureSampler.CreateInput("st", Sdf.ValueTypeNames.Float2).ConnectToSource(stReader, 'result')
diffuseTextureSampler.CreateOutput('rgb', Sdf.ValueTypeNames.Float3)
pbrShader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).ConnectToSource(diffuseTextureSampler, 'rgb')

stInput = material.CreateInput('frame:stPrimvarName', Sdf.ValueTypeNames.Token)
stInput.Set('st')

stReader.CreateInput('varname',Sdf.ValueTypeNames.Token).ConnectToSource(stInput)

UsdShade.MaterialBindingAPI(refSphere).Bind(material)

stage.Save()