xsi = Application
root = xsi.ActiveSceneRoot

points = [-1.0, 0.0, -1.0,
			-1.0, 0.0, 0.0,
			-1.0, 0.0, 1.0,
			0.0, 0.0, -1.0,
			0.0, 0.0, 0.0,
			0.0, 0.0, 1.0,
			1.0, 0.0, -1.0,
			1.0, 0.0, 0.0,
			1.0, 0.0, 1.0]

faces = [4,0,1,4,3,
        4,1,2,5,4,
        4,3,4,7,6,
        4,4,5,8,7]

for i in xrange(12):
	offseted_points = []
	for index, value in enumerate(points):
		print index
		if index % 3 == 1:
			print "WTF"
			offseted_points.append(value+i)
		else:
			offseted_points.append(value)
			
	root.AddPolygonMesh( offseted_points, faces, "Cube"+str(i+1) )
	
xsi.SaveSceneAs( "E:\\Projects\\Softimage\\Procedural\\Scenes\\AudioTest\\myfuckingscene.scn")
	