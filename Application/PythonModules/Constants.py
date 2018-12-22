# -------------------------------------------------------------------
# Constants
# -------------------------------------------------------------------
RIG_SKELETON = ["Chain", "Spine", "Tail"]
RIG_SKELETON_ITEM_ARRAY = ["Chain", 0, "Spine", 1, "Tail", 2]
RIG_CONTROLS_ITEM_ARRAY = ["SRT", 0, "IK", 1, "FK", 2, "IK/FK", 3, "Spring", 4, "External", 5, "FootRoll", 6]
RIG_UPVECTOR_ITEM_ARRAY = ["None", 0, "Collider Normal", 1, "External Object", 2]
RIG_SIDE = ["Left", "Right", "None"]
RIG_SIDE_ITEM_ARRAY = ["Left", 0, "Right", 1, "None", 2]
RIG_AXIS_ITEM_ARRAY = ["XY", 0, "YZ", 1, "XZ", 2]

# element type
ID_SKELETON = 0
ID_MUSCLE = 1
ID_CONTROL = 2
ID_GEOMETRY = 3

SUFFIX_SKELETON = "_Crv"
SUFFIX_MUSCLE = "_Crv"
SUFFIX_CONTROL = "_Ctrl"
SUFFIX_GEOMETRY = "_Geo"

PROP_BUILDER = "RigBuilder"
PROP_SKELETON = "RigElement"
PROP_MUSCLE = "MuscleElement"
PROP_CONTROL = "RigControl"

# skeleton type
ID_CHAIN = 0
ID_SPINE = 1
ID_TAIL = 2

# side enum
LEFT = 0
RIGHT = 1
MIDDLE = 2

# symmetry enum
XY_AXIS = 0
YZ_AXIS = 1
XZ_AXIS = 2

# control type enum
CTRL_SRT = 0
CTRL_IK = 1
CTRL_FK = 2
CTRL_IKFK = 3
CTRL_SPRING = 4
CTRL_EXTERNAL = 5
CTRL_FOOTROLL = 6

# colors
COLOR_SKELETON_R = 0.5
COLOR_SKELETON_G = 0.75
COLOR_SKELETON_B = 1.0

COLOR_MUSCLE_R = 1.0
COLOR_MUSCLE_G = 0.75
COLOR_MUSCLE_B = 0.5

# control icon type
RIG_CONTROL_ICON = ["cube",
					"pyramid",
					"square",
					"flower",
					"circle",
					"cylinder",
					"compas",
					"foil",
					"diamond",
					"leash",
					"cubewithpeak",
					"sphere",
					"arrow",
					"crossarrow",
					"bendedarrow",
					"bendedarrow2",
					"cross",
					"glasses",
					"lookat",
					"eyearrow",
					"anglesurvey",
					"eyeball",
					"rectanglecube",
					"man",
					"null"]
					
RIG_CONTROL_ICON_ITEM_ARRAY = ["cube", 0,
							   "pyramid", 1,
							   "square", 2,
							   "flower", 3,
							   "circle", 4,
							   "cylinder", 5,
							   "compas", 6,
							   "foil", 7,
							   "diamond", 8,
							   "leash", 9,
							   "cubewithpeak", 10,
							   "sphere", 11,
							   "arrow", 12,
							   "crossarrow", 13,
							   "bendedarrow", 14,
							   "bendedarrow2", 15,
							   "cross",16,
							   "glasses",17,
							   "lookat",18,
							   "eyearrow",19,
							   "anglesurvey",20,
							   "eyeball",21,
							   "rectanglecube",22,
							   "man",23,
							   "null",24]
