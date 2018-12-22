#-----------------------------------------------------------------------------------------
# Write a Polymesh to Raafal Shape Data Format
#-----------------------------------------------------------------------------------------

xsi = Application
base = xsi.Selection(0).Name


filename = 'D:\\Projects\\Raafal\\tests\\Shapes\\'+base+'.txt'
name = base.upper()
label = base.lower() 

# open output file
target = open(filename,'w')

def DataSection_WritePositions(file,positions,nbp):
	for i in range(0,nbp):
		file.write("	Data.GLfloat "+str(positions[0][i])+","+str(positions[1][i])+","+str(positions[2][i])+"\n")

def DataSection_WriteIndices(file,indices,nbt):
	for i in range(0,nbt):
		file.write("	Data.GLuint "+str(indices[0][i])+","+str(indices[1][i])+","+str(indices[2][i])+"\n")
	
def DataSection_WriteEdges(file,geom,nbe):
	for i in range(0,nbe):
		vertices = geom.Edges[i].Vertices
		file.write("	Data.GLuint "+str(vertices[0].Index)+","+str(vertices[1].Index)+"\n")
		


geom = xsi.Selection(0).ActivePrimitive.Geometry
triangles = geom.Triangles
edges = geom.Edges
indices = triangles.IndexArray
positions = geom.Points.PositionArray

nbt = triangles.Count
nbv = geom.Points.Count
nbi = nbt*3
nbe = geom.Edges.Count


# Header
target.write("#"+name+"_NUM_TRIANGLES ="+str(nbt)+"\n")
target.write("#"+name+"_NUM_VERTICES ="+str(nbv)+"\n")
target.write("#"+name+"_NUM_INDICES ="+str(nbi)+"\n")
target.write("#"+name+"_NUM_EDGES ="+str(nbe)+"\n")
target.write("DataSection\n")
# Positions
target.write("	raa_shape_"+label+"_positions:\n")
DataSection_WritePositions(target,positions,nbv)
target.write("\n")
# Indices
target.write("	raa_shape_"+label+"_indices:\n")
DataSection_WriteIndices(target,indices,nbt)
target.write("\n")
# Edges
target.write("	raa_shape_"+label+"_edges:\n")
DataSection_WriteEdges(target,geom,nbe)
target.write("\n")
target.write("EndDataSection")

'''
xsi.LogMessage(indices)
for i in range(0,triangles.Count):
	xsi.LogMessage(str(indices[0][i])+","+str(indices[1][i])+","+str(indices[2][i]))
'''
target.close()