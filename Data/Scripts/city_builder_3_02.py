#!/usr/bin/env python
#----------------------------------------------------------------------------
# Name:         City Builder
# Purpose:      Create a random City
#
# Author:       Pierre Lalancette
#
# History:      Version 1.0: July 02nd 2007
#               -Create a random city
#               Version 2.0:
#               -No more WXPyton needed
#               -Install in plugin
#               Version 2.1: October 13th 2009
#               -Progress Bar (thanks to Marco Wehe)
#               -Discontinuity angle set to 30 (thanks to gent_k)
#               Version 2.21:
#               -Toggle the viewports to mute for speed
#               Version 2.22:
#               -A better Progress Bar
#               -Tighter PPG code
#               -fix problem if float front or float side are used
#               Version 2.23:
#               -Side walk are now higher than the street
#               -Most of the script is now object oriented
#               -Version removed from inside the script
#               -All variables converted to camelCases (ex: thisIsAVariable)
#               -All globals variables eliminated
#               Version 2.4: November 7th 2011
#               -Added random base quantity
#               -Added bevel quality
#               Version 2.5: November 9th 2011
#               -min < max
#               Vesion 2.56: January 18th 2012
#               -City sides have now minimums to contain at least 2 blocs
#               -new method RoadSideWalkCorrection that corrects sidewalks over streets
#               Version 2.6: February 7th 2012
#               -All scale represent 10cm a Unit
#               -Progress bar for shaders construction
#               -Shaders creation is more Object Oriented
#               Version 2.63: February 29th 2012
#               -New ground shader
#               -RoadSideWalkCorrection skipped if there is no street, only roads
#               Version 2.69: May 16th 2012
#               -Better Building shaders (Thanks for the ideas to 3nD of March)
#               Version 2.691: July 23 2012
#               -Parking shader
#               Version 2.693: July 24 2012
#               -Vegetation
#               Version 2.694: July 24 2012
#               -Merged Roads and SideWalk with shaders
#               Version 3.0: July 25 2012
#               -Better vegetation, the "about" tab updated to 3.0, ready for a release.
#               Version 3.02: July 26 2012
#               -Changed the material/shader assignment filters for the buildings to use progID, hoping to solve issue in Softimage 2013
#
# Copyright:    Use as you wish, at your own risk.
#              
#
# To Run:       -Copy this file into your pluging directory (Example: C:\Users\Lancelot\Autodesk\Softimage_2010\Application\Plugins)
#               -Restart Softimage
#               -In MODEL menu, GET -> PRIMITIVE -> POLYGON MESH -> CITY BUILDER
#
#----------------------------------------------------------------------------

import win32com.client
from win32com.client import constants
import random
from time import gmtime, strftime

#globals
null = None
false = 0
true = 1
           
def XSILoadPlugin( in_reg ):
    in_reg.Author = "Pierre 'Lancelot' Lalancette"
    in_reg.Name = "CityBuilderPlugin"
    in_reg.Major = 3
    in_reg.Minor = 0
    in_reg.RegisterProperty("CityBuilder")
    in_reg.RegisterMenu(constants.siMenuTbGetPrimitiveID,"CityBuilder_Menu",False,False)
    #RegistrationInsertionPoint - do not remove this line
    return True

def XSIUnloadPlugin( in_reg ):
    strPluginName = in_reg.Name
    name = strPluginName
    return True

def CityBuilder_Define( in_ctxt ):
    oCustomProperty = in_ctxt.Source
    oCustomProperty.AddParameter2("Front_city_size_km",constants.siFloat,1.0,0.05,1000000,0,10,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Side_city_size_km",constants.siFloat,1.0,0.05,1000000,0,10,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Do_you_want_a_Downtown",constants.siBool,True,null,null,null,null,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Do_you_want_a_basic_lighting",constants.siBool,True,null,null,null,null,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Percentage_of_empty_space",constants.siInt4,4,0,100,0,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Percentage_of_parking",constants.siInt4,20,0,100,0,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Do_you_want_vegetation",constants.siBool,True,null,null,null,null,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Number_of_maximum_vegetation",constants.siInt4,30,1,1000000,1,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Number_of_minimum_vegetation",constants.siInt4,5,1,1000000,1,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Number_of_maximum_stories",constants.siInt4,80,1,1000000,1,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Number_of_minimum_stories",constants.siInt4,20,1,1000000,1,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Maximum_front_size_m",constants.siFloat,50,0,1000000,0,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Minimum_front_size_m",constants.siFloat,20,0,1000000,0,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("maximum_side_size_m",constants.siFloat,75,0,1000000,0,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Minimum_side_size_m",constants.siFloat,30,0,1000000,0,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Do_you_want_bases",constants.siBool,True,null,null,null,null,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Maximum_bases_stories",constants.siInt4,6,1,1000000,1,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Minimum_bases_stories",constants.siInt4,2,1,1000000,1,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Percentage_of_base",constants.siInt4,44,0,100,0,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Percentage_of_full_sized_base",constants.siInt4,33,0,100,0,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Do_you_want_windows_reflections",constants.siBool,True,null,null,null,null,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Percentage_of_cylinders_vs_cubes",constants.siInt4,20,0,100,0,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Percentage_of_bevel_decorations",constants.siInt4,20,0,100,0,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Bevel_quality",constants.siInt4,0,0,100,0,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Number_of_building_shaders",constants.siInt4,10,1,1000000,1,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Number_of_roof_chaders",constants.siInt4,10,1,1000000,1,100,constants.siClassifUnknown,constants.siPersistable)
    oCustomProperty.AddParameter2("Saturation",constants.siFloat,-0.5,-1,1,-1,1,constants.siClassifUnknown,constants.siPersistable)
    return True

def CityBuilder_DefineLayout( in_ctxt ):
    oLayout = in_ctxt.Source
    oLayout.Clear()
    oLayout.AddTab("City")
    oLayout.AddGroup("City Size")
    oLayout.AddItem("Front_city_size_km")
    oLayout.AddItem("Side_city_size_km")
    oLayout.EndGroup()
    oLayout.AddGroup("City Decorations")
    oLayout.AddItem("Do_you_want_a_Downtown")
    oLayout.AddItem("Do_you_want_a_basic_lighting")
    oLayout.AddItem("Percentage_of_empty_space")
    oLayout.AddItem("Percentage_of_parking")
    oLayout.EndGroup()
    oLayout.AddGroup("Vegetation")
    oLayout.AddItem("Do_you_want_vegetation")
    oLayout.AddItem("Number_of_maximum_vegetation")
    oLayout.AddItem("Number_of_minimum_vegetation")
    oLayout.EndGroup()
    oLayout.AddTab("Buildings")
    oLayout.AddGroup("Buildings Size")
    oLayout.AddItem("Number_of_maximum_stories")
    oLayout.AddItem("Number_of_minimum_stories")
    oLayout.AddItem("Maximum_front_size_m")
    oLayout.AddItem("Minimum_front_size_m")
    oLayout.AddItem("maximum_side_size_m")
    oLayout.AddItem("Minimum_side_size_m")
    oLayout.EndGroup()
    oLayout.AddGroup("Bases")
    oLayout.AddItem("Do_you_want_bases")
    oLayout.AddItem("Maximum_bases_stories")
    oLayout.AddItem("Minimum_bases_stories")
    oLayout.AddItem("Percentage_of_base")
    oLayout.AddItem("Percentage_of_full_sized_base")
    oLayout.EndGroup()
    oLayout.AddGroup("Buildings Decorations")
    oLayout.AddItem("Do_you_want_windows_reflections")
    oLayout.AddItem("Percentage_of_cylinders_vs_cubes")
    oLayout.AddItem("Percentage_of_bevel_decorations")
    oLayout.AddItem("Bevel_quality")
    oLayout.EndGroup()
    oLayout.AddGroup("Shaders")
    oLayout.AddItem("Number_of_building_shaders")
    oLayout.AddItem("Number_of_roof_chaders")
    oLayout.AddItem("Saturation")
    oLayout.EndGroup()
    oLayout.AddTab("Generation")
    oLayout.AddButton("Generate")
    oLayout.AddButton("About")
    return True

"""

def CityBuilder_OnInit( ):

def CityBuilder_OnClosed( ):

"""

def CityBuilder_Front_city_size_km_OnChanged( ):
    if PPG.Front_city_size_km.Value * 1000 < PPG.Maximum_front_size_m.Value: # in Metres
        PPG.Front_city_size_km.Value = PPG.Maximum_front_size_m.Value / 1000
    return True

def CityBuilder_Side_city_size_km_OnChanged( ):
    if PPG.Side_city_size_km.Value * 1000 < (PPG.Maximum_side_size_m.Value + 32) * 2: # in Metres, 2 roads and 2 buildings
        PPG.Side_city_size_km.Value = ((PPG.Maximum_side_size_m.Value + 32) * 2) / 1000
    return True

"""

def CityBuilder_Do_you_want_a_Downtown_OnChanged( ):
    PPG.Do_you_want_a_Downtown.Value
    return True

def CityBuilder_Do_you_want_a_basic_lighting_OnChanged( ):
    PPG.Do_you_want_a_basic_lighting.Value
    return True

def CityBuilder_Percentage_of_empty_space_OnChanged( ):
    PPG.Percentage_of_empty_space.Value
    return True

def CityBuilder_Percentage_of_parking_OnChanged( ):
    PPG.Percentage_of_parking.Value
    return True

"""
def CityBuilder_Do_you_want_vegetation_OnChanged( ):
    if (PPG.Do_you_want_vegetation.Value) :
        PPG.Inspected(0).Number_of_maximum_vegetation.Enable(True)
        PPG.Inspected(0).Number_of_minimum_vegetation.Enable(True)
    else:
        PPG.Inspected(0).Number_of_maximum_vegetation.Enable(False)
        PPG.Inspected(0).Number_of_minimum_vegetation.Enable(False)
    return True

def CityBuilder_Number_of_maximum_vegetation_OnChanged( ):
    if PPG.Number_of_maximum_vegetation.Value <= PPG.Number_of_minimum_vegetation.Value:
        if PPG.Number_of_maximum_vegetation.Value < 2:
            PPG.Number_of_maximum_vegetation.Value = 2
        PPG.Number_of_minimum_vegetation.Value = PPG.Number_of_maximum_vegetation.Value - 1
    return True

def CityBuilder_Number_of_minimum_vegetation_OnChanged( ):
    if PPG.Number_of_minimum_vegetation.Value >= PPG.Number_of_maximum_vegetation.Value:
        PPG.Number_of_maximum_vegetation.Value = PPG.Number_of_minimum_vegetation.Value + 1
    return True

def CityBuilder_Number_of_maximum_stories_OnChanged( ):
    if PPG.Number_of_maximum_stories.Value <= PPG.Number_of_minimum_stories.Value:
        if PPG.Number_of_maximum_stories.Value < 2:
            PPG.Number_of_maximum_stories.Value = 2
        PPG.Number_of_minimum_stories.Value = PPG.Number_of_maximum_stories.Value - 1
    return True

def CityBuilder_Number_of_minimum_stories_OnChanged( ):
    if PPG.Number_of_minimum_stories.Value >= PPG.Number_of_maximum_stories.Value:
        PPG.Number_of_maximum_stories.Value = PPG.Number_of_minimum_stories.Value + 1
    return True

def CityBuilder_Maximum_front_size_m_OnChanged( ):
    if PPG.Maximum_front_size_m.Value <= PPG.Minimum_front_size_m.Value:
        if PPG.Maximum_front_size_m.Value < 2:
            PPG.Maximum_front_size_m.Value = 2
        PPG.Minimum_front_size_m.Value = PPG.Maximum_front_size_m.Value - 1
    if PPG.Front_city_size_km.Value * 1000 < PPG.Maximum_front_size_m.Value: # in Metres
        PPG.Front_city_size_km.Value = PPG.Maximum_front_size_m.Value / 1000
    return True

def CityBuilder_Minimum_front_size_m_OnChanged( ):
    if PPG.Minimum_front_size_m.Value >= PPG.Maximum_front_size_m.Value:
        PPG.Maximum_front_size_m.Value = PPG.Minimum_front_size_m.Value + 1
    return True

def CityBuilder_maximum_side_size_m_OnChanged( ):
    if PPG.maximum_side_size_m.Value <= PPG.Minimum_side_size_m.Value:
        if PPG.maximum_side_size_m.Value < 2:
            PPG.maximum_side_size_m.Value = 2
        PPG.Minimum_side_size_m.Value = PPG.maximum_side_size_m.Value - 1
    if PPG.Side_city_size_km.Value * 1000 < (PPG.Maximum_side_size_m.Value + 32) * 2: # in Metres, 2 roads and 2 buildings
        PPG.Side_city_size_km.Value = ((PPG.Maximum_side_size_m.Value + 32) * 2) / 1000
    return True

def CityBuilder_Minimum_side_size_m_OnChanged( ):
    if PPG.Minimum_side_size_m.Value >= PPG.maximum_side_size_m.Value:
        PPG.maximum_side_size_m.Value = PPG.Minimum_side_size_m.Value + 1
    return True

def CityBuilder_Do_you_want_bases_OnChanged( ):
    if (PPG.Do_you_want_bases.Value) :
        PPG.Inspected(0).Maximum_bases_stories.Enable(True)
        PPG.Inspected(0).Minimum_bases_stories.Enable(True)
        PPG.Inspected(0).Percentage_of_base.Enable(True)
        PPG.Inspected(0).Percentage_of_full_sized_base.Enable(True)
    else:
        PPG.Inspected(0).Maximum_bases_stories.Enable(False)
        PPG.Inspected(0).Minimum_bases_stories.Enable(False)
        PPG.Inspected(0).Percentage_of_base.Enable(False)
        PPG.Inspected(0).Percentage_of_full_sized_base.Enable(False)
    return True

def CityBuilder_Maximum_bases_stories_OnChanged( ):
    if PPG.Maximum_bases_stories.Value <= PPG.Minimum_bases_stories.Value:
        if PPG.Maximum_bases_stories.Value < 2:
            PPG.Maximum_bases_stories.Value = 2
        PPG.Minimum_bases_stories.Value = PPG.Maximum_bases_stories.Value - 1
    return True

def CityBuilder_Minimum_bases_stories_OnChanged( ):
    if PPG.Minimum_bases_stories.Value >= PPG.Maximum_bases_stories.Value:
        PPG.Maximum_bases_stories.Value = PPG.Minimum_bases_stories.Value + 1
    return True

"""

def CityBuilder_Percentage_of_base_OnChanged( ):
    PPG.Percentage_of_base.Value
    return True

def CityBuilder_Percentage_of_full_sized_base_OnChanged( ):
    PPG.Percentage_of_full_sized_base.Value
    return True

def CityBuilder_Do_you_want_windows_reflections_OnChanged( ):
    PPG.Do_you_want_windows_reflections.Value
    return True

def CityBuilder_Percentage_of_cylinders_vs_cubes_OnChanged( ):
    PPG.Percentage_of_cylinders_vs_cubes.Value
    return True

def CityBuilder_Percentage_of_bevel_decorations_OnChanged( ):
    PPG.Percentage_of_bevel_decorations.Value
    return True

def CityBuilder_Bevel_quality_OnChanged( ):
    PPG.Bevel_quality.Value
    return True

def CityBuilder_Number_of_building_shaders_OnChanged( ):
    PPG.Number_of_building_shaders.Value
    return True

def CityBuilder_Number_of_roof_chaders_OnChanged( ):
    PPG.Number_of_roof_chaders.Value
    return True
    
def CityBuilder_Saturaton_OnChanged( ):
    PPG.Saturation.Value
    return True

"""

def CityBuilder_About_OnClicked( ):
    XSIUIToolkit.Msgbox( "City Builder 3.01 by Pierre Lalancette\nwww.lalancelot.com\nlancelot@cgocable.ca\nlalancelot@hotmail.com")
    return True

def CityBuilder_Menu_Init( in_ctxt ):
    oMenu = in_ctxt.Source
    oMenu.AddCallbackItem("CityBuilder","OnCityBuilderMenuClicked")
    return True

def OnCityBuilderMenuClicked( in_ctxt ):
    oCustomProperty = XSIFactory.CreateObject("CityBuilder")
    Application.InspectObj(oCustomProperty)
    return True
    
def CityBuilder_Generate_OnClicked( ):
    userpreference = Application.GetValue("preferences.scripting.cmdlog", "") # what is the current status of command log
    Application.SetValue("preferences.scripting.cmdlog", 0, "") # deactive the command log
    Application.SetValue("preferences.Interaction.autoinspect", 0, "")
    # time started
    startTime = strftime("%a, %d %b %Y %H:%M:%S", gmtime())
    # Toggle all viewport to mute
    Application.ToggleMuteSolo(0,0)
    Application.ToggleMuteSolo(1,0)
    Application.ToggleMuteSolo(2,0)
    Application.ToggleMuteSolo(3,0)
    # Start fresh
    Application.DeselectAll()
    # create my class instance 'mycity'
    myCity = City(PPG.Front_city_size_km.Value, PPG.Side_city_size_km.Value, PPG.Do_you_want_a_Downtown.Value, PPG.Do_you_want_a_basic_lighting.Value,
            PPG.Percentage_of_empty_space.Value, PPG.Percentage_of_parking.Value, PPG.Do_you_want_vegetation.Value, PPG.Number_of_maximum_vegetation.Value,PPG.Number_of_minimum_vegetation.Value,
            PPG.Number_of_maximum_stories.Value,PPG.Number_of_minimum_stories.Value,PPG.Maximum_front_size_m.Value, PPG.Minimum_front_size_m.Value, PPG.maximum_side_size_m.Value, PPG.Minimum_side_size_m.Value,
            PPG.Do_you_want_bases.Value, PPG.Maximum_bases_stories.Value, PPG.Minimum_bases_stories.Value, PPG.Percentage_of_base.Value, PPG.Percentage_of_full_sized_base.Value,
            PPG.Do_you_want_windows_reflections.Value, PPG.Percentage_of_cylinders_vs_cubes.Value, PPG.Percentage_of_bevel_decorations.Value,
            PPG.Bevel_quality.Value, PPG.Number_of_building_shaders.Value, PPG.Number_of_roof_chaders.Value, PPG.Saturation.Value, None)
    # create a basic lighting if needed
    if myCity.light:
        myCity.LightCreate()
    # create the nulls and groups
    Application.DeselectAll()
    Application.CreateModel("", "road_null", "", "")
    roadNull = Application.selection(0)
    Application.DeselectAll()
    Application.CreateModel("", "base_null", "", "")
    baseNull = Application.selection(0)
    Application.DeselectAll()
    Application.CreateModel("", "building_null", "", "")
    buildingNull = Application.selection(0)
    Application.DeselectAll()
    Application.CreateModel("", "parking_null", "", "")
    parkingNull = Application.selection(0)
    Application.DeselectAll()
    Application.CreateModel("", "vegetation_null", "", "")
    vegetationNull = Application.selection(0)
    Application.DeselectAll()

    # ground and roads
    myCity.ground = myCity.Ground(myCity.cityFront, myCity.citySide)
    roadWide = myCity.Road(myCity.cityFront, myCity.citySide, myCity.frontMax, myCity.sideMax, roadNull)
    # prepare shaders dictionaries and all initialisation
    (shaderBuildDict, shaderRoofDict) = myCity.ShadersBR(myCity.shaderBuild, myCity.shaderRoof, myCity.reflection, myCity.frontMax, myCity.sideMax, myCity.storiesMax, myCity.saturation)
    numberLandSide = ((myCity.citySide * 10000) // ((myCity.sideMax * 20) + roadWide)) #unites
    numberLand = (myCity.cityFront * 1000) // myCity.frontMax # metres
    posY = 0.5
    posZ = -roadWide
    orient = 180 
    streetCount = 0
    streetWide = 0
    baseFront = random.uniform(myCity.frontMin, myCity.frontMax)
    baseSide = random.uniform(myCity.sideMin, myCity.sideMax)
    posXAdjust = 0 # when there is a street
    posXAdjustCount = 0
    # variable needed for the Downtown
    differenceStories = (myCity.storiesMax - myCity.storiesMin) * 2
    storyStepFront = (myCity.cityFront * 10000) // differenceStories
    storyStepSide = (myCity.citySide * 10000) // differenceStories
    posXRef = -(myCity.cityFront * 5000)
    posZRef = (myCity.citySide * 5000)
    # here we go!
    # added Progress bar ...
    oProgressBar = XSIUIToolkit.ProgressBar
    oProgressBar.Value = 0
    if numberLand <> 0:
        oProgressBar.Maximum = numberLand * numberLandSide * 2
    else:
        numberLand = 1
        oProgressBar.Step = 1
    oProgressBar.CancelEnabled = True 
    oProgressBar.Caption = "Creating City."
    oProgressBar.Visible = True
    numberLandSideCount = 0
    front = 0
    side = 0
    doVegetation = True
    fullBaseVeg = False
    geo = ""
    while numberLandSideCount < numberLandSide:
        iterations = 0
        posX = 0
        posXAdjust = 0
        while posX < myCity.cityFront * 10000:
            doVegetation = myCity.vegetation
            fullBaseVeg = False
            geo = myCity.GeoBuild(myCity.cylCube, myCity.empty, myCity.parking) # Percentage of cubes vs cylinders, empty spaces and parkings
            oProgressBar.Increment()
            oProgressBar.Caption = "Creating City.  Now at position X : " + str(posX) + " and position Z : " + str(posZ) + " with Geo : " + geo
            posX = (iterations*myCity.frontMax * 10) + (myCity.frontMax * 5) - posXAdjust
            if orient == 0:
                posZActive = posZ - (myCity.sideMax * 20) # 2 espaces (*20)
                orient = 180                    
            else:
                posZActive = posZ
                orient = 0
                if streetCount > random.randint(1,4):
                    posX = posX - (myCity.frontMax * 5) #put the position to the corner of frontmax
                    if posX < myCity.cityFront*10000: # don't do this object if it's out of the city
                        streetWide = myCity.Street(posX, posY, posZ, myCity.cityFront, myCity.citySide, myCity.frontMax, myCity.sideMax, roadNull)
                    streetCount = 0
                    posXAdjust += ((myCity.frontMax * 10) - streetWide) #prepare adjustment to place posx back to the corner of the street
                    posXAdjustCount += posXAdjust
                    if posXAdjustCount > myCity.frontMax * 10:
                        numberLand += 1
                        posXAdjustCount -= myCity.frontMax * 10
                    orient = 180
                    geo = 'Empty'
                    doVegetation = False
            if geo == 'Parking':
                if posX < myCity.cityFront * 10000: # don't do this object if it's out of the city
                    myCity.Parking(posX, 0, posZActive, orient, myCity.frontMax, myCity.sideMax, myCity.cityFront, myCity.citySide, parkingNull, myCity.ground)
            if geo == 'Cube' or geo == 'Cylinder':
                if myCity.base and random.randint(0,100) < myCity.percentBase: # a Percentage of base 
                    if random.randint(0,100) < myCity.fullBase: # a Percentage of full sized base
                        baseFront = myCity.frontMax
                        baseSide = myCity.sideMax
                        fullBaseVeg = True
                    else:
                        baseFront = random.uniform(myCity.frontMin, myCity.frontMax)
                        baseSide = random.uniform(myCity.sideMin, myCity.sideMax)
                        fullBaseVeg = False
                    baseStories = random.randint(myCity.baseMin, myCity.baseMax)
                    if posX < myCity.cityFront*10000: # don't do this object if it's out of the city
                        myCity.Building(shaderBuildDict, shaderRoofDict, 'Cube', posX, 0, posZActive, orient, baseStories, myCity.storiesMax, baseFront, baseSide, True, myCity.bevel, myCity.bevelQuality, baseNull, buildingNull)
                    posY = baseStories * 40 # 4m a story
                else:
                    baseFront = myCity.frontMax
                    baseSide = myCity.sideMax
                if myCity.storiesMin >= myCity.storiesMax:
                    myCity.storiesMax = myCity.storiesMin + 1
                buildingStories = random.randint(myCity.storiesMin, myCity.storiesMax)
                front = random.uniform(myCity.frontMin, baseFront)
                side = random.uniform(myCity.sideMin, baseSide)
                if myCity.downTown:
                    actualPosX = posXRef + posX
                    if actualPosX < 0:
                        actualPosX = -actualPosX
                    actualPosZ = posZRef + posZActive
                    if actualPosZ < 0:
                        actualPosZ = -actualPosZ
                    buildingStoriesX = int((actualPosX // storyStepFront) * 1.2)
                    if buildingStoriesX > myCity.storiesMax:
                        buildingStoriesX = myCity.storiesMax
                    buildingStoriesZ = int((actualPosZ // storyStepSide) * 1.2)
                    if buildingStoriesZ > myCity.storiesMax:
                        buildingStoriesZ = myCity.storiesMax
                    if buildingStoriesX > buildingStoriesZ: #take the highest
                        buildingStories = random.randint(myCity.storiesMin, myCity.storiesMin + (myCity.storiesMax - buildingStoriesX))
                    else:
                        buildingStories = random.randint(myCity.storiesMin, myCity.storiesMin + (myCity.storiesMax - buildingStoriesZ))
                    if buildingStories <= 0:
                            buildingStories = 1
                if posX < myCity.cityFront * 10000: # don't do this object if it's out of the city
                    myCity.Building(shaderBuildDict, shaderRoofDict, geo, posX, posY, posZActive, orient, buildingStories, myCity.storiesMax, front, side, False, myCity.bevel, myCity.bevelQuality, baseNull, buildingNull)
                posY = 0
            if doVegetation and not fullBaseVeg:
                myCity.Vegetation(posX, posY, posZActive, orient, myCity.frontMax, myCity.sideMax, front, side, baseFront, baseSide, geo, myCity.vegetationMax, myCity.vegetationMin, vegetationNull)
            if orient == 180:
                iterations += 1
                streetCount += 1
        orient = 180
        posZ = posZ - (roadWide + (myCity.sideMax * 20))
        if oProgressBar.CancelPressed:
            break
        numberLandSideCount += 1
    #Correct sideWalk between streets
    oProgressBar.Caption = "Merging Roads (please wait...)"
    if numberLand > 1:
        myCity.RoadSideWalkCorrection( roadNull, myCity.citySide)
    Application.DeselectAll()
    #Merge Roads
    myCity.MergeRoads(myCity.cityFront, myCity.citySide, roadNull, myCity.saturation)
    Application.DeselectAll()
    #Merge Vegetation
    oProgressBar.Caption = "Merging Vegetation (Can take a while, depending on how much you exagerated...)"
    if myCity.vegetation:
        myCity.MergeVegetation(front, side, myCity.saturation, vegetationNull)
    Application.DeselectAll()
    #Bring the viewport back
    oProgressBar.Visible = False
    Application.ToggleMuteSolo(0,1)
    Application.ToggleMuteSolo(1,0)
    Application.ToggleMuteSolo(2,0)
    Application.ToggleMuteSolo(3,0)
    # Time when finished
    endTime = strftime("%a, %d %b %Y %H:%M:%S", gmtime())
    Application.Logmessage("Started at: " + startTime)
    Application.Logmessage("ended at: " + endTime)
    # Bring back preferences
    Application.SetValue("preferences.scripting.cmdlog", userpreference, "")
    Application.SetValue("preferences.Interaction.autoinspect", 1, "")
    return True
    
class City:
    '''
    Create a City object with all variable needed to carry the creation of the full CG city
    '''
    def __init__(self, cityFront = 1.0, citySide = 1.0, downTown = True , light = True, empty = 4, parking = 20, 
            vegetation = True, vegetationMax = 30, vegetationMin = 5, storiesMax = 80, storiesMin = 2, frontMax = 50, frontMin = 20, sideMax = 75, sideMin = 30,
            base = True, baseMax = 6, baseMin = 2, percentBase = 44, fullBase = 33, reflection = True,
            cylCube = 20, bevel = 20, bevelQuality = 0, shaderBuild = 10, shaderRoof = 10, saturation = -0.5, ground = None):
        self.cityFront = cityFront
        self.citySide = citySide
        self.downTown = downTown
        self.light = light
        self.empty = empty
        self.parking = parking
        self.vegetation = vegetation
        self.vegetationMax = vegetationMax
        self.vegetationMin = vegetationMin
        self.storiesMax = storiesMax
        self.storiesMin = storiesMin
        self.frontMax = frontMax
        self.frontMin = frontMin
        self.sideMax = sideMax
        self.sideMin = sideMin
        self.base = base
        self.baseMax = baseMax
        self.baseMin = baseMin
        self.percentBase = percentBase
        self.fullBase = fullBase
        self.reflection = reflection
        self.cylCube = cylCube
        self.bevel = bevel
        self.bevelQuality = bevelQuality
        self.shaderBuild = shaderBuild
        self.shaderRoof = shaderRoof
        self.saturation = saturation
        self.ground = ground

    def LightCreate(self):
        # remove the ambiance light of the scene
        Application.ActiveSceneRoot.localProperties.find("AmbientLighting").ambience.red.Value = 0
        Application.ActiveSceneRoot.localProperties.find("AmbientLighting").ambience.green.Value = 0
        Application.ActiveSceneRoot.localProperties.find("AmbientLighting").ambience.blue.Value = 0
        # all lights under a same null for convinience
        Application.CreateModel("", "light_null", "", "")
        lightNull = Application.selection(0)
        # key light, casting shadow, little yellow
        key = Application.GetPrimLight("Infinite.Preset", "Key", lightNull.Name, "", "", "")
        key.Kinematics.Global.rotx.Value = -60
        key.Kinematics.Global.roty.Value = -40
        key.Kinematics.Global.rotz.Value = 0
        key.Shaders("soft_light").intensity.Value = 1.5
        key.Shaders("soft_light").shadow.Value = 1
        key.Shaders("soft_light").factor.Value = 0.074
        key.Shaders("soft_light").color.blue.Value = 0.933
        # grey fill light
        fill = Application.GetPrimLight("Infinite.Preset", "Fill", lightNull.Name, "", "", "")
        fill.Kinematics.Global.rotx.Value = 30
        fill.Kinematics.Global.roty.Value = -100
        fill.Kinematics.Global.rotz.Value = 0
        fill.Shaders("soft_light").intensity.Value = 0.5
        fill.Shaders("soft_light").color.red.Value = 0.8
        fill.Shaders("soft_light").color.green.Value = 0.8
        fill.Shaders("soft_light").color.blue.Value = 0.8
        # rim, small and little blue
        rim = Application.GetPrimLight("Infinite.Preset", "Rim", lightNull.Name, "", "", "")
        rim.Kinematics.Global.rotx.Value = 30
        rim.Kinematics.Global.roty.Value = -200
        rim.Kinematics.Global.rotz.Value = 0
        rim.Shaders("soft_light").intensity.Value = 0.4
        rim.Shaders("soft_light").color.red.Value = 0.933
        Application.DeselectAll()
        return True

    def GeoBuild(self, percentage = 80, empty = 4, parking = 20):
        if random.randint(0,100) < empty: # random Percentage of empty place
            return "Empty"
        if random.randint(0,100) < parking:  # random Percentage of parking
            return "Parking"
        if(random.randint(0,100) < percentage):  # random Percentage of cylinders vs cubes
            return "Cylinder"
        else:
            return "Cube"

    def Ground(self,cityFront=2,citySide=2):
        activeObj = Application.ActiveSceneRoot.addGeometry("grid", "MeshSurface","ground")
        activeObj.Properties.find("geomapprox").gapproxmoan.Value = 30 # geo approximation angle
        activeObj.Parameters("subdivu").Value = 2 #reduce geo
        activeObj.Parameters("subdivv").Value = 2

        cityFront = cityFront * 10000 # km
        citySide = citySide * 10000 

        # Scale to 1 unit * city size
        activeObj.Kinematics.Global.sclx.Value = 0.125
        activeObj.Kinematics.Global.scly.Value = 0.125
        activeObj.Kinematics.Global.sclz.Value = 0.125
        Application.ResetTransform(activeObj.Name, "siCtr", "siScl", "siXYZ")
        activeObj.Kinematics.Global.sclx.Value = cityFront
        activeObj.Kinematics.Global.sclz.Value = citySide
        activeObj.Kinematics.Global.posx.Value = 0.5 * cityFront
        activeObj.Kinematics.Global.posy.Value = 0
        activeObj.Kinematics.Global.posz.Value = -0.5 * citySide

        # Let's make a terrain shader 
        oMaterial = activeObj.AddMaterial( "Lambert", False, "Ground_Shader" );
        oLambert = activeObj.Material.Shaders(0)
        oTerrain = Application.CreateShaderFromProgID("Softimage.txt2d-terrain.1.0", oMaterial.FullName, "Terrain")
        oFractal = Application.CreateShaderFromProgID("Softimage.txt3d-fractal_v3.1.0", oMaterial.FullName, "Fractal")
        oMarble = Application.CreateShaderFromProgID("Softimage.txt3d-marble.1.0", oMaterial.FullName, "Marble")
        oFractal1 = Application.CreateShaderFromProgID("Softimage.txt3d-fractal_v3.1.0", oMaterial.FullName, "Fractal")
        oMix8 = Application.CreateShaderFromProgID("Softimage.sib_color_8mix.1.0", oMaterial.FullName, "Mix_8_Colors")
        # connections
        Application.SIConnectShaderToCnxPoint(oMix8.FullName + ".out", oLambert.FullName + ".ambient", False)
        Application.SIConnectShaderToCnxPoint(oMix8.FullName + ".out", oLambert.FullName + ".diffuse", False)
        
        Application.SIConnectShaderToCnxPoint(oTerrain.FullName + ".out", oMix8.FullName + ".base_color", False)
        Application.SIConnectShaderToCnxPoint(oFractal.FullName + ".out", oMix8.FullName + ".color1", False)
        Application.SIConnectShaderToCnxPoint(oMarble.FullName + ".out", oMix8.FullName + ".color2", False)
        Application.SIConnectShaderToCnxPoint(oFractal1.FullName + ".out", oMix8.FullName + ".color3", False)
        # Texture projecton
        dummyName, projectionGround = Application.CreateProjection(activeObj.FullName, "siTxtPlanarXZ", "siTxtDefaultSpherical", "Texture_Support", "Texture_Projection", "", "", "")
        dummyName, projectionGround1 = Application.CreateProjection(activeObj.FullName, "siTxtPlanarXZ", "siTxtDefaultSpherical", "Texture_Support1", "Texture_Projection1", "", "", "")
        dummyName, projectionGround2 = Application.CreateProjection(activeObj.FullName, "siTxtPlanarXZ", "siTxtDefaultSpherical", "Texture_Support2", "Texture_Projection2", "", "", "")

        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("Texture_Projection").NestedObjects("Texture_Projection_Def").Parameters("projsclu").Value = 4 * (cityFront / 10000) 
        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("Texture_Projection").NestedObjects("Texture_Projection_Def").Parameters("projsclv").Value = 4 * (citySide / 10000)
        Application.SetInstanceDataValue(activeObj.FullName, oTerrain.FullName + ".tspace_id", projectionGround)
        
        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("Texture_Projection1").NestedObjects("Texture_Projection1_Def").Parameters("projsclu").Value = 20 * (cityFront / 10000) 
        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("Texture_Projection1").NestedObjects("Texture_Projection1_Def").Parameters("projsclv").Value = 20 * (citySide / 10000)
        Application.SetInstanceDataValue(activeObj.FullName, oFractal.FullName + ".tspace_id", projectionGround1)

        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("Texture_Projection2").NestedObjects("Texture_Projection2_Def").Parameters("projsclu").Value = (cityFront / 10000) 
        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("Texture_Projection2").NestedObjects("Texture_Projection2_Def").Parameters("projsclv").Value = (citySide / 10000)
        Application.SetInstanceDataValue(activeObj.FullName, oMarble.FullName + ".tspace_id", projectionGround2)
        Application.SetInstanceDataValue(activeObj.FullName, oFractal1.FullName + ".tspace_id", projectionGround2)
        # Terrain
        oTerrain.Parameters("snow_color").Parameters("red").Value = random.uniform(0.1,0.2)
        oTerrain.Parameters("snow_color").Parameters("green").Value = random.uniform(0.35,0.45)
        oTerrain.Parameters("snow_color").Parameters("blue").Value = random.uniform(0.1,0.2)
        oTerrain.Parameters("rock_color").Parameters("red").Value = random.uniform(0.35,0.45)
        oTerrain.Parameters("rock_color").Parameters("green").Value = random.uniform(0.25,0.35)
        oTerrain.Parameters("rock_color").Parameters("blue").Value = random.uniform(0.15,0.25)
        oTerrain.Parameters("grass_color").Parameters("red").Value = random.uniform(0.3,0.4)
        oTerrain.Parameters("grass_color").Parameters("green").Value = random.uniform(0.4,0.5)
        oTerrain.Parameters("grass_color").Parameters("blue").Value = random.uniform(0.3,0.4)
        oTerrain.Parameters("snow_rock_boundary").Value = random.uniform(0.7,0.8)
        oTerrain.Parameters("rock_grass_boundary").Value = random.uniform(0.55,0.65)
        oTerrain.Parameters("boundary_blend").Value = random.uniform(0.45,0.55)
        oTerrain.Parameters("boundary_ruggedness").Value = random.uniform(0.45,0.55)
        oTerrain.Parameters("ratio").Value = random.uniform(0.9,1)
        oTerrain.Parameters("iterations").Value = random.uniform(1.95,2.05)
        oTerrain.Parameters("amplitude").Value = random.uniform(0.9,1)
        oTerrain.Parameters("ruggedness").Value = random.uniform(0.25,0.35)
        oTerrain.Parameters("u_frequency").Value = random.uniform(1.95,2.05)
        oTerrain.Parameters("v_frequency").Value = random.uniform(1.95,2.05)
        # Fractal
        oFractal.Parameters("color1").Parameters("red").Value = random.uniform(0.8,0.9)
        oFractal.Parameters("color1").Parameters("green").Value = random.uniform(0.5,0.6)
        oFractal.Parameters("color1").Parameters("blue").Value = random.uniform(0.3,0.4)
        oFractal.Parameters("color2").Parameters("red").Value = 0
        oFractal.Parameters("color2").Parameters("green").Value = 0
        oFractal.Parameters("color2").Parameters("blue").Value = 0
        oFractal.Parameters("noise_type").Value = 0
        oFractal.Parameters("iterations").Value = 2
        oFractal.Parameters("diffusion").Value = random.uniform(0.9,1)
        oFractal.Parameters("upper_bound").Value = random.uniform(0.9,1)
        oFractal.Parameters("freq_mul").Value = random.uniform(1.95,2.05)
        oFractal.Parameters("level_min").Value = random.uniform(0.45,0.55)
        oFractal.Parameters("level_decay").Value = random.uniform(0.65,0.75)
        oFractal.Parameters("threshold").Value = random.uniform(0.3,0.6)
        oFractal.Parameters("time").Value = 0
        # Marble
        oMarble.Parameters("vein_col1").Parameters("red").Value = random.uniform(0.3,0.4)
        oMarble.Parameters("vein_col1").Parameters("green").Value = random.uniform(0.4,0.5)
        oMarble.Parameters("vein_col1").Parameters("blue").Value = random.uniform(0.05,0.15)
        oMarble.Parameters("vein_col2").Parameters("red").Value = random.uniform(0.2,0.3)
        oMarble.Parameters("vein_col2").Parameters("green").Value = random.uniform(0.45,0.55)
        oMarble.Parameters("vein_col2").Parameters("blue").Value = random.uniform(0.65,0.75)
        oMarble.Parameters("filler_col").Parameters("red").Value = random.uniform(0.4,0.5)
        oMarble.Parameters("filler_col").Parameters("green").Value = random.uniform(0.75,0.85)
        oMarble.Parameters("filler_col").Parameters("blue").Value = random.uniform(0.75,0.85)
        oMarble.Parameters("vein_width").Value = random.uniform(0.15,0.18)
        oMarble.Parameters("diffusion").Value = random.uniform(0.25,0.35)
        oMarble.Parameters("spot_color").Parameters("red").Value = random.uniform(0.9,1)
        oMarble.Parameters("spot_color").Parameters("green").Value = random.uniform(0.9,1)
        oMarble.Parameters("spot_color").Parameters("blue").Value = random.uniform(0.9,1)
        oMarble.Parameters("spot_density").Value = random.uniform(4,6)
        oMarble.Parameters("spot_bias").Value = random.uniform(0.005,0.015)
        oMarble.Parameters("spot_scale").Value = random.uniform(0.7,0.8)
        oMarble.Parameters("amplitude").Value = random.uniform(0,5) * (cityFront / 10000)
        oMarble.Parameters("ratio").Value = random.uniform(0,1)
        oMarble.Parameters("complexity").Value = random.uniform(1,20)
        oMarble.Parameters("frequencies").Parameters("x").Value = random.uniform(0.3,0.4)
        oMarble.Parameters("frequencies").Parameters("y").Value = random.uniform(0.2,0.3)
        oMarble.Parameters("frequencies").Parameters("z").Value = random.uniform(0.35,0.45)
        # Fractal1
        oFractal1.Parameters("color1").Parameters("red").Value = random.uniform(0.8,0.9)
        oFractal1.Parameters("color1").Parameters("green").Value = random.uniform(0.5,0.6)
        oFractal1.Parameters("color1").Parameters("blue").Value = random.uniform(0.3,0.4)
        oFractal1.Parameters("color2").Parameters("red").Value = 0
        oFractal1.Parameters("color2").Parameters("green").Value = 0
        oFractal1.Parameters("color2").Parameters("blue").Value = 0
        oFractal1.Parameters("noise_type").Value = 0
        oFractal1.Parameters("iterations").Value = 6
        oFractal1.Parameters("diffusion").Value = 0
        oFractal1.Parameters("upper_bound").Value = 1
        oFractal1.Parameters("freq_mul").Value = random.uniform(1.75,2)
        oFractal1.Parameters("level_min").Value = random.uniform(0.05,0.35)
        oFractal1.Parameters("level_decay").Value = 1
        oFractal1.Parameters("threshold").Value = random.uniform(0.1,0.2)
        oFractal1.Parameters("time").Value = 0
        # Mix
        oMix8.Parameters("weight1").Parameters("red").Value = 0.65
        oMix8.Parameters("weight1").Parameters("green").Value = 0.65
        oMix8.Parameters("weight1").Parameters("blue").Value = 0.65
        oMix8.Parameters("inuse1").Value = True
        oMix8.Parameters("mode1").Value = 1
        oMix8.Parameters("weight2").Parameters("red").Value = 0.65
        oMix8.Parameters("weight2").Parameters("green").Value = 0.65
        oMix8.Parameters("weight2").Parameters("blue").Value = 0.65
        oMix8.Parameters("inuse2").Value = True
        oMix8.Parameters("mode2").Value = 10
        oMix8.Parameters("weight3").Parameters("red").Value = 0.4
        oMix8.Parameters("weight3").Parameters("green").Value = 0.4
        oMix8.Parameters("weight3").Parameters("blue").Value = 0.4
        oMix8.Parameters("inuse3").Value = True
        oMix8.Parameters("mode3").Value = 1
        
        Application.FreezeObj(activeObj)
        Application.DeselectAll()
        return activeObj

    def Street(self, posX, posY, posZ, cityFront = 2, citySide = 2, frontMax = 50, sideMax = 50, roadNull = ''):
        lane = random.randint(2,6) * 40 # 2 to 6 lanes of 4m
        sidewalk = 40 # 4m
        streetwide = lane + (sidewalk * 2)
        cityFront = cityFront * 10000 # metres
        citySide = citySide * 10000
        frontMax = frontMax * 10
        sideMax = sideMax * 10
        street = sideMax * 2

        activeObjRef = Application.ActiveSceneRoot.addGeometry("cube", "MeshSurface","sideWalkPart")
        activeObjRef.Properties.find("geomapprox").gapproxmoan.Value = 30 # geo approximation angle
        Application.ApplyTopoOp("DeleteComponent", activeObjRef.Name+".poly[1]", "siUnspecified", "siPersistentOperation", "")
        # scale to 1 unit
        activeObjRef.Kinematics.Global.sclx.Value = 0.125
        activeObjRef.Kinematics.Global.scly.Value = 0.125
        activeObjRef.Kinematics.Global.sclz.Value = 0.125
        Application.ResetTransform(activeObjRef.Name, "siCtr", "siScl", "siXYZ")
        Application.FreezeObj(activeObjRef) # freeze

        # use a duplicate of the reference object to make: sidewalk
        activeObjColl = Application.Duplicate(activeObjRef.Name, 1, 0, 0, 0, 0, 0, 1, 0, 1, sidewalk, 2, street, "", "", "", "", "", "","", 0)
        Application.MoveCtr2Vertices(activeObjColl(0).Name+".pnt[4]", 0) # put center to bottom front left
        Application.FreezeObj(activeObjColl(0)) # freeze
        activeObjColl(0).Kinematics.Global.posx.Value = posX
        activeObjColl(0).Kinematics.Global.posz.Value = posZ
        # all road, sidewalk and street child of the same null
        roadNull.AddChild(activeObjColl(0))

        #sidewalks 2
        Application.SelectObj(activeObjRef, "", 1) # starting cube 1x1x1
        Application.MoveCtr2Vertices(activeObjRef.Name+".pnt[4]", 0) # put center to bottom front left
        activeObjRef.Kinematics.Global.sclx.Value = sidewalk
        activeObjRef.Kinematics.Global.scly.Value = 2
        activeObjRef.Kinematics.Global.sclz.Value = street
        activeObjRef.Kinematics.Global.posx.Value = posX+lane+sidewalk
        activeObjRef.Kinematics.Global.posy.Value = posY
        activeObjRef.Kinematics.Global.posz.Value = posZ
        Application.FreezeObj(activeObjRef) # freeze
        # all road, sidewalk and street child of the same null
        roadNull.AddChild(activeObjRef)

        # street
        activeObj = Application.ActiveSceneRoot.addGeometry("grid", "MeshSurface","roadPart")
        activeObj.Properties.find("geomapprox").gapproxmoan.Value = 30 # geo approximation angle
        activeObj.Parameters("subdivu").Value = 2 #reduce geo
        activeObj.Parameters("subdivv").Value = 2
        activeObj.Kinematics.Global.sclx.Value = 0.125
        activeObj.Kinematics.Global.scly.Value = 0.125
        activeObj.Kinematics.Global.sclz.Value = 0.125 
        Application.MoveCtr2Vertices(activeObj.Name+".pnt[2]", "") # put center to bottom front left
        Application.ResetTransform(activeObj.Name, "siCtr", "siScl", "siXYZ")
        activeObj.Kinematics.Global.sclx.Value = lane
        activeObj.Kinematics.Global.sclz.Value = (street + (sidewalk * 2))
        activeObj.Kinematics.Global.posx.Value = posX + sidewalk
        activeObj.Kinematics.Global.posy.Value = 0.5
        activeObj.Kinematics.Global.posz.Value = posZ + sidewalk
        Application.FreezeObj(activeObj) # freeze
        # all road, sidewalk and street child of the same null
        roadNull.AddChild(activeObj)

        Application.DeselectAll()
        return streetwide

    def Road(self, cityfront = 2, cityside = 2, frontmax = 50, sidemax = 50, roadNull = ''):
        lane = 6 * 40 # 6 lanes of 4m
        sidewalk = 40 # 4m
        road = lane + (sidewalk*2)
        cityfront = cityfront*10000 # 10 centimeters to kms
        cityside = cityside*10000
        frontmax=frontmax*10
        sidemax = sidemax*10
        numsideroad = (cityside // (road+(sidemax*2))) + 1
        numSideRoadCount = 0
        while numSideRoadCount < numsideroad:
            posZ = -(numSideRoadCount * (road+(sidemax*2))) #metres

            activeObjRef = Application.ActiveSceneRoot.addGeometry("cube", "MeshSurface","sideWalkPart")
            activeObjRef.Properties.find("geomapprox").gapproxmoan.Value = 30 # geo approximation angle
            Application.ApplyTopoOp("DeleteComponent", activeObjRef.Name+".poly[1]", "siUnspecified", "siPersistentOperation", "")
            # scale to 1 unit
            activeObjRef.Kinematics.Global.sclx.Value = 0.125
            activeObjRef.Kinematics.Global.scly.Value = 0.125
            activeObjRef.Kinematics.Global.sclz.Value = 0.125
            Application.ResetTransform(activeObjRef.Name, "siCtr", "siScl", "siXYZ")
            Application.FreezeObj(activeObjRef) # freeze

            # use a duplicate of the reference object to make: sidewalk
            activeObjColl = Application.Duplicate(activeObjRef.Name, 1, 0, 0, 0, 0, 0, 1, 0, 1, cityfront, 2, sidewalk, "", "", "", "", "", "","", 0)
            Application.MoveCtr2Vertices(activeObjColl(0).Name+".pnt[4]", 0) # put center to bottom front left
            Application.FreezeObj(activeObjColl(0)) # freeze
            activeObjColl(0).Kinematics.Global.posx.Value = 0
            activeObjColl(0).Kinematics.Global.posy.Value = 0.5
            activeObjColl(0).Kinematics.Global.posz.Value = posZ
            # all road, sidewalk and street child of the same null
            roadNull.AddChild(activeObjColl(0))

            #sidewalks 2
            Application.SelectObj(activeObjRef, "", 1) # starting cube 1x1x1
            Application.MoveCtr2Vertices(activeObjRef.Name+".pnt[4]", 0) # put center to bottom front left
            activeObjRef.Kinematics.Global.sclx.Value = cityfront
            activeObjRef.Kinematics.Global.scly.Value = 2
            activeObjRef.Kinematics.Global.sclz.Value = sidewalk
            activeObjRef.Kinematics.Global.posx.Value = 0
            activeObjRef.Kinematics.Global.posy.Value = 0.5
            activeObjRef.Kinematics.Global.posz.Value = posZ - (road - sidewalk)
            Application.FreezeObj(activeObjRef) # freeze
            # all road, sidewalk and street child of the same null
            roadNull.AddChild(activeObjRef)
            numSideRoadCount += 1
        
            # make roads
            activeObj = Application.ActiveSceneRoot.addGeometry("grid", "MeshSurface","roadPart")
            activeObj.Properties.find("geomapprox").gapproxmoan.Value = 30 # geo approximation angle
            activeObj.Parameters("subdivu").Value = 2 # reduce geo
            activeObj.Parameters("subdivv").Value = 2
            activeObj.Kinematics.Global.sclx.Value = 0.125
            activeObj.Kinematics.Global.scly.Value = 0.125
            activeObj.Kinematics.Global.sclz.Value = 0.125
            Application.MoveCtr2Vertices(activeObj.Name+".pnt[2]", "") # put center to bottom front left
            Application.ResetTransform(activeObj.Name, "siCtr", "siScl", "siXYZ")
            activeObj.Kinematics.Global.sclx.Value = activeObj.Kinematics.Global.sclx.Value * cityfront
            activeObj.Kinematics.Global.sclz.Value = activeObj.Kinematics.Global.sclz.Value * lane
            activeObj.Kinematics.Global.posx.Value = 0
            activeObj.Kinematics.Global.posy.Value = 0.5
            activeObj.Kinematics.Global.posz.Value = posZ - sidewalk
            Application.FreezeObj(activeObj) # freeze

            # all road, sidewalk and street child of the same null
            roadNull.AddChild(activeObj)
        
        Application.DeselectAll()
        return road

    def RoadSideWalkCorrection(self,roadNull, citySide = 2 ):
        testStreet = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart4", False)
        if str(type(testStreet)) == "<type 'NoneType'>":
            # No street to correct!
            return True
        Application.DeselectAll()
        citySide = citySide*10000
        side1 = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart", False)
        side2 = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart2", False)
        rightBorder = side2.Kinematics.Global.posz.Value #side walk must test left of this mark
        num = 3
        oneMore = 1
        #roads
        while oneMore:
            #increment side walk at every pass
            side3 = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart" + str(num), False)
            if str(type(side3)) <> "<type 'NoneType'>":
                if rightBorder < side3.Kinematics.Global.posz.Value:
                    #no more road, side 3 is now part of a street
                    oneMore = 0
                else:
                    #go to the next sidewalk
                    num = num + 1
            else:
                oneMore = 0
        #streets
        rowTest = side3.Kinematics.Global.posz.Value #side walk must test equal to this mark
        firstRowTest = rowTest
        oneMore = 1
        numRoad = 3
        lastStreetX = 0
        firstStreet = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart" + str(num), False)
        activeStreetX = firstStreet.Kinematics.Global.posx.Value
        side1 = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart", False) #sidewalk0 and 1 are inverted, use 0 insted of 1 (then use numRoad)
        side2 = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart2", False)
        while oneMore:
            #increment side walk at every pass
            sideStreet = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart" + str(num), False)
            if str(type(sideStreet)) <> "<type 'NoneType'>":
                if rowTest <> sideStreet.Kinematics.Global.posz.Value:
                    newSideWalkR = Application.Duplicate(roadNull.Name + "." + side1.Name, "", 2, 1, 1, 0, 0, 1, 0, 1, "", "", "", "", "", "", "", "", "", "", 0)
                    newSideWalkL = Application.Duplicate(roadNull.Name + "." + side2.Name, "", 2, 1, 1, 0, 0, 1, 0, 1, "", "", "", "", "", "", "", "", "", "", 0)
                    newSideWalkR(0).Name = "newSideWalkR"
                    newSideWalkL(0).Name = "newSideWalkL"
                    newSideWalkR(0).Kinematics.Global.posx.Value = lastStreetX 
                    newSideWalkL(0).Kinematics.Global.posx.Value = lastStreetX
                    newSideWalkR(0).Kinematics.Global.sclx.Value = citySide - lastStreetX + 40
                    newSideWalkL(0).Kinematics.Global.sclx.Value = citySide - lastStreetX + 40
                    #no more street in this row
                    Application.DeleteObj(side1)
                    Application.DeleteObj(side2)
                    #Application.ToggleVisibility(side1, "", "")
                    #Application.ToggleVisibility(side2, "", "")
                    rowTest = sideStreet.Kinematics.Global.posz.Value
                    side1 = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart" + str(numRoad), False)
                    side2 = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart" + str(numRoad + 1), False)
                    numRoad = numRoad + 2
                    lastStreetX = 0
                    if side1.Kinematics.Global.posz.Value < side2.Kinematics.Global.posz.Value: # All streets sidewalks are done
                        oneMore = 0
                else:
                    newSideWalkR = Application.Duplicate(roadNull.Name + "." + side1.Name, "", 2, 1, 1, 0, 0, 1, 0, 1, "", "", "", "", "", "", "", "", "", "", 0)
                    newSideWalkL = Application.Duplicate(roadNull.Name + "." + side2.Name, "", 2, 1, 1, 0, 0, 1, 0, 1, "", "", "", "", "", "", "", "", "", "", 0)
                    newSideWalkR(0).Name = "newSideWalkR"
                    newSideWalkL(0).Name = "newSideWalkL"
                    newSideWalkR(0).Kinematics.Global.posx.Value = lastStreetX
                    newSideWalkL(0).Kinematics.Global.posx.Value = lastStreetX
                    newSideWalkR(0).Kinematics.Global.sclx.Value = activeStreetX - lastStreetX + 40
                    newSideWalkL(0).Kinematics.Global.sclx.Value = activeStreetX - lastStreetX + 40
                    otherSide = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart" + str(num + 1), False) # num + 1 to jump the road
                    lastStreetX = otherSide.Kinematics.Global.posx.Value
                    #go to the next sidewalk
                    num = num + 2
                    firstStreet = Application.Dictionary.GetObject(roadNull.Name + ".sideWalkPart" + str(num), False)
                    if str(type(firstStreet)) <> "<type 'NoneType'>":
                        activeStreetX = firstStreet.Kinematics.Global.posx.Value
                    else:
                        activeStreetX = citySide
            else:
                oneMore = 0
        #last few things to take care of
        newSideWalkR = Application.Duplicate(roadNull.Name + "." + side1.Name, "", 2, 1, 1, 0, 0, 1, 0, 1, "", "", "", "", "", "", "", "", "", "", 0)
        newSideWalkL = Application.Duplicate(roadNull.Name + "." + side2.Name, "", 2, 1, 1, 0, 0, 1, 0, 1, "", "", "", "", "", "", "", "", "", "", 0)
        newSideWalkR(0).Name = "newSideWalkR"
        newSideWalkL(0).Name = "newSideWalkL"
        newSideWalkR(0).Kinematics.Global.posx.Value = lastStreetX 
        newSideWalkL(0).Kinematics.Global.posx.Value = lastStreetX
        newSideWalkR(0).Kinematics.Global.sclx.Value = citySide - lastStreetX + 40
        newSideWalkL(0).Kinematics.Global.sclx.Value = citySide - lastStreetX + 40
        Application.DeleteObj(side1)
        Application.DeleteObj(side2)
        return True
        
    def MergeRoads(self, cityFront, citySide, roadNull = None, saturation = -0.5):
        # Roads
        Application.DeselectAll()
        allSelectRoad = Application.selection
        for road in roadNull.Children:
            if road.Name.find("roadPart") + 1:
                allSelectRoad.Add(road)
        
        allRoad = Application.ApplyGenOp("MeshMerge", "", allSelectRoad, 3, "siPersistentOperation", "siKeepGenOpInputs", "")
        mergedRoad = Application.Selection(0)
        Application.SetValue(mergedRoad.Fullname + ".polymsh.mergemesh.tolerance", 0, "")
        Application.DeleteGeneratorInputs(mergedRoad.Fullname + ".polymsh.mergemesh")
        mergedRoad.Parameters("Name").Value = "all_roads"
        
        roadNull.AddChild(mergedRoad)
        # Materials
        oMaterial = mergedRoad.AddMaterial( "Phong", False, "bush_Shader" );
        oPhong= mergedRoad.Material.Shaders(0)
        oPhong.Parameters("diffuse").Parameters("red").Value = 0.5
        oPhong.Parameters("diffuse").Parameters("green").Value = 0.5
        oPhong.Parameters("diffuse").Parameters("blue").Value = 0.5
        oPhong.Parameters("ambient").Parameters("red").Value = 0
        oPhong.Parameters("ambient").Parameters("green").Value = 0
        oPhong.Parameters("ambient").Parameters("blue").Value = 0
        oPhong.Parameters("shiny").Value = 100
        # Shaders
        oCheckerWall_0 = Application.CreateShaderFromProgID("Softimage.txt3d-checkerboard.1.0", oMaterial.FullName, "Checkerboard")
        oCheckerWall_1 = Application.CreateShaderFromProgID("Softimage.txt3d-checkerboard.1.0", oMaterial.FullName, "Checkerboard")
        oCheckerWall_2 = Application.CreateShaderFromProgID("Softimage.txt3d-checkerboard.1.0", oMaterial.FullName, "Checkerboard")
        oCloudWall = Application.CreateShaderFromProgID("Softimage.txt3d-cloud.1.0",oMaterial.FullName, "Cloud")
        oMix8Wall_0 = Application.CreateShaderFromProgID("Softimage.sib_color_8mix.1.0", oMaterial.FullName, "Mix_8_Colors")
        oCCWall = Application.CreateShaderFromProgID("Softimage.sib_color_correction.1.0", oMaterial.FullName, "Color_Correction")
        # Connections
        Application.SIConnectShaderToCnxPoint(oCCWall.FullName + ".out", oPhong.FullName + ".ambient", False)
        Application.SIConnectShaderToCnxPoint(oCCWall.FullName + ".out", oPhong.FullName + ".diffuse", False)
        Application.SIConnectShaderToCnxPoint(oCheckerWall_0.FullName + ".out", oMix8Wall_0.FullName + ".base_color", False)
        Application.SIConnectShaderToCnxPoint(oCheckerWall_1.FullName + ".out", oMix8Wall_0.FullName + ".color1", False)
        Application.SIConnectShaderToCnxPoint(oCheckerWall_2.FullName + ".out", oMix8Wall_0.FullName + ".color2", False)
        Application.SIConnectShaderToCnxPoint(oCloudWall.FullName + ".out", oMix8Wall_0.FullName + ".color3", False)
        Application.SIConnectShaderToCnxPoint(oMix8Wall_0.FullName + ".out", oCCWall.FullName + ".input", False)
        # Checker
        all_Checkers = [oCheckerWall_0, oCheckerWall_1, oCheckerWall_2]
        for iteration in range(0,3):
            color1 = random.uniform(0.4,0.6)
            color2 = random.uniform(0.4,0.6)
            all_Checkers[iteration].Parameters("color1").Parameters("red").Value = color1 + random.uniform(0.01,0.09)
            all_Checkers[iteration].Parameters("color1").Parameters("green").Value = color1 + random.uniform(0.01,0.09)
            all_Checkers[iteration].Parameters("color1").Parameters("blue").Value = color1 + random.uniform(0.01,0.09)
            all_Checkers[iteration].Parameters("color2").Parameters("red").Value = color2 + random.uniform(0.01,0.09)
            all_Checkers[iteration].Parameters("color2").Parameters("green").Value = color2 + random.uniform(0.01,0.09)
            all_Checkers[iteration].Parameters("color2").Parameters("blue").Value = color2 + random.uniform(0.01,0.09)
            all_Checkers[iteration].Parameters("xsize").Value = random.uniform(0.2,0.8)
            all_Checkers[iteration].Parameters("ysize").Value = random.uniform(0.2,0.8)
            all_Checkers[iteration].Parameters("zsize").Value = random.uniform(0.2,0.8)
        # Cloud
        color1 = random.uniform(0.7,0.75)
        color2 = random.uniform(0.3,0.35)
        oCloudWall.Parameters("color1").Parameters("red").Value = color1
        oCloudWall.Parameters("color1").Parameters("green").Value = color1
        oCloudWall.Parameters("color1").Parameters("blue").Value = color1
        oCloudWall.Parameters("color2").Parameters("red").Value = color2
        oCloudWall.Parameters("color2").Parameters("green").Value = color2
        oCloudWall.Parameters("color2").Parameters("blue").Value = color2
        oCloudWall.Parameters("level_decay").Value = random.uniform(0.1,0.9)
        oCloudWall.Parameters("level_min").Value = random.uniform(0.1,0.9)
        oCloudWall.Parameters("freq_mul").Value = random.uniform(1,3)
        oCloudWall.Parameters("iter_max").Value = random.randint(2,6)
        oCloudWall.Parameters("trans_range").Value = random.uniform(0.1,0.9)
        oCloudWall.Parameters("center_thresh").Value = random.uniform(-2,2)
        oCloudWall.Parameters("trans_amp").Value = random.uniform(0,2)
        oCloudWall.Parameters("edge_thresh").Value = random.uniform(-2,2)
        # Mix8Color
        for iteration in range(1,4):
            oMix8Wall_0.Parameters("weight" + str(iteration)).Parameters("red").Value = random.uniform(0.5,0.7)
            oMix8Wall_0.Parameters("weight" + str(iteration)).Parameters("green").Value = random.uniform(0.5,0.7)
            oMix8Wall_0.Parameters("weight" + str(iteration)).Parameters("blue").Value = random.uniform(0.5,0.7)
            oMix8Wall_0.Parameters("inuse" + str(iteration)).Value = True
            oMix8Wall_0.Parameters("mode" + str(iteration)).Value = 11 # Overlay
        # Color Correct
        oCCWall.Parameters("saturation").Value = saturation
        
        # Projection
        dummyName, projectionRoad = Application.CreateProjection(mergedRoad.FullName, "siTxtPlanarXZ", "siTxtDefaultSpherical", "Texture_Support", "Texture_Projection", "", "", "")
        Application.SetInstanceDataValue(mergedRoad.FullName, oCheckerWall_0.FullName + ".tspace_id", projectionRoad)
        Application.SetInstanceDataValue(mergedRoad.FullName, oCheckerWall_1.FullName + ".tspace_id", projectionRoad)
        Application.SetInstanceDataValue(mergedRoad.FullName, oCheckerWall_2.FullName + ".tspace_id", projectionRoad)
        Application.SetInstanceDataValue(mergedRoad.FullName, oCloudWall.FullName + ".tspace_id", projectionRoad)
        
        oCheckerWall_0.Parameters("repeats").Parameters("x").Value = cityFront * 10
        oCheckerWall_0.Parameters("repeats").Parameters("y").Value = citySide * 10
        oCheckerWall_1.Parameters("repeats").Parameters("x").Value = cityFront * 10
        oCheckerWall_1.Parameters("repeats").Parameters("y").Value = citySide * 10
        oCheckerWall_2.Parameters("repeats").Parameters("x").Value = cityFront * 10
        oCheckerWall_2.Parameters("repeats").Parameters("y").Value = citySide * 10
        oCloudWall.Parameters("repeats").Parameters("x").Value = cityFront * 5
        oCloudWall.Parameters("repeats").Parameters("y").Value = citySide * 5
        
        Application.FreezeObj(mergedRoad.Fullname, "", "")
        
        # Sidewalk
        Application.DeselectAll()
        allSelectSidewalk = Application.selection
        for sidewalk in roadNull.Children:
            if sidewalk.Name.find("sideWalkPart") + 1 or sidewalk.Name.find("newSideWalk") + 1:
                allSelectSidewalk.Add(sidewalk)
        
        allSidewalk = Application.ApplyGenOp("MeshMerge", "", allSelectSidewalk, 3, "siPersistentOperation", "siKeepGenOpInputs", "")
        mergedSidewalk = Application.Selection(0)
        Application.SetValue(mergedSidewalk.Fullname + ".polymsh.mergemesh.tolerance", 0, "")
        Application.DeleteGeneratorInputs(mergedSidewalk.Fullname + ".polymsh.mergemesh")
        mergedSidewalk.Parameters("Name").Value = "all_roads"
        
        roadNull.AddChild(mergedSidewalk)
        # Materials
        oMaterial = mergedSidewalk.AddMaterial( "Phong", False, "bush_Shader" );
        oPhong= mergedSidewalk.Material.Shaders(0)
        oPhong.Parameters("diffuse").Parameters("red").Value = 0.5
        oPhong.Parameters("diffuse").Parameters("green").Value = 0.5
        oPhong.Parameters("diffuse").Parameters("blue").Value = 0.5
        oPhong.Parameters("ambient").Parameters("red").Value = 0
        oPhong.Parameters("ambient").Parameters("green").Value = 0
        oPhong.Parameters("ambient").Parameters("blue").Value = 0
        oPhong.Parameters("shiny").Value = 100
        
        oFractal = Application.CreateShaderFromProgID("Softimage.txt3d-fractal_v3.1.0", oMaterial.FullName, "Fractal")
        
        Application.SIConnectShaderToCnxPoint(oFractal.FullName + ".out", oPhong.FullName + ".ambient", False)
        Application.SIConnectShaderToCnxPoint(oFractal.FullName + ".out", oPhong.FullName + ".diffuse", False)
        
        # Fractals
        oFractal.Parameters("color1").Parameters("red").Value = 0.87
        oFractal.Parameters("color1").Parameters("green").Value = 0.87
        oFractal.Parameters("color1").Parameters("blue").Value = 0.87
        oFractal.Parameters("color2").Parameters("red").Value = 0
        oFractal.Parameters("color2").Parameters("green").Value = 0
        oFractal.Parameters("color2").Parameters("blue").Value = 0
        oFractal.Parameters("threshold").Value = 0.55

        dummyName, projectionGround = Application.CreateProjection(mergedSidewalk.FullName, "siTxtPlanarXZ", "siTxtDefaultSpherical", "Texture_Support", "Texture_Projection", "", "", "")
        Application.SetInstanceDataValue(mergedSidewalk.FullName, oFractal.FullName + ".tspace_id", projectionGround)
        oFractal.Parameters("repeats").Parameters("x").Value = cityFront * 1000
        oFractal.Parameters("repeats").Parameters("y").Value = citySide * 1000
        
        Application.FreezeObj(mergedSidewalk.Fullname, "", "")
        return True

    def Parking(self, posX = 0, posY = 0, posZ = 0, orient = 0, frontMax = 50, sideMax = 50, cityFront = 1, citySide = 1, parkingNull = '', ground = None):
        activeObj = Application.ActiveSceneRoot.addGeometry("cube", "MeshSurface","parking")
        activeObj.Properties.find("geomapprox").gapproxmoan.Value = 30 # geo approximation angle
        Application.ApplyTopoOp("DeleteComponent", activeObj.Name+".poly[1]", "siUnspecified", "siPersistentOperation", "")
        # scale to 1 unit
        activeObj.Kinematics.Global.sclx.Value = 0.125 * frontMax * 10
        activeObj.Kinematics.Global.scly.Value = 0.125
        activeObj.Kinematics.Global.sclz.Value = 0.125 * sideMax * 10
        Application.MoveCtr2Vertices(activeObj.Name+".pnt[4,5]", 0) # put center to bottom front
        Application.ResetTransform(activeObj.Name, "siCtr", "siScl", "siXYZ")
        # apply the scale and put in place, rotate
        activeObj.Kinematics.Global.posx.Value = posX
        activeObj.Kinematics.Global.posy.Value = posY
        activeObj.Kinematics.Global.posz.Value = posZ
        activeObj.Kinematics.Global.roty.Value = orient
        # all parking child of the same null
        parkingNull.AddChild(activeObj)
        #lets apply a shader
        oMaterial = activeObj.AddMaterial( "Phong", False, "Parking_Shader" );
        oPhong= activeObj.Material.Shaders(0)
        oPhong.Parameters("diffuse").Parameters("red").Value = 1
        oPhong.Parameters("diffuse").Parameters("green").Value = 1
        oPhong.Parameters("diffuse").Parameters("blue").Value = 1
        oPhong.Parameters("ambient").Parameters("red").Value = 0
        oPhong.Parameters("ambient").Parameters("green").Value = 0
        oPhong.Parameters("ambient").Parameters("blue").Value = 0
        specular_color = random.uniform(0.1,0.4)
        oPhong.Parameters("specular").Parameters("red").Value = specular_color
        oPhong.Parameters("specular").Parameters("green").Value = specular_color
        oPhong.Parameters("specular").Parameters("blue").Value = specular_color
        oPhong.Parameters("shiny").Value = random.uniform(1,10)
        
        oRock = Application.CreateShaderFromProgID("Softimage.txt3d-rock_v3.1.0", oMaterial.FullName, "Rock1")
        oGrid = Application.CreateShaderFromProgID("Softimage.txt2d-grid.1.0", oMaterial.FullName, "Grid")
        oFabric = Application.CreateShaderFromProgID("Softimage.txt2d-fabric.1.0", oMaterial.FullName, "Fabric1")
        oFractal = Application.CreateShaderFromProgID("Softimage.txt3d-fractal_v3.1.0", oMaterial.FullName, "Fractal")
        oFractal1 = Application.CreateShaderFromProgID("Softimage.txt3d-fractal_v3.1.0", oMaterial.FullName, "Fractal")
        oMix8 = Application.CreateShaderFromProgID("Softimage.sib_color_8mix.1.0", oMaterial.FullName, "Mix_8_Colors")
        
        Application.SIConnectShaderToCnxPoint(oMix8.FullName + ".out", oPhong.FullName + ".ambient", False)
        Application.SIConnectShaderToCnxPoint(oMix8.FullName + ".out", oPhong.FullName + ".diffuse", False)
        Application.SIConnectShaderToCnxPoint(oRock.FullName + ".out", oMix8.FullName + ".base_color", False)
        Application.SIConnectShaderToCnxPoint(oGrid.FullName + ".out", oMix8.FullName + ".color1", False)
        Application.SIConnectShaderToCnxPoint(oFabric.FullName + ".out", oMix8.FullName + ".color2", False)
        Application.SIConnectShaderToCnxPoint(oFractal.FullName + ".out", oMix8.FullName + ".color3", False)
        Application.SIConnectShaderToCnxPoint(oFractal1.FullName + ".out", oFractal.FullName + ".color2", False)
        # Rock
        oRock.Parameters("color1").Parameters("red").Value = 1
        oRock.Parameters("color1").Parameters("green").Value = 1
        oRock.Parameters("color1").Parameters("blue").Value = 1
        oRock.Parameters("color2").Parameters("red").Value = 0
        oRock.Parameters("color2").Parameters("green").Value = 0
        oRock.Parameters("color2").Parameters("blue").Value = 0
        oRock.Parameters("grain_size").Value = 0.6
        oRock.Parameters("diffusion").Value = 0.05
        oRock.Parameters("mix_ratio").Value = random.uniform(0.45,0.55)
        # Grid
        oGrid.Parameters("line_color").Parameters("red").Value = 0
        oGrid.Parameters("line_color").Parameters("green").Value = 0
        oGrid.Parameters("line_color").Parameters("blue").Value = 0
        fill_color = random.uniform(0.6,0.95)
        oGrid.Parameters("fill_color").Parameters("red").Value = fill_color
        oGrid.Parameters("fill_color").Parameters("green").Value = fill_color
        oGrid.Parameters("fill_color").Parameters("blue").Value = fill_color
        oGrid.Parameters("diffusion").Value = random.uniform(0,0.15)
        oGrid.Parameters("u_width").Value = random.uniform(0.1,0.5)
        oGrid.Parameters("v_width").Value = random.uniform(0.1,0.5)
        # Fabric
        oFabric.Parameters("uthread_color").Parameters("red").Value = 0.2
        oFabric.Parameters("uthread_color").Parameters("green").Value = 0.2
        oFabric.Parameters("uthread_color").Parameters("blue").Value = 0.35
        oFabric.Parameters("vthread_color").Parameters("red").Value = 0.55
        oFabric.Parameters("vthread_color").Parameters("green").Value = 0.425
        oFabric.Parameters("vthread_color").Parameters("blue").Value = 0.3
        oFabric.Parameters("gap_color").Parameters("red").Value = 0.78
        oFabric.Parameters("gap_color").Parameters("green").Value = 0.36
        oFabric.Parameters("gap_color").Parameters("blue").Value = 0.36
        oFabric.Parameters("randomness").Value = 0.13
        oFabric.Parameters("uthread_width").Value = 0.684
        oFabric.Parameters("uthread_width").Value = 0.684
        oFabric.Parameters("u_wave").Value = 0.439
        oFabric.Parameters("u_wave").Value = 0.439
        # Fractals
        oFractal.Parameters("color1").Parameters("red").Value = 0.87
        oFractal.Parameters("color1").Parameters("green").Value = 0.87
        oFractal.Parameters("color1").Parameters("blue").Value = 0.87
        oFractal.Parameters("color2").Parameters("red").Value = 0
        oFractal.Parameters("color2").Parameters("green").Value = 0
        oFractal.Parameters("color2").Parameters("blue").Value = 0
        oFractal1.Parameters("color1").Parameters("red").Value = 0.7
        oFractal1.Parameters("color1").Parameters("green").Value = 0.7
        oFractal1.Parameters("color1").Parameters("blue").Value = 0.7
        oFractal1.Parameters("color2").Parameters("red").Value = 0
        oFractal1.Parameters("color2").Parameters("green").Value = 0
        oFractal1.Parameters("color2").Parameters("blue").Value = 0
        # Mix8Color
        for iteration in range(1,4):
            oMix8.Parameters("weight" + str(iteration)).Parameters("red").Value = 1
            oMix8.Parameters("weight" + str(iteration)).Parameters("green").Value = 1
            oMix8.Parameters("weight" + str(iteration)).Parameters("blue").Value = 1
            oMix8.Parameters("inuse" + str(iteration)).Value = True
            oMix8.Parameters("mode" + str(iteration)).Value = 4 # Multiply
        oMix8.Parameters("weight" + str(iteration)).Parameters("red").Value = 0.55
        oMix8.Parameters("weight" + str(iteration)).Parameters("green").Value = 0.55
        oMix8.Parameters("weight" + str(iteration)).Parameters("blue").Value = 0.55
        oMix8.Parameters("inuse" + str(iteration)).Value = True
        oMix8.Parameters("mode" + str(iteration)).Value = 6 # RGB Intensity
        
        allShaders = oMaterial.GetAllShaders()
        
        # Texture projecton
        dummyName, projectionParking = Application.CreateProjection(activeObj.FullName, "siTxtPlanarXZ", "siTxtDefaultSpherical", "Texture_Support", "groundProjection", "", "", "")
        dummyName, projectionParking1 = Application.CreateProjection(activeObj.FullName, "siTxtPlanarXZ", "siTxtDefaultSpherical", "Texture_Support1", "groundProjection1", "", "", "")
        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("groundProjection").NestedObjects("groundProjection_Def").Parameters("projsclu").Value = random.uniform(0.8,1) * (frontMax / 100)
        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("groundProjection").NestedObjects("groundProjection_Def").Parameters("projsclv").Value = random.uniform(0.8,1) * (sideMax / 100)
        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("groundProjection1").NestedObjects("groundProjection1_Def").Parameters("projsclu").Value = random.uniform(15,20) * (frontMax / 100)
        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("groundProjection1").NestedObjects("groundProjection1_Def").Parameters("projsclv").Value = random.uniform(15,20) * (sideMax / 100)

        for shad in allShaders:
            # Skip the shader itself, the mixers, Color To Scalar, Scalar Change Range, RGBA_Combine
            if(shad.progID == "Softimage.XSINormalMap3.1.0"):
                Application.SetInstanceDataValue(activeObj.FullName, oMaterial.FullName + "." + shad.Name + ".tspaceid", "groundProjection") # Different tspaceid argument
                Application.SetInstanceDataValue(activeObj.FullName, oMaterial.FullName + "." + shad.Name + ".Tangents", "Tangents");
            else:
                if(shad.progID <> "Softimage.material-phong.1.0" and shad.progID <> "Softimage.sib_color_8mix.1.0" and shad.progID <> "Softimage.sib_color_correction.1.0" and shad.progID <> "Softimage.sib_color_2mix.1.0" and shad.progID <> "Softimage.sib_color_to_scalar.1.0" and shad.progID <> "Softimage.sib_interp_linear.1.0" and shad.progID <> "Softimage.sib_color_combine.1.0"):
                    Application.SetInstanceDataValue(activeObj.FullName, oMaterial.FullName + "." + shad.Name + ".tspace_id", "groundProjection")

        # oFrabric use a 10x10 projection (projectionGround1)
        Application.SetInstanceDataValue(activeObj.FullName, oMaterial.FullName + "." + oFabric.Name + ".tspace_id", "groundProjection1")
        
        Application.FreezeObj(parkingNull.FullName + "." + activeObj.Name, "", "")

        Application.DeselectAll()
        return True
        
    def Vegetation(self, posX = 0, posY = 0, posZActive = 0, orient = 0, frontMax = 50, sideMax = 50, front = 40, side = 40, baseFront = 50, baseSide = 50, geo = "empty", vegetationMax = 30, vegetationMin = 5, vegetationNull = None ):
        activeObj = Application.ActiveSceneRoot.addGeometry("sphere", "MeshSurface","bush")
        activeObj.Properties.find("geomapprox").gapproxmoan.Value = 30 # geo approximation angle
        # Delete botom sphere
        Application.MoveCtr2Vertices(activeObj.Fullname + ".pnt[2,9,16,23,30,37,44,51]", "")
        Application.ApplyTopoOp("DeleteComponent", activeObj.Fullname + ".poly[0,8,16,24,32,40,48,56]", "siUnspecified", "siPersistentOperation", "")
        # Keep the original object for duplication
        originalObj = activeObj
        tempX = 0
        tempZ = 0
        for vegNumber in range(1,random.randint(vegetationMin, vegetationMax)):
            clear = False
            tempX = posX + (random.uniform(-frontMax / 2, frontMax / 2) * 10)
            if orient == 180:
                tempZ = posZActive + (random.uniform(0, sideMax) * 10)
                if tempZ > posZActive + (side * 10):
                    clear = True
            else:
                tempZ = posZActive - (random.uniform(0, sideMax) * 10)
                if tempZ < posZActive - (side * 10):
                    clear = True
            if clear or (tempX < posX - ((front / 2) * 10) or tempX > posX + ((front / 2) * 10)):
                clear = True
            if clear or (geo <> "Cube" and geo <> "Cylinder"):
                    Application.Duplicate(originalObj.Fullname, "", 2, 1, 1, 0, 0, 1, 0, 1, "", "", "", "", "", "", "", "", "", "", 0)
                    activeObj = Application.selection(0)
                    activeObj.Kinematics.Global.sclx.Value = random.uniform(4,15)
                    activeObj.Kinematics.Global.scly.Value = random.uniform(4,25)
                    activeObj.Kinematics.Global.sclz.Value = random.uniform(4,15)
                    activeObj.Kinematics.Global.posx.Value = tempX
                    activeObj.Kinematics.Global.posz.Value = tempZ
                    activeObj.Kinematics.Global.posy.Value = 0
                    Application.FreezeObj(activeObj.Fullname, "", "")
                    vegetationNull.AddChild(activeObj)
        Application.DeleteObj(originalObj.Fullname)
        return True
        
    def MergeVegetation(self, cityFront, citySide, saturation, vegetationNull = None):
        Application.DeselectAll()
        allSelection = Application.selection
        for vege in vegetationNull.Children:
            allSelection.Add(vege)
        
        allVegetation = Application.ApplyGenOp("MeshMerge", "", allSelection, 3, "siPersistentOperation", "siKeepGenOpInputs", "")
        mergedVege = Application.Selection(0)
        Application.SetValue(mergedVege.Fullname + ".polymsh.mergemesh.tolerance", 0, "")
        Application.DeleteGeneratorInputs(mergedVege.Fullname + ".polymsh.mergemesh")
        mergedVege.Parameters("Name").Value = "all_vegetation"
        vegetationNull.AddChild(mergedVege)
        # Materials
        oMaterial = mergedVege.AddMaterial( "Phong", False, "bush_Shader" );
        oPhong= mergedVege.Material.Shaders(0)
        oPhong.Parameters("diffuse").Parameters("red").Value = 0.08
        oPhong.Parameters("diffuse").Parameters("green").Value = 1
        oPhong.Parameters("diffuse").Parameters("blue").Value = 0.106
        oPhong.Parameters("ambient").Parameters("red").Value = 0
        oPhong.Parameters("ambient").Parameters("green").Value = 0
        oPhong.Parameters("ambient").Parameters("blue").Value = 0
        oPhong.Parameters("shiny").Value = 5
        # Shaders
        oFractal = Application.CreateShaderFromProgID("Softimage.txt3d-fractal_v3.1.0", oMaterial.FullName, "Fractal")
        oMarble = Application.CreateShaderFromProgID("Softimage.txt3d-marble.1.0", oMaterial.FullName, "Marble")
        oCC = Application.CreateShaderFromProgID("Softimage.sib_color_correction.1.0", oMaterial.FullName, "Color_Correction")
        # Connections
        Application.SIConnectShaderToCnxPoint(oCC.FullName + ".out", oPhong.FullName + ".diffuse", False)
        Application.SIConnectShaderToCnxPoint(oCC.FullName + ".out", oPhong.FullName + ".specular", False)
        Application.SIConnectShaderToCnxPoint(oMarble.FullName + ".out", oFractal.FullName + ".color1", False)
        Application.SIConnectShaderToCnxPoint(oFractal.FullName + ".out", oCC.FullName + ".input", False)
        # oFractal
        oFractal.Parameters("color1").Parameters("red").Value = random.uniform(0,0.3)
        oFractal.Parameters("color1").Parameters("green").Value = random.uniform(0.7,1)
        oFractal.Parameters("color1").Parameters("blue").Value = random.uniform(0,0.3)
        oFractal.Parameters("color2").Parameters("red").Value = 0
        oFractal.Parameters("color2").Parameters("green").Value = 0
        oFractal.Parameters("color2").Parameters("blue").Value = 0
        oFractal.Parameters("noise_type").Value = 1
        oFractal.Parameters("iterations").Value = 3
        oFractal.Parameters("diffusion").Value = 0
        oFractal.Parameters("upper_bound").Value = random.uniform(0.7,1)
        oFractal.Parameters("freq_mul").Value = 0.15
        oFractal.Parameters("level_min").Value = 0
        oFractal.Parameters("level_decay").Value = 0.15
        oFractal.Parameters("threshold").Value = 0
        oFractal.Parameters("time").Value = 0
        # oMarble
        oMarble.Parameters("vein_col1").Parameters("red").Value = random.uniform(0.7,0.9)
        oMarble.Parameters("vein_col1").Parameters("green").Value = random.uniform(0.8,1)
        oMarble.Parameters("vein_col1").Parameters("blue").Value = random.uniform(0.05,0.15)
        oMarble.Parameters("vein_col2").Parameters("red").Value = random.uniform(0.8,1)
        oMarble.Parameters("vein_col2").Parameters("green").Value = random.uniform(0.4,0.6)
        oMarble.Parameters("vein_col2").Parameters("blue").Value = random.uniform(0.05,0.2)
        oMarble.Parameters("filler_col").Parameters("red").Value = random.uniform(0.07,0.09)
        oMarble.Parameters("filler_col").Parameters("green").Value = random.uniform(0.7,0.8)
        oMarble.Parameters("filler_col").Parameters("blue").Value = random.uniform(0.07,0.09)
        oMarble.Parameters("vein_width").Value = random.uniform(0.15,0.18)
        oMarble.Parameters("diffusion").Value = random.uniform(0.25,0.35)
        oMarble.Parameters("spot_color").Parameters("red").Value = random.uniform(0.45,0.55)
        oMarble.Parameters("spot_color").Parameters("green").Value = random.uniform(0.45,0.55)
        oMarble.Parameters("spot_color").Parameters("blue").Value = random.uniform(0.45,0.55)
        oMarble.Parameters("spot_density").Value = random.uniform(0,0.5)
        oMarble.Parameters("spot_bias").Value = random.uniform(0.005,0.015)
        oMarble.Parameters("spot_scale").Value = random.uniform(0.7,0.8)
        oMarble.Parameters("amplitude").Value = random.uniform(0,5)
        oMarble.Parameters("ratio").Value = random.uniform(0,1)
        oMarble.Parameters("complexity").Value = random.uniform(1,20)
        oMarble.Parameters("frequencies").Parameters("x").Value = random.uniform(0.3,0.4)
        oMarble.Parameters("frequencies").Parameters("y").Value = random.uniform(0.2,0.3)
        oMarble.Parameters("frequencies").Parameters("z").Value = random.uniform(0.35,0.45)
        # Color Correct
        if saturation < 0:
            oCC.Parameters("saturation").Value = saturation / 2 # for a bit brighter vegetation
        else:
            oCC.Parameters("saturation").Value = saturation
        
        dummyName, projectionGround = Application.CreateProjection(mergedVege.FullName, "siTxtPlanarXZ", "siTxtDefaultSpherical", "Texture_Support", "Texture_Projection", "", "", "")
        Application.SetInstanceDataValue(mergedVege.FullName, oMarble.FullName + ".tspace_id", projectionGround)
        dummyName, projectionGround1 = Application.CreateProjection(mergedVege.FullName, "siTxtSpatial", "siTxtDefaultSpherical", "Texture_Support1", "Texture_Projection1", "", "", "")
        Application.SetInstanceDataValue(mergedVege.FullName, oFractal.FullName + ".tspace_id", projectionGround1)
        
        oMarble.Parameters("repeats").Parameters("x").Value = cityFront / 20
        oMarble.Parameters("repeats").Parameters("y").Value = citySide / 20
        
        Application.FreezeObj(mergedVege.Fullname, "", "")
        return True
        
    def Building(self, shader_dict, shader_roof, geometry = 'Cube', posX = 0, posY = 0, posZ = 0, orient = 0, stories = 80, storyMax = 80, front = 50, side = 50, baseOn = True, bevel = 20, bevelQuality = 0, baseNull = None, buildingNull = None):
        activeObj = Application.ActiveSceneRoot.addGeometry(geometry, "MeshSurface","building")
        activeObj.Properties.find("geomapprox").gapproxmoan.Value = 30 # geo approximation angle
        #tangent
        Application.GenerateUniqueUVs(activeObj.FullName, "Texcoord", "", "", "", "", "", "", "")
        Application.CreateVertexColorSupport("ColorAtVertices", "Tangents", activeObj.FullName, "")
        Application.ChangeVertexColorDatatype(activeObj.FullName + ".polymsh.cls.Texture_Coordinates_AUTO.Tangents", 1, "")
        Application.ApplyOp("TangentOp2_cpp", activeObj.FullName + ".polymsh.cls.Texture_Coordinates_AUTO.Tangents;" + activeObj.FullName + ".polymsh.cls.Texture_Coordinates_AUTO.Texcoord", "siUnspecified", "siPersistentOperation", "", 2)
        if geometry == "Cube":
            # if cube
            Application.ApplyTopoOp("DeleteComponent", activeObj.Name+".poly[1]", "siUnspecified", "siPersistentOperation", "")
            Application.MoveCtr2Vertices(activeObj.Name+".pnt[4,5]", 0) # put center to bottom front
            # scale to 1 unit so all scales are related
            activeObj.Kinematics.Global.sclx.Value = 0.125
            activeObj.Kinematics.Global.scly.Value = 0.125
            activeObj.Kinematics.Global.sclz.Value = 0.125
            Application.ResetTransform(activeObj.Name, "siCtr", "siScl", "siXYZ")
            activeObj.Kinematics.Global.sclx.Value = activeObj.Kinematics.Global.sclx.Value * front * 10
            activeObj.Kinematics.Global.scly.Value = activeObj.Kinematics.Global.scly.Value * stories * 4 * 10 #one story = 4m
            activeObj.Kinematics.Global.sclz.Value = activeObj.Kinematics.Global.sclz.Value * side * 10
            # apply bevel to remove hard edges
            Application.ApplyTopoOp("BevelComponent", activeObj.Name+".edge[0-2,7-LAST]", "siUnspecified", "siPersistentOperation", "")
            Application.SetValue(activeObj.Name+".polymsh.bevelop.ratio", random.uniform(0.01,0.15), "")
            if random.randint(0,100) < bevel and baseOn == False : # in some cases, use bevel as a modeling tool on the cube
                Application.SetValue(activeObj.Name+".polymsh.bevelop.ratio", random.uniform(0.2,2), "")
            Application.SetValue(activeObj.Name+".polymsh.bevelop.nbsubdivs", bevelQuality, "")
            Application.FreezeObj(activeObj)
        else:
            # if cylinder, reduce geometry but put more sides
            activeObj.Parameters("subdivu").Value = 24
            activeObj.Parameters("subdivbase").Value = 1
            activeObj.Parameters("subdivv").Value = 1
            Application.ApplyTopoOp("DeleteComponent", activeObj.Name+".poly[0,3,6,9,12,15,18,21,24,27,30,33,36,39,42,45,48,51,54,57,60,63,66,69]", "siUnspecified", "siPersistentOperation", "")
            # scale to 1 unit
            activeObj.Kinematics.Global.sclx.Value = 0.5
            activeObj.Kinematics.Global.scly.Value = 0.25
            activeObj.Kinematics.Global.sclz.Value = 0.5
            Application.MoveCtr2Vertices(activeObj.Name+".pnt[14]", 0) # put center to bottom front
            Application.ResetTransform(activeObj.Name, "siCtr", "siScl", "siXYZ")
            activeObj.Kinematics.Global.sclx.Value = activeObj.Kinematics.Global.sclx.Value * front * 10
            activeObj.Kinematics.Global.scly.Value = activeObj.Kinematics.Global.scly.Value * stories * 4 * 10
            activeObj.Kinematics.Global.sclz.Value = activeObj.Kinematics.Global.sclz.Value * side * 10
            # apply bevel to remove hard edges
            Application.ApplyTopoOp("BevelComponent", activeObj.Name+".edge[1,4,11,14,16,21,26,31,34,36,41,46,51,54,56,61,66,71,74,76,81,86,91,94]", "siUnspecified", "siPersistentOperation", "")
            Application.SetValue(activeObj.Name+".polymsh.bevelop.ratio", random.uniform(0.01,0.15), "")
            if random.randint(0,100) < bevel and baseOn == False: # in some cases, use bevel as a modeling tool on the cylinder
                Application.SetValue(activeObj.Name+".polymsh.bevelop.ratio", random.uniform(0.2,2), "")
            #Application.SetValue(activeObj.Name+".polymsh.bevelop.nbsubdivs", bevelQuality, "")
            Application.FreezeObj(activeObj) # freeze
        # put in place
        activeObj.Kinematics.Global.posx.Value = posX
        activeObj.Kinematics.Global.posy.Value = posY
        activeObj.Kinematics.Global.posz.Value = posZ
        activeObj.Kinematics.Global.roty.Value = orient
        # all building child of the same null
        if baseOn:
            baseNull.AddChild(activeObj)
            nullType = "base_null"
        else:
            buildingNull.AddChild(activeObj)
            nullType = "building_null"

        # lets apply a random shader from Shaders
        orderedkeys = shader_dict.keys() # list of shaders keys
        orderedkeys.sort()
        keynum = str(random.randint(int(orderedkeys[0]),(int(orderedkeys[0])+len(shader_dict))-1)) #wow! random number from the first key (we know is 'integer') to the last
        
        Application.SelectObj(shader_dict[keynum].FullName, "", "") # Trying to use dictionary.GetObject on this object fails...?  have to be selected.
        matObj = Application.selection(0) # Then we can access the object
        Application.DeselectAll() # Just in case
        
        Application.AssignMaterial(matObj.FullName+","+activeObj.FullName,"siLetLocalMaterialsOverlap")
        allShaders = matObj.GetAllShaders()
       
        if(geometry == "Cube"):
            Application.CreateProjection(activeObj.FullName, "siTxtCubic", "siTxtDefaultSpherical", "Texture_Support", "BuildingTXTProjection", "", "", "")
        else:
            Application.CreateProjection(activeObj.FullName, "siTxtCylindrical", "siTxtDefaultSpherical", "Texture_Support", "BuildingTXTProjection", "", "", "")

        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("BuildingTXTProjection").NestedObjects("BuildingTXTProjection_Def").Parameters("projsclu").Value = random.randint(1,2)
        activeObj.ActivePrimitive.Geometry.Clusters("Texture_Coordinates_AUTO").Properties("BuildingTXTProjection").NestedObjects("BuildingTXTProjection_Def").Parameters("projsclv").Value = random.randint(1,2)

        for shad in allShaders:
            # Skip the shader itself, the mixers, Color To Scalar, Scalar Change Range, RGBA_Combine
            if(shad.progID == "Softimage.XSINormalMap3.1.0"):
                Application.SetInstanceDataValue(activeObj.FullName, matObj.FullName + "." + shad.Name + ".tspaceid", "BuildingTXTProjection") # Different tspaceid argument
                Application.SetInstanceDataValue(activeObj.FullName, matObj.FullName + "." + shad.Name + ".Tangents", "Tangents");
            else:
                if(shad.progID <> "Softimage.material-phong.1.0" and shad.progID <> "Softimage.sib_color_8mix.1.0" and shad.progID <> "Softimage.sib_color_correction.1.0" and shad.progID <> "Softimage.sib_color_2mix.1.0" and shad.progID <> "Softimage.sib_color_to_scalar.1.0" and shad.progID <> "Softimage.sib_interp_linear.1.0" and shad.progID <> "Softimage.sib_color_combine.1.0"):
                    Application.SetInstanceDataValue(activeObj.FullName, matObj.FullName + "." + shad.Name + ".tspace_id", "BuildingTXTProjection")
                        
        TXTSupport = activeObj.Children("Texture_Support")   
        TXTSupport.Kinematics.Global.sclx.Value = 1.25 * storyMax * 4 #same as height for square results
        TXTSupport.Kinematics.Global.scly.Value = 1.25 * storyMax * 4 #one story = 4m
        TXTSupport.Kinematics.Global.sclz.Value = 1.25 * storyMax * 4
        
        Application.FreezeObj(nullType+"."+activeObj.Name, "", "")

        # lets apply a random shader from Shaders for the roof
        orderedkeys = shader_roof.keys() # list of shaders keys
        orderedkeys.sort()
        keynum = str(random.randint(int(orderedkeys[0]),(int(orderedkeys[0])+len(shader_roof))-1)) #wow! random number from the first key (we know is 'integer') to the last
        
        Application.SelectObj(shader_roof[keynum].FullName, "", "") # Let not takes any chances and use the same trick than above to get the material
        matObj = Application.selection(0) # Then we can access the object
        Application.DeselectAll() # Just in case
        
        if(geometry == "Cube"):
            Application.SelectGeometryComponents(nullType + "." + activeObj.Name+".poly[4]")
        else:
            Application.SelectGeometryComponents(nullType + "." + activeObj.Name+".poly[0,2,5,6,8,11,12,14,17,18,20,23,24,26,29,30,32,35,36,38,41,42,44,47]")
        active_cluster = Application.CreateCluster("")
        Application.AssignMaterial(matObj.Fullname + "," + str(active_cluster))
        Application.CreateProjection(nullType + "." + activeObj.Name, "siTxtSpatial", "siTxtDefaultSpherical", "Texture_Support", "Texture_Projection", "", "", "")
        Application.SetInstanceDataValue(nullType + "." + activeObj.Name, matObj.Fullname + ".Fractal.tspace_id", "Texture_Projection")
        Application.SetInstanceDataValue(nullType + "." + activeObj.Name, matObj.Fullname + ".Flagstone.tspace_id", "Texture_Projection")
        Application.FreezeObj(nullType + "." + activeObj.Name)
        Application.DeselectAll()
        return True

    def ShadersBR(self, shaderBuild = 25, shaderRoof = 25, reflection = True, frontMax = 50, sideMax = 50, storyMax = 80, saturation = -0.5):
        oProgressBar = XSIUIToolkit.ProgressBar
        oProgressBar.Value = 0
        oProgressBar.Maximum = shaderBuild
        oProgressBar.Step = 1
        oProgressBar.CancelEnabled = True
        oProgressBar.Caption = "Creating Building's Shader."
        oProgressBar.Visible = True
        activeObj = Application.ActiveSceneRoot.addGeometry("cube", "MeshSurface","shader")
        activeObj.Properties.find("geomapprox").gapproxmoan.Value = 30 # geo approximation angle
        activeObj.Kinematics.Global.sclx.Value = 1.25 * frontMax
        activeObj.Kinematics.Global.scly.Value = 1.25 * storyMax * 4 #one story = 4m
        activeObj.Kinematics.Global.sclz.Value = 1.25 * sideMax
        Application.ResetTransform(activeObj.Name, "siCtr", "siScl", "siXYZ")
        Application.FreezeObj(activeObj) # freeze
        i=0
        shader_dict = {}
        while i < shaderBuild:
            #lets create a number of random color grid shaders
            oProgressBar.Caption = "Creating Building's Shader " + str(i)
            if oProgressBar.CancelPressed:
                break
                
            # Building Material ############################################################################################
            # create a new phong material
            oMaterial = activeObj.AddMaterial( "Phong", False, "Building_Shader" );
            oPhong= activeObj.Material.Shaders(0)
            oPhong.Parameters("diffuse").Parameters("red").Value = 1
            oPhong.Parameters("diffuse").Parameters("green").Value = 1
            oPhong.Parameters("diffuse").Parameters("blue").Value = 1
            oPhong.Parameters("ambient").Parameters("red").Value = 0
            oPhong.Parameters("ambient").Parameters("green").Value = 0
            oPhong.Parameters("ambient").Parameters("blue").Value = 0
            specular_color = random.uniform(0.1,0.4)
            oPhong.Parameters("specular").Parameters("red").Value = specular_color
            oPhong.Parameters("specular").Parameters("green").Value = specular_color
            oPhong.Parameters("specular").Parameters("blue").Value = specular_color
            oPhong.Parameters("shiny").Value = random.uniform(1,10)
            # Wall creation
            oGridBuilding = Application.CreateShaderFromProgID("Softimage.txt2d-grid.1.0", oMaterial.FullName, "Grid")
            oGridMatte = Application.CreateShaderFromProgID("Softimage.txt2d-grid.1.0", oMaterial.FullName, "GridMatte")
            oCheckerWall_0 = Application.CreateShaderFromProgID("Softimage.txt3d-checkerboard.1.0", oMaterial.FullName, "Checkerboard")
            oCheckerWall_1 = Application.CreateShaderFromProgID("Softimage.txt3d-checkerboard.1.0", oMaterial.FullName, "Checkerboard")
            oCheckerWall_2 = Application.CreateShaderFromProgID("Softimage.txt3d-checkerboard.1.0", oMaterial.FullName, "Checkerboard")
            oCloudWall = Application.CreateShaderFromProgID("Softimage.txt3d-cloud.1.0",oMaterial.FullName, "Cloud")
            oMix8Wall_0 = Application.CreateShaderFromProgID("Softimage.sib_color_8mix.1.0", oMaterial.FullName, "Mix_8_Colors")
            oCCWall = Application.CreateShaderFromProgID("Softimage.sib_color_correction.1.0", oMaterial.FullName, "Color_Correction")
            # Concrete, stones, bricks
            wallType = random.randint(1,3)
            if wallType == 1:
                oMix2Wall = Application.CreateShaderFromProgID("Softimage.sib_color_2mix.1.0", oMaterial.FullName, "Mix_2_Colors")
                oGridWall_0 = Application.CreateShaderFromProgID("Softimage.txt2d-grid.1.0", oMaterial.FullName, "GridMatte")
            if wallType == 2:
                oGridWall_0 = Application.CreateShaderFromProgID("Softimage.txt2d-grid.1.0", oMaterial.FullName, "GridMatte")
                oFlagstoneWall = Application.CreateShaderFromProgID("Softimage.txt3d-flagstone_v3.1.0", oMaterial.FullName, "Flagstone")
                oMix8Wall_1 = Application.CreateShaderFromProgID("Softimage.sib_color_8mix.1.0", oMaterial.FullName, "Mix_8_Colors")
            if wallType == 3:
                oGridWall_0 = Application.CreateShaderFromProgID("Softimage.txt2d-grid.1.0", oMaterial.FullName, "GridMatte")
                oGridWall_1 = Application.CreateShaderFromProgID("Softimage.txt2d-grid.1.0", oMaterial.FullName, "GridMatte")
                oMix8Wall_2 = Application.CreateShaderFromProgID("Softimage.sib_color_8mix.1.0", oMaterial.FullName, "Mix_8_Colors")
            
            # Wall connections
            Application.SIConnectShaderToCnxPoint(oGridBuilding.FullName + ".out", oPhong.FullName + ".ambient", False)
            Application.SIConnectShaderToCnxPoint(oGridBuilding.FullName + ".out", oPhong.FullName + ".diffuse", False)
            Application.SIConnectShaderToCnxPoint(oGridMatte.FullName + ".out", oPhong.FullName + ".reflectivity", False)
            Application.SIConnectShaderToCnxPoint(oCheckerWall_0.FullName + ".out", oMix8Wall_0.FullName + ".base_color", False)
            Application.SIConnectShaderToCnxPoint(oCheckerWall_1.FullName + ".out", oMix8Wall_0.FullName + ".color1", False)
            Application.SIConnectShaderToCnxPoint(oCheckerWall_2.FullName + ".out", oMix8Wall_0.FullName + ".color2", False)
            Application.SIConnectShaderToCnxPoint(oCloudWall.FullName + ".out", oMix8Wall_0.FullName + ".color3", False)
            Application.SIConnectShaderToCnxPoint(oMix8Wall_0.FullName + ".out", oCCWall.FullName + ".input", False)
            if wallType == 1:
                Application.SIConnectShaderToCnxPoint(oCCWall.FullName + ".out", oMix2Wall.FullName + ".base_color", False)
                Application.SIConnectShaderToCnxPoint(oGridWall_0.FullName + ".out", oMix2Wall.FullName + ".color1", False)
                Application.SIConnectShaderToCnxPoint(oMix2Wall.FullName + ".out", oGridBuilding.FullName + ".line_color", False)
            if wallType == 2:
                Application.SIConnectShaderToCnxPoint(oCCWall.FullName + ".out", oMix8Wall_1.FullName + ".base_color", False)
                Application.SIConnectShaderToCnxPoint(oGridWall_0.FullName + ".out", oMix8Wall_1.FullName + ".color1", False)
                Application.SIConnectShaderToCnxPoint(oFlagstoneWall.FullName + ".out", oMix8Wall_1.FullName + ".color2", False)
                Application.SIConnectShaderToCnxPoint(oMix8Wall_1.FullName + ".out", oGridBuilding.FullName + ".line_color", False)
            if wallType == 3:
                Application.SIConnectShaderToCnxPoint(oCCWall.FullName + ".out", oMix8Wall_2.FullName + ".base_color", False)
                Application.SIConnectShaderToCnxPoint(oGridWall_0.FullName + ".out", oMix8Wall_2.FullName + ".color1", False)
                Application.SIConnectShaderToCnxPoint(oGridWall_1.FullName + ".out", oMix8Wall_2.FullName + ".color2", False)
                Application.SIConnectShaderToCnxPoint(oMix8Wall_2.FullName + ".out", oGridBuilding.FullName + ".line_color", False)
            # Wall Parameters
            # Grid Building
            oGridBuilding.Parameters("diffusion").Value = 0
            u_building = random.uniform(0.02,0.9) / 2
            v_building = random.uniform(0.02,0.9) / 2
            oGridBuilding.Parameters("u_width").Value = u_building
            oGridBuilding.Parameters("v_width").Value = v_building
            # Grid Matte
            oGridMatte.Parameters("line_color").Parameters("red").Value = 0
            oGridMatte.Parameters("line_color").Parameters("green").Value = 0
            oGridMatte.Parameters("line_color").Parameters("blue").Value = 0
            matteColor = random.uniform(0.6,0.95)
            oGridMatte.Parameters("fill_color").Parameters("red").Value = matteColor
            oGridMatte.Parameters("fill_color").Parameters("green").Value = matteColor
            oGridMatte.Parameters("fill_color").Parameters("blue").Value = matteColor
            oGridMatte.Parameters("diffusion").Value = random.uniform(0,0.05)
            Application.CopyPaste(oGridBuilding.FullName + ".u_width", "", oGridMatte.FullName + ".u_width", 1)
            Application.CopyPaste(oGridBuilding.FullName + ".v_width", "", oGridMatte.FullName + ".v_width", 1)
            # Checker
            all_Checkers = [oCheckerWall_0, oCheckerWall_1, oCheckerWall_2]
            for iteration in range(0,3):
                all_Checkers[iteration].Parameters("color1").Parameters("red").Value = random.uniform(0.01,0.09)
                all_Checkers[iteration].Parameters("color1").Parameters("green").Value = random.uniform(0.01,0.09)
                all_Checkers[iteration].Parameters("color1").Parameters("blue").Value = random.uniform(0.01,0.09)
                all_Checkers[iteration].Parameters("color2").Parameters("red").Value = random.uniform(0,1)
                all_Checkers[iteration].Parameters("color2").Parameters("green").Value = random.uniform(0,1)
                all_Checkers[iteration].Parameters("color2").Parameters("blue").Value = random.uniform(0,1)
                all_Checkers[iteration].Parameters("xsize").Value = random.uniform(0.2,0.8)
                all_Checkers[iteration].Parameters("ysize").Value = random.uniform(0.2,0.8)
                all_Checkers[iteration].Parameters("zsize").Value = random.uniform(0.2,0.8)
            # Cloud
            color1 = random.uniform(0.7,0.75)
            color2 = random.uniform(0.3,0.35)
            oCloudWall.Parameters("color1").Parameters("red").Value = color1
            oCloudWall.Parameters("color1").Parameters("green").Value = color1
            oCloudWall.Parameters("color1").Parameters("blue").Value = color1
            oCloudWall.Parameters("color2").Parameters("red").Value = color2
            oCloudWall.Parameters("color2").Parameters("green").Value = color2
            oCloudWall.Parameters("color2").Parameters("blue").Value = color2
            oCloudWall.Parameters("level_decay").Value = random.uniform(0.1,0.9)
            oCloudWall.Parameters("level_min").Value = random.uniform(0.1,0.9)
            oCloudWall.Parameters("freq_mul").Value = random.uniform(1,3)
            oCloudWall.Parameters("iter_max").Value = random.randint(2,6)
            oCloudWall.Parameters("trans_range").Value = random.uniform(0.1,0.9)
            oCloudWall.Parameters("center_thresh").Value = random.uniform(-2,2)
            oCloudWall.Parameters("trans_amp").Value = random.uniform(0,2)
            oCloudWall.Parameters("edge_thresh").Value = random.uniform(-2,2)
            # Mix8Color
            for iteration in range(1,5):
                oMix8Wall_0.Parameters("weight" + str(iteration)).Parameters("red").Value = random.uniform(0.5,0.7)
                oMix8Wall_0.Parameters("weight" + str(iteration)).Parameters("green").Value = random.uniform(0.5,0.7)
                oMix8Wall_0.Parameters("weight" + str(iteration)).Parameters("blue").Value = random.uniform(0.5,0.7)
                oMix8Wall_0.Parameters("inuse" + str(iteration)).Value = True
                oMix8Wall_0.Parameters("mode" + str(iteration)).Value = 9 # Difference (Substract) mode
            oMix8Wall_0.Parameters("mode4").Value = 4 # The last one change to Hide/Reveal Multiply
            # Color Correct
            oCCWall.Parameters("saturation").Value = saturation
            if wallType == 1:
                oMix2Wall.Parameters("weight" + str(iteration)).Parameters("red").Value = random.uniform(0.5,0.7)
                oMix2Wall.Parameters("weight" + str(iteration)).Parameters("green").Value = random.uniform(0.5,0.7)
                oMix2Wall.Parameters("weight" + str(iteration)).Parameters("blue").Value = random.uniform(0.5,0.7)
                oMix2Wall.Parameters("mode1").Value = 11 # Offset Overlay mode
                oGridWall_0.Parameters("fill_color").Parameters("red").Value = 0
                oGridWall_0.Parameters("fill_color").Parameters("green").Value = 0
                oGridWall_0.Parameters("fill_color").Parameters("blue").Value = 0
                oGridWall_0.Parameters("line_color").Parameters("red").Value = 1
                oGridWall_0.Parameters("line_color").Parameters("green").Value = 1
                oGridWall_0.Parameters("line_color").Parameters("blue").Value = 1
                oGridWall_0.Parameters("contrast").Value = 1
                oGridWall_0.Parameters("diffusion").Value = 0
                oGridWall_0.Parameters("u_width").Value = u_building - random.uniform(0,0.1)
                oGridWall_0.Parameters("v_width").Value = v_building - random.uniform(0,0.1)
            if wallType == 2:
                oGridWall_0.Parameters("fill_color").Parameters("red").Value = 0
                oGridWall_0.Parameters("fill_color").Parameters("green").Value = 0
                oGridWall_0.Parameters("fill_color").Parameters("blue").Value = 0
                oGridWall_0.Parameters("line_color").Parameters("red").Value = 1
                oGridWall_0.Parameters("line_color").Parameters("green").Value = 1
                oGridWall_0.Parameters("line_color").Parameters("blue").Value = 1
                oGridWall_0.Parameters("contrast").Value = 1
                oGridWall_0.Parameters("diffusion").Value = 0
                oGridWall_0.Parameters("u_width").Value = u_building - random.uniform(0,0.1)
                oGridWall_0.Parameters("v_width").Value = v_building - random.uniform(0,0.1)
                color1 = random.uniform(0,0.85)
                oFlagstoneWall.Parameters("color1").Parameters("red").Value = color1 + random.uniform(0,0.15)
                oFlagstoneWall.Parameters("color1").Parameters("green").Value = color1 + random.uniform(0,0.15)
                oFlagstoneWall.Parameters("color1").Parameters("blue").Value = color1 + random.uniform(0,0.15)
                oFlagstoneWall.Parameters("color2").Parameters("red").Value = random.uniform(0.01,0.09)
                oFlagstoneWall.Parameters("color2").Parameters("green").Value = random.uniform(0.01,0.09)
                oFlagstoneWall.Parameters("color2").Parameters("blue").Value = random.uniform(0.01,0.09)
                oFlagstoneWall.Parameters("mortar_width").Value = random.uniform(0,0.3)
                oMix8Wall_1.Parameters("weight1").Parameters("red").Value = random.uniform(0.5,0.7)
                oMix8Wall_1.Parameters("weight1").Parameters("green").Value = random.uniform(0.5,0.7)
                oMix8Wall_1.Parameters("weight1").Parameters("blue").Value = random.uniform(0.5,0.7)
                oMix8Wall_1.Parameters("inuse1").Value = True
                oMix8Wall_1.Parameters("mode1").Value = 11 # Offset Overlay mode
                oMix8Wall_1.Parameters("weight2").Parameters("red").Value = random.uniform(0.5,0.7)
                oMix8Wall_1.Parameters("weight2").Parameters("green").Value = random.uniform(0.5,0.7)
                oMix8Wall_1.Parameters("weight2").Parameters("blue").Value = random.uniform(0.5,0.7)
                oMix8Wall_1.Parameters("inuse2").Value = True
                oMix8Wall_1.Parameters("mode2").Value = 11 # Offset Overlay mode
            if wallType == 3:
                oGridWall_0.Parameters("fill_color").Parameters("red").Value = 0
                oGridWall_0.Parameters("fill_color").Parameters("green").Value = 0
                oGridWall_0.Parameters("fill_color").Parameters("blue").Value = 0
                oGridWall_0.Parameters("line_color").Parameters("red").Value = 1
                oGridWall_0.Parameters("line_color").Parameters("green").Value = 1
                oGridWall_0.Parameters("line_color").Parameters("blue").Value = 1
                oGridWall_0.Parameters("contrast").Value = 1
                oGridWall_0.Parameters("diffusion").Value = 0
                oGridWall_0.Parameters("u_width").Value = u_building - random.uniform(0,0.1)
                oGridWall_0.Parameters("v_width").Value = v_building - random.uniform(0,0.1)
                oGridWall_1.Parameters("fill_color").Parameters("red").Value = 0.75
                oGridWall_1.Parameters("fill_color").Parameters("green").Value = 0
                oGridWall_1.Parameters("fill_color").Parameters("blue").Value = 0
                oGridWall_1.Parameters("line_color").Parameters("red").Value = 1
                oGridWall_1.Parameters("line_color").Parameters("green").Value = 1
                oGridWall_1.Parameters("line_color").Parameters("blue").Value = 1
                oGridWall_1.Parameters("contrast").Value = 1
                oGridWall_1.Parameters("diffusion").Value = random.uniform(0.1,0.3)
                oGridWall_1.Parameters("v_width").Value = v_building - random.uniform(0.1,0.3)
                oGridWall_1.Parameters("u_width").Value = oGridWall_1.Parameters("v_width").Value / 2
                oMix8Wall_2.Parameters("weight1").Parameters("red").Value = random.uniform(0.5,0.7)
                oMix8Wall_2.Parameters("weight1").Parameters("green").Value = random.uniform(0.5,0.7)
                oMix8Wall_2.Parameters("weight1").Parameters("blue").Value = random.uniform(0.5,0.7)
                oMix8Wall_2.Parameters("inuse1").Value = True
                oMix8Wall_2.Parameters("mode1").Value = 11 # Offset Overlay mode
                oMix8Wall_2.Parameters("weight2").Parameters("red").Value = random.uniform(0.5,0.7)
                oMix8Wall_2.Parameters("weight2").Parameters("green").Value = random.uniform(0.5,0.7)
                oMix8Wall_2.Parameters("weight2").Parameters("blue").Value = random.uniform(0.5,0.7)
                oMix8Wall_2.Parameters("inuse2").Value = True
                oMix8Wall_2.Parameters("mode2").Value = 10 # HardLight mode

            # ##########################################################################################################
            
            # Glass creation
            oCell = Application.CreateShaderFromProgID("Softimage.txt3d-cellular_v3.1.0", oMaterial.FullName, "Cell")
            oChecker = Application.CreateShaderFromProgID("Softimage.txt3d-checkerboard.1.0", oMaterial.FullName, "Checkerboard")
            oCloud = Application.CreateShaderFromProgID("Softimage.txt3d-cloud.1.0",oMaterial.FullName, "Cloud")
            oFlagstone = Application.CreateShaderFromProgID("Softimage.txt3d-flagstone_v3.1.0", oMaterial.FullName, "Flagstone")
            oFractal = Application.CreateShaderFromProgID("Softimage.txt3d-fractal_v3.1.0", oMaterial.FullName, "Fractal")
            oGrid = Application.CreateShaderFromProgID("Softimage.txt2d-grid.1.0", oMaterial.FullName, "Grid")
            oMarble = Application.CreateShaderFromProgID("Softimage.txt3d-marble.1.0", oMaterial.FullName, "Marble")
            oWood = Application.CreateShaderFromProgID("Softimage.txt3d-wood.1.0", oMaterial.FullName, "Wood")
            oMix8 = Application.CreateShaderFromProgID("Softimage.sib_color_8mix.1.0", oMaterial.FullName, "Mix_8_Colors")
            oCC = Application.CreateShaderFromProgID("Softimage.sib_color_correction.1.0", oMaterial.FullName, "Color_Correction")
            # Reflectivity with normal maps
            if reflection == True: 
                oCloud0 = Application.CreateShaderFromProgID("Softimage.txt3d-cloud.1.0", oMaterial.FullName, "Cloud")
                oCloud1 = Application.CreateShaderFromProgID("Softimage.txt3d-cloud.1.0", oMaterial.FullName, "Cloud")
                oCloud2 = Application.CreateShaderFromProgID("Softimage.txt3d-cloud.1.0", oMaterial.FullName, "Cloud")
                oCloud3 = Application.CreateShaderFromProgID("Softimage.txt3d-cloud.1.0", oMaterial.FullName, "Cloud")
                oCloud4 = Application.CreateShaderFromProgID("Softimage.txt3d-cloud.1.0", oMaterial.FullName, "Cloud")
                oCloud5 = Application.CreateShaderFromProgID("Softimage.txt3d-cloud.1.0", oMaterial.FullName, "Cloud")
                oMix2_0 = Application.CreateShaderFromProgID("Softimage.sib_color_2mix.1.0", oMaterial.FullName, "Mix_2_Colors")
                oMix2_1 = Application.CreateShaderFromProgID("Softimage.sib_color_2mix.1.0", oMaterial.FullName, "Mix_2_Colors")
                oMix2_2 = Application.CreateShaderFromProgID("Softimage.sib_color_2mix.1.0", oMaterial.FullName, "Mix_2_Colors")
                oColorToScalar0 = Application.CreateShaderFromProgID("Softimage.sib_color_to_scalar.1.0", oMaterial.FullName, "Color_To_Scalar")
                oColorToScalar1 = Application.CreateShaderFromProgID("Softimage.sib_color_to_scalar.1.0", oMaterial.FullName, "Color_To_Scalar")
                oColorToScalar2 = Application.CreateShaderFromProgID("Softimage.sib_color_to_scalar.1.0", oMaterial.FullName, "Color_To_Scalar")
                oChangeRange0 = Application.CreateShaderFromProgID("Softimage.sib_interp_linear.1.0", oMaterial.FullName, "Scalar_Change_Range")
                oChangeRange1 = Application.CreateShaderFromProgID("Softimage.sib_interp_linear.1.0", oMaterial.FullName, "Scalar_Change_Range")
                oChangeRange2 = Application.CreateShaderFromProgID("Softimage.sib_interp_linear.1.0", oMaterial.FullName, "Scalar_Change_Range")
                oCombine = Application.CreateShaderFromProgID("Softimage.sib_color_combine.1.0", oMaterial.FullName, "RGBA_Combine")
                oNormalMap = Application.CreateShaderFromProgID("Softimage.XSINormalMap3.1.0", oMaterial.FullName, "XSINormalMap")
            # Glass connections
            Application.SIConnectShaderToCnxPoint(oCell.FullName + ".out", oMix8.FullName + ".base_color", False)
            Application.SIConnectShaderToCnxPoint(oChecker.FullName + ".out", oMix8.FullName + ".color1", False)
            Application.SIConnectShaderToCnxPoint(oCloud.FullName + ".out", oMix8.FullName + ".color2", False)
            Application.SIConnectShaderToCnxPoint(oFlagstone.FullName + ".out", oMix8.FullName + ".color3", False)
            Application.SIConnectShaderToCnxPoint(oFractal.FullName + ".out", oMix8.FullName + ".color4", False)
            Application.SIConnectShaderToCnxPoint(oGrid.FullName + ".out", oMix8.FullName + ".color5", False)
            Application.SIConnectShaderToCnxPoint(oMarble.FullName + ".out", oMix8.FullName + ".color6", False)
            Application.SIConnectShaderToCnxPoint(oWood.FullName + ".out", oMix8.FullName + ".color7", False)
            Application.SIConnectShaderToCnxPoint(oMix8.FullName + ".out", oCC.FullName + ".input", False)
            Application.SIConnectShaderToCnxPoint(oCC.FullName + ".out", oGridBuilding.FullName + ".fill_color", False)
            # Reflectivity with normal maps
            if reflection == True:
                Application.SIConnectShaderToCnxPoint(oCloud0.FullName + ".out", oMix2_0.FullName + ".base_color", False)
                Application.SIConnectShaderToCnxPoint(oCloud1.FullName + ".out", oMix2_0.FullName + ".color1", False)
                Application.SIConnectShaderToCnxPoint(oCloud2.FullName + ".out", oMix2_1.FullName + ".base_color", False)
                Application.SIConnectShaderToCnxPoint(oCloud3.FullName + ".out", oMix2_1.FullName + ".color1", False)
                Application.SIConnectShaderToCnxPoint(oCloud4.FullName + ".out", oMix2_2.FullName + ".base_color", False)
                Application.SIConnectShaderToCnxPoint(oCloud5.FullName + ".out", oMix2_2.FullName + ".color1", False)
                Application.SIConnectShaderToCnxPoint(oMix2_0.FullName + ".out", oColorToScalar0.FullName + ".input", False)
                Application.SIConnectShaderToCnxPoint(oMix2_1.FullName + ".out", oColorToScalar1.FullName + ".input", False)
                Application.SIConnectShaderToCnxPoint(oMix2_2.FullName + ".out", oColorToScalar2.FullName + ".input", False)
                Application.SIConnectShaderToCnxPoint(oColorToScalar0.FullName + ".out", oChangeRange0.FullName + ".input", False)
                Application.SIConnectShaderToCnxPoint(oColorToScalar1.FullName + ".out", oChangeRange1.FullName + ".input", False)
                Application.SIConnectShaderToCnxPoint(oColorToScalar2.FullName + ".out", oChangeRange2.FullName + ".input", False)
                Application.SIConnectShaderToCnxPoint(oChangeRange0.FullName + ".out", oCombine.FullName + ".red", False)
                Application.SIConnectShaderToCnxPoint(oChangeRange1.FullName + ".out", oCombine.FullName + ".green", False)
                Application.SIConnectShaderToCnxPoint(oChangeRange2.FullName + ".out", oCombine.FullName + ".blue", False)
                Application.SIConnectShaderToCnxPoint(oCombine.FullName + ".out", oNormalMap.FullName + ".Map", False)
                Application.SIConnectShaderToCnxPoint(oNormalMap.FullName + ".out", oMaterial.FullName + ".normal", False)
            # Glass Parameters
            # Mix8Color
            for iteration in range(1,8):
                oMix8.Parameters("weight" + str(iteration)).Parameters("red").Value = random.uniform(0.1,0.9)
                oMix8.Parameters("weight" + str(iteration)).Parameters("green").Value = random.uniform(0.1,0.9)
                oMix8.Parameters("weight" + str(iteration)).Parameters("blue").Value = random.uniform(0.1,0.9)
                oMix8.Parameters("inuse" + str(iteration)).Value = True
                oMix8.Parameters("mode" + str(iteration)).Value = 8 # Lighter mode
            # Cell
            color1 = random.uniform(0.7,1)
            color2 = random.uniform(0,0.3)
            oCell.Parameters("color1").Parameters("red").Value = color1
            oCell.Parameters("color1").Parameters("green").Value = color1
            oCell.Parameters("color1").Parameters("blue").Value = color1
            oCell.Parameters("color2").Parameters("red").Value = color2
            oCell.Parameters("color2").Parameters("green").Value = color2
            oCell.Parameters("color2").Parameters("blue").Value = color2
            # Checker
            color1 = random.uniform(0.7,1)
            color2 = random.uniform(0,0.3)
            oChecker.Parameters("color1").Parameters("red").Value = color1
            oChecker.Parameters("color1").Parameters("green").Value = color1
            oChecker.Parameters("color1").Parameters("blue").Value = color1
            oChecker.Parameters("color2").Parameters("red").Value = color2
            oChecker.Parameters("color2").Parameters("green").Value = color2
            oChecker.Parameters("color2").Parameters("blue").Value = color2
            oChecker.Parameters("xsize").Value = random.uniform(0.2,0.8)
            oChecker.Parameters("ysize").Value = random.uniform(0.2,0.8)
            oChecker.Parameters("zsize").Value = random.uniform(0.2,0.8)
            # Cloud
            oCloud.Parameters("color1").Parameters("red").Value = random.uniform(0.8,0.83)
            oCloud.Parameters("color1").Parameters("green").Value = random.uniform(0.8,0.83)
            oCloud.Parameters("color1").Parameters("blue").Value = random.uniform(0.8,0.83)
            oCloud.Parameters("color2").Parameters("red").Value = random.uniform(0.1,0.2)
            oCloud.Parameters("color2").Parameters("green").Value = random.uniform(0.1,0.2)
            oCloud.Parameters("color2").Parameters("blue").Value = random.uniform(0.1,0.2)
            oCloud.Parameters("level_decay").Value = random.uniform(0.1,0.9)
            oCloud.Parameters("level_min").Value = random.uniform(0.1,0.9)
            oCloud.Parameters("freq_mul").Value = random.uniform(1,3)
            oCloud.Parameters("iter_max").Value = random.randint(2,6)
            oCloud.Parameters("trans_range").Value = random.uniform(0.1,0.9)
            oCloud.Parameters("center_thresh").Value = random.uniform(-2,2)
            oCloud.Parameters("trans_amp").Value = random.uniform(0,2)
            oCloud.Parameters("edge_thresh").Value = random.uniform(-2,2)
            # Flagstone
            color1 = random.uniform(0.6,1)
            color2 = random.uniform(0,0.4)
            oFlagstone.Parameters("color1").Parameters("red").Value = color1
            oFlagstone.Parameters("color1").Parameters("green").Value = color1
            oFlagstone.Parameters("color1").Parameters("blue").Value = color1
            oFlagstone.Parameters("color2").Parameters("red").Value = color2
            oFlagstone.Parameters("color2").Parameters("green").Value = color2
            oFlagstone.Parameters("color2").Parameters("blue").Value = color2
            oFlagstone.Parameters("mortar_width").Value = random.uniform(0.05,0.25)
            # Fractal
            color1 = random.uniform(0.7,1)
            color2 = random.uniform(0,0.3)
            oFractal.Parameters("color1").Parameters("red").Value = color1
            oFractal.Parameters("color1").Parameters("green").Value = color1
            oFractal.Parameters("color1").Parameters("blue").Value = color1
            oFractal.Parameters("color2").Parameters("red").Value = color2
            oFractal.Parameters("color2").Parameters("green").Value = color2
            oFractal.Parameters("color2").Parameters("blue").Value = color2
            oFractal.Parameters("noise_type").Value = random.randint(0,1)
            oFractal.Parameters("upper_bound").Value = random.uniform(0.7,1)
            oFractal.Parameters("freq_mul").Value = random.uniform(0,4)
            oFractal.Parameters("level_decay").Value = random.uniform(0,1)
            oFractal.Parameters("threshold").Value = random.uniform(0,0.7)
            oFractal.Parameters("time").Value = random.uniform(0,100)
            # Grid
            oGrid.Parameters("fill_color").Parameters("red").Value = random.uniform(0.9,1)
            oGrid.Parameters("fill_color").Parameters("green").Value = random.uniform(0.9,1)
            oGrid.Parameters("fill_color").Parameters("blue").Value = random.uniform(0.9,1)
            oGrid.Parameters("line_color").Parameters("red").Value = random.uniform(0.9,1)
            oGrid.Parameters("line_color").Parameters("green").Value = random.uniform(0.75,0.85)
            oGrid.Parameters("line_color").Parameters("blue").Value = random.uniform(0.55,0.65)
            oGrid.Parameters("contrast").Value = 1
            oGrid.Parameters("diffusion").Value = random.uniform(0,0.05)
            oGrid.Parameters("u_width").Value = random.uniform(0.1,0.9)
            oGrid.Parameters("v_width").Value = random.uniform(0.1,0.9)
            # Marble
            oMarble.Parameters("vein_col1").Parameters("red").Value = random.uniform(0.4,0.6)
            oMarble.Parameters("vein_col1").Parameters("green").Value = random.uniform(0.4,0.6)
            oMarble.Parameters("vein_col1").Parameters("blue").Value = random.uniform(0.4,0.6)
            oMarble.Parameters("vein_col2").Parameters("red").Value = random.uniform(0.9,1)
            oMarble.Parameters("vein_col2").Parameters("green").Value = random.uniform(0.9,1)
            oMarble.Parameters("vein_col2").Parameters("blue").Value = random.uniform(0.9,1)
            oMarble.Parameters("filler_col").Parameters("red").Value = random.uniform(0.7,0.9)
            oMarble.Parameters("filler_col").Parameters("green").Value = random.uniform(0.7,0.9)
            oMarble.Parameters("filler_col").Parameters("blue").Value = random.uniform(0.7,0.9)
            oMarble.Parameters("vein_width").Value = random.uniform(0.15,0.18)
            oMarble.Parameters("diffusion").Value = random.uniform(0.25,0.35)
            oMarble.Parameters("spot_color").Parameters("red").Value = random.uniform(0.9,1)
            oMarble.Parameters("spot_color").Parameters("green").Value = random.uniform(0.9,1)
            oMarble.Parameters("spot_color").Parameters("blue").Value = random.uniform(0.9,1)
            oMarble.Parameters("spot_density").Value = random.uniform(0,0.5)
            oMarble.Parameters("spot_bias").Value = random.uniform(0.005,0.015)
            oMarble.Parameters("spot_scale").Value = random.uniform(0.7,0.8)
            oMarble.Parameters("amplitude").Value = random.uniform(0,5)
            oMarble.Parameters("ratio").Value = random.uniform(0,1)
            oMarble.Parameters("complexity").Value = random.uniform(1,20)
            oMarble.Parameters("frequencies").Parameters("x").Value = random.uniform(0.3,0.4)
            oMarble.Parameters("frequencies").Parameters("y").Value = random.uniform(0.2,0.3)
            oMarble.Parameters("frequencies").Parameters("z").Value = random.uniform(0.35,0.45)
            # Wood
            color1 = random.uniform(0.7,1)
            color2 = random.uniform(0,0.3)
            oWood.Parameters("filler_col").Parameters("red").Value = color1
            oWood.Parameters("filler_col").Parameters("green").Value = color1
            oWood.Parameters("filler_col").Parameters("blue").Value = color1
            oWood.Parameters("vein_col").Parameters("red").Value = color2
            oWood.Parameters("vein_col").Parameters("green").Value = color2
            oWood.Parameters("vein_col").Parameters("blue").Value = color2
            oWood.Parameters("vein_spread").Value = random.uniform(1,4)
            oWood.Parameters("age").Value = random.uniform(0,25)
            oWood.Parameters("randomness").Value = random.uniform(0.1,0.9)
            oWood.Parameters("center_u").Value = random.uniform(-2,2)
            oWood.Parameters("center_v").Value = random.uniform(-2,2)
            oWood.Parameters("ratio").Value = random.uniform(5,15)
            oWood.Parameters("vamplitude").Value = random.uniform(0.2,2)
            # CC
            oCC.Parameters("saturation").Value = random.uniform(-0.45,-0.75)
            # Reflectivity with normal maps
            cloudArray = [oCloud0, oCloud1, oCloud2, oCloud3, oCloud4, oCloud5]
            changeRangeArray = [oChangeRange0, oChangeRange1, oChangeRange2]
            if reflection == True:
                for iteration in range(0,5):
                    cloudArray[iteration].Parameters("color1").Parameters("red").Value = random.uniform(0.8,0.83)
                    cloudArray[iteration].Parameters("color1").Parameters("green").Value = random.uniform(0.8,0.83)
                    cloudArray[iteration].Parameters("color1").Parameters("blue").Value = random.uniform(0.8,0.83)
                    cloudArray[iteration].Parameters("color2").Parameters("red").Value = random.uniform(0.1,0.2)
                    cloudArray[iteration].Parameters("color2").Parameters("green").Value = random.uniform(0.1,0.2)
                    cloudArray[iteration].Parameters("color2").Parameters("blue").Value = random.uniform(0.1,0.2)
                    cloudArray[iteration].Parameters("level_decay").Value = random.uniform(0.1,0.9)
                    cloudArray[iteration].Parameters("level_min").Value = random.uniform(0.1,0.9)
                    cloudArray[iteration].Parameters("freq_mul").Value = random.uniform(1,3)
                    cloudArray[iteration].Parameters("iter_max").Value = random.randint(2,6)
                    cloudArray[iteration].Parameters("trans_range").Value = random.uniform(0.1,0.9)
                    cloudArray[iteration].Parameters("center_thresh").Value = random.uniform(-2,2)
                    cloudArray[iteration].Parameters("trans_amp").Value = random.uniform(0,2)
                    cloudArray[iteration].Parameters("edge_thresh").Value = random.uniform(-2,2)
                for iteration in range(0,2):
                    changeRangeArray[iteration].Parameters("newrange_min").Value = random.uniform(-0.05,-0.02)
                    changeRangeArray[iteration].Parameters("newrange_max").Value = random.uniform(0.02,0.05)
            
            # freeze and save into the shader dictionary
            Application.DeselectAll()
            Application.FreezeObj(activeObj.Name)
            shader_dict[str(i)] = oMaterial
            i += 1
            oProgressBar.Increment()
            
        # ROOF SHADERS ########################################################################################################

        oProgressBar.Value = 0
        oProgressBar.Maximum = shaderRoof
        oProgressBar.Step = 1
        oProgressBar.CancelEnabled = True
        oProgressBar.Caption = "Creating Roof's Shader."
        oProgressBar.Visible = True
        i=0
        shader_dict2 = {}
        while i < shaderRoof:
            #lets create a number or random color fractal shaders for the roofs
            oProgressBar.Caption = "Creating Roof's Shader " + str(i)
            if oProgressBar.CancelPressed:
                break
            darkR = random.uniform(0.25,0.5)
            if(random.randint(0,1)):
                darkG = darkR + random.uniform(0,0.1)
            else:
                darkG = darkR - random.uniform(0,0.1)
            if(random.randint(0,1)):
                darkB = darkR + random.uniform(0,0.1)
            else:
                darkB = darkR - random.uniform(0,0.1)
            lightR = random.uniform(0.25,0.7)
            if(random.randint(0,1)):
                lightG = lightR + random.uniform(0,0.01)
            else:
                lightG = lightR - random.uniform(0,0.01)
            if(random.randint(0,1)):
                lightB = lightR + random.uniform(0,0.01)
            else:
                lightB = lightR - random.uniform(0,0.01)
            oMaterial = activeObj.AddMaterial( "Phong", False, "Roof_Shader" );
            oPhong= activeObj.Material.Shaders(0)
            # ambiant to black
            oPhong.Parameters("ambient").Parameters("red").Value = 0
            oPhong.Parameters("ambient").Parameters("green").Value = 0
            oPhong.Parameters("ambient").Parameters("blue").Value = 0
            # random specular
            oPhong.Parameters("specular").Parameters("red").Value = random.uniform(0.1,0.4)
            oPhong.Parameters("specular").Parameters("green").Value = random.uniform(0.1,0.4)
            oPhong.Parameters("specular").Parameters("blue").Value = random.uniform(0.1,0.4)
            oMix = Application.CreateShaderFromProgID("Softimage.sib_color_2mix.1.0", "Sources.Materials.DefaultLib."+oMaterial.Name, "Mix_2_Colors")

            Application.SIConnectShaderToCnxPoint("Sources.Materials.DefaultLib." + oMaterial.Name + "." + oMix.Name + ".out", "Sources.Materials.DefaultLib." + oMaterial.Name + "." + oPhong.Name + ".diffuse", False)
            oMix.Parameters("mode1").Value = 4 # nultiply mode

            # fractal shader
            oFractal = Application.CreateShaderFromProgID("Softimage.txt3d-fractal_v3.1.0", "Sources.Materials.DefaultLib." + oMaterial.Name, "Fractal")
            Application.SIConnectShaderToCnxPoint("Sources.Materials.DefaultLib." + oMaterial.Name + "." + oFractal.Name, "Sources.Materials.DefaultLib." + oMaterial.Name + "." + oMix.Name + ".base_color", False)
            # color of the line with a fractal
            oFractal.Parameters("threshold").Value = random.uniform(0,1)
            oFractal.Parameters("color1").Parameters("red").Value = lightR
            oFractal.Parameters("color1").Parameters("green").Value = lightG
            oFractal.Parameters("color1").Parameters("blue").Value = lightB
            oFractal.Parameters("color2").Parameters("red").Value = darkR
            oFractal.Parameters("color2").Parameters("green").Value = darkG
            oFractal.Parameters("color2").Parameters("blue").Value = darkB
            # flagstone shader
            oFlagstone = Application.CreateShaderFromProgID("Softimage.txt3d-flagstone_v3.1.0", "Sources.Materials.DefaultLib." + oMaterial.Name, "Flagstone")
            Application.SIConnectShaderToCnxPoint("Sources.Materials.DefaultLib." + oMaterial.Name + "." + oFlagstone.Name + ".out", "Sources.Materials.DefaultLib." + oMaterial.Name + "." + oMix.Name + ".color1", 0)
            oFlagstone.Parameters("mortar_width").Value = random.uniform(0.2,0.4)
            # freeze and save into the shader dictionary2
            Application.FreezeObj(activeObj.Name)
            shader_dict2[str(i)] = oMaterial
            i += 1
            oProgressBar.Increment()
            
        Application.DeleteObj(activeObj.Name+"")
        Application.DeselectAll()
        return (shader_dict,shader_dict2)
