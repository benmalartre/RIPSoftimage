from pxr import Gf, Kind, Sdf, Usd, UsdGeom, UsdShade

stage = Usd.Stage.CreateNew("simpleShading.usda")
UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.y)

modelRoot = UsdGeom.Xform.Define(stage, "/TexModel")
Usd.ModelAPI(modelRoot).SetKind(Kind.Tokens.component)


billboard = UsdGeom.Mesh.Define(stage, "/TexModel/card")
billboard.CreatePointsAttr([(-400, -250, 0), (400, -250, 0), (400, 250, 0), (-400, 250, 0)])
billboard.CreateFaceVertexCountsAttr([4])
billboard.CreateFaceVertexIndicesAttr([0,1,2,3])
billboard.CreateExtentAttr([(-430, -145, 0), (430, 145, 0)])
texCoords = billboard.CreatePrimvar("st", 
                                    Sdf.ValueTypeNames.TexCoord2fArray, 
                                    UsdGeom.Tokens.varying)
texCoords.Set([(0, 0), (1, 0), (1,1), (0, 1)])



refSphere = stage.OverridePrim('/refSphere')
refSphere.GetReferences().AddReference('./Test.usda')


material = UsdShade.Material.Define(stage, '/TexModel/boardMat')

pbrShader = UsdShade.Shader.Define(stage, '/TexModel/boardMat/PBRShader')
pbrShader.CreateIdAttr("UsdPreviewSurface")
pbrShader.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(0.4)
pbrShader.CreateInput("metallic", Sdf.ValueTypeNames.Float).Set(0.0)

material.CreateSurfaceOutput().ConnectToSource(pbrShader, "surface")

stReader = UsdShade.Shader.Define(stage, '/TexModel/boardMat/stReader')
stReader.CreateIdAttr('UsdPrimvarReader_float2')

diffuseTextureSampler = UsdShade.Shader.Define(stage,'/TexModel/boardMat/diffuseTexture')
diffuseTextureSampler.CreateIdAttr('UsdUVTexture')
diffuseTextureSampler.CreateInput('file', Sdf.ValueTypeNames.Asset).Set("texture.jpg")
diffuseTextureSampler.CreateInput("st", Sdf.ValueTypeNames.Float2).ConnectToSource(stReader, 'result')
diffuseTextureSampler.CreateOutput('rgb', Sdf.ValueTypeNames.Float3)
pbrShader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).ConnectToSource(diffuseTextureSampler, 'rgb')

stInput = material.CreateInput('frame:stPrimvarName', Sdf.ValueTypeNames.Token)
stInput.Set('st')

stReader.CreateInput('varname',Sdf.ValueTypeNames.Token).ConnectToSource(stInput)

UsdShade.MaterialBindingAPI(billboard).Bind(material)
UsdShade.MaterialBindingAPI(refSphere).Bind(material)

stage.Save()