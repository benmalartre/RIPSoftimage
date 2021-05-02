#include <xsi_status.h>			// CStatus
#include <xsi_application.h>	// Application
#include <xsi_pluginregistrar.h>
#include <xsi_customproperty.h>
#include <xsi_ppglayout.h>
#include <xsi_ppgeventcontext.h>
#include <xsi_selection.h>
#include <xsi_menu.h>
#include <xsi_model.h>
#include <xsi_sceneitem.h>
#include <xsi_clip.h>
#include <xsi_shader.h>
#include <xsi_material.h>
#include <xsi_actionsource.h>
using namespace XSI;


XSIPLUGINCALLBACK CStatus UsdPrimitive_Define( const CRef & in_Ctx )
{
	// Here is where we add all the parameters to the 
	// Custom Property.  This will be called each time 
	// an new instance of the Custom Property is called.
	// It is not called when an persisted Custom Property is loaded.

	Application app ;
	CustomProperty prop = Context(in_Ctx).GetSource() ;
	Parameter param ;

	// Default capabilities for most of these parameters
	int caps = siPersistable | siAnimatable ;
	CValue dft ;	// Used for arguments we don't want to set

	//
	// Parameters on the "Numbers" tab
	// (For the most part the order of parameters does not matter.
	// In this case the order matches the layout order just for
	// readability)  See the color control section below for the
	// only case where order does matter.
	//

	prop.AddParameter(	L"NoSliderDemo", CValue::siDouble, caps, 
						L"", L"", 
						1.05, 0.0, 100.0, 0.0, 100.0, param ) ;
	prop.AddParameter(	L"ThumbWheelDemo", CValue::siFloat, caps, 
						L"", L"", 
						99.9, -1000.0, 1000.0, dft, dft, param ) ;

	// No UI Range is set - the range will be used
	prop.AddParameter(	L"NoLabelDemo",CValue::siInt1, caps, 
						L"", L"", 
						(LONG)15, (LONG)10, (LONG)20, dft, dft, param ) ;	

	prop.AddParameter(	L"NoLabelNotAnimatable",CValue::siInt2, siPersistable, 
						L"", L"", 
						(LONG)230, (LONG)200, (LONG)300, dft, dft, param ) ;	
	prop.AddParameter(	L"ValueOnlyDemo",CValue::siInt2, caps, 
						L"", L"", 
						(LONG)215, (LONG)200, (LONG)300, dft, dft, param ) ;	
	prop.AddParameter(	L"NormalSlider",CValue::siDouble, caps, 
						L"", L"", 
						50.0, 0.0, 100.0, 0.0, 100.0, param ) ;	
	prop.AddParameter(	L"LogarithmicSliderDemo",CValue::siDouble, caps, 
						L"Logarithmic Slider Demo", L"",		// You can specify a user-readable name hear, 
																// or specify label in the layout
						50.0, 0.0, 100.0, 0.0, 100.0, param ) ;	


	//
	// Parameters on the "File Controls" tab
	// As far as the Custom Property is concerned these 
	// are just normal string parameters but the
	// layout turns them into more powerful controls.
	// 

	prop.AddParameter(	L"ExportFilename",CValue::siString, siPersistable, 
						L"", L"", dft, param ) ;	
	prop.AddParameter(	L"ImportFilename",CValue::siString, siPersistable, 
						L"Import Filename", L"", dft, param ) ;
	prop.AddParameter(	L"ImportImage",CValue::siString, siPersistable, 
						L"", L"", dft, param ) ;
	prop.AddParameter(	L"FolderPicker",CValue::siString, siPersistable, 
						L"", L"", dft, param ) ;

	//
	// Parameters on the "Text" tab
	//

	// As far as the Custom Property is concerned, Multiline edits are just
	// normal strings
	prop.AddParameter(	L"MultiLine",CValue::siString, siPersistable, 
						L"", L"", dft, param ) ;
	prop.AddParameter(	L"EnableEdit", CValue::siBool, siPersistable, 
						L"", L"",	
						dft, param ) ;	
	prop.AddParameter(	L"CurrentPass",CValue::siString, siPersistable | siReadOnly, 
						L"", L"", dft, param ) ;	


	//
	// Parameters on the "Colors" tab
	//


	// There are 4 parameters that make up an RGBA color.
	// These must be defined IN ORDER.

	prop.AddParameter(	L"MyRGBAColorR",CValue::siDouble, caps, 
						L"MyRGBAColor", L"",					// This will be the label
						0.0, -10000.0, 10000.0, 0.0, 1.0, param ) ;	


	prop.AddParameter(	L"MyRGBAColorG",CValue::siDouble, caps, 
						L"", L"",	
						0.0, 																		
						-10000.0,// The range is -1000 to 1000 rather 
						10000.0, // than 0 to 1 because sometimes  								
						0.0,     // it is desirable to allow colors outside the normal range of  
						1.0,    // the color space.  However UI Range should always be 0-1.
						param ) ;	
	prop.AddParameter(	L"MyRGBAColorB",CValue::siDouble, caps, 
						L"", L"",	
						0.0, -10000.0, 10000.0, 0.0, 1.0, param ) ;	
	prop.AddParameter(	L"MyRGBAColorA",CValue::siDouble, caps, 
						L"", L"",	
						0.0, -10000.0, 10000.0, 0.0, 1.0, param ) ;	

	// This grouping of 3 parameters defines an RGB color
	prop.AddParameter(	L"MyRGBColorR",CValue::siDouble, caps, 
						L"MyRGBColor",	// The name of the first parameter appears as the label
						L"",	
						0.4, -10000.0, 10000.0, 0.0, 1.0, param ) ;	
	prop.AddParameter(	L"MyRGBColorG",CValue::siDouble, caps, 
						L"", L"",	
						0.3, -10000.0, 10000.0, 0.0, 1.0, param ) ;	
	prop.AddParameter(	L"MyRGBColorB",CValue::siDouble, caps, 
						L"", L"",	
						0.3, -10000.0, 10000.0, 0.0, 1.0, param ) ;	

	//
	// Parameters on the "Options" tab
	//

	prop.AddParameter(	L"ComboDemo",CValue::siInt1, caps, 
						L"", L"", 
						dft, (LONG)5, (LONG)10, dft, dft, param ) ;	
	prop.AddParameter(	L"DynamicCombo",CValue::siString, siPersistable, 
						L"", L"", dft, param ) ;	
	prop.AddParameter(	L"RadioBtnDemo",CValue::siInt4, caps, 
						L"Radio Button", L"",	
						(LONG)11, (LONG)0, (LONG)222, dft, dft, param ) ;	
	prop.AddParameter(	L"BitMaskDemo",CValue::siInt4, siPersistable, 
						L"", L"",	
						(LONG)1, (LONG)0, (LONG)64, dft, dft, param ) ;	
	prop.AddParameter(	L"ListBoxDemo",CValue::siInt4, caps, 
						L"", L"",	
						dft, dft, dft, dft, dft, param ) ;

	// Readonly but the value will be set by Logic code based on
	// whether there is a selection or not when the UI is shown
	prop.AddParameter(	L"Selection",CValue::siBool, siPersistable | siReadOnly, 
						L"Selection?", L"",	
						dft, 
						(LONG)0,  // Specifying range is optional for booleans and
						(LONG)-1, // a bit bizarre when done correctly
							 // because vbscript considers true to be
							// the value -1
						dft, dft, param ) ;	
	//
	// Parameters on the "Check Boxes" tab
	//

	prop.AddParameter(	L"Check1", CValue::siBool, siPersistable, 
						L"1", L"",	
						dft,		// No need to specify range of a boolean
						param ) ;	
	prop.AddParameter(	L"Check2", CValue::siBool, siPersistable, 
						L"2", L"",	
						dft, param ) ;	
	prop.AddParameter(	L"CheckA", CValue::siBool, siPersistable, 
						L"A", L"",	
						dft, param ) ;	
	prop.AddParameter(	L"CheckB", CValue::siBool, siPersistable, 
						L"B", L"",	
						(LONG)-1,		// On by default
						param ) ;	

	// A normal boolean parameter but some tricks in the layout
	// will make its appearance completely different
	prop.AddParameter(	L"CheckButton", CValue::siBool, siPersistable, 
						L"", L"",	
						dft, param ) ;	

	//
	// Parameters on the "Images" tab
	//
	prop.AddParameter(	L"ImageClipControl", CValue::siString, siPersistable, 
						L"", L"",	
						CString( L"Clips.noIcon_pic" ),
						param ) ;	
	prop.AddParameter(	L"ImageClipControlNoClip", CValue::siString, siPersistable, 
						L"", L"",	
						CString( L"Clips.noIcon_pic" ),
						param ) ;	
	prop.AddParameter(	L"StaticBitmapControl", CValue::siString, siPersistable, 
						L"", L"",	
						dft,
						param ) ;	
	return CStatus::OK;	
}

XSIPLUGINCALLBACK CStatus UsdPrimitive_DefineLayout( const CRef & in_Ctx )
{
	// Softimage will call this to define the visual appearance of the CustomProperty
	// The layout is shared between all instances of the CustomProperty
	// and is cached.  You can force the code to re-execute by using the 
	// XSIUtils.Refresh feature.
	
	Application app;
	PPGLayout oLayout = Context( in_Ctx ).GetSource() ;
	PPGItem item ;

	oLayout.Clear() ;

	// The code indentation matches the nesting inside the layout.
	oLayout.AddTab( L"Numbers" ) ;
		oLayout.AddGroup() ;
			oLayout.AddRow() ;
				
				// To save space turn off the slider and just show an edit control
				item = oLayout.AddItem( L"NoSliderDemo" ) ;
				item.PutAttribute( siUINoSlider, true ) ;
	
				// A potentially better way to save space
				item = oLayout.AddItem( L"ThumbwheelDemo", L"Thumbwheel" ) ;
				item.PutAttribute( siUIThumbWheel, true ) ;

			oLayout.EndRow() ;
		oLayout.EndGroup() ;


		oLayout.AddGroup( L"NoLabel and ValueOnly" ) ;
			// Shows how to hide the divot or label
			item = oLayout.AddItem( L"NoLabelDemo" ) ;
			item.PutAttribute( siUINoLabel, true ) ;

			item = oLayout.AddItem( L"NoLabelNotAnimatable" ) ;
			item.PutAttribute( siUINoLabel, true ) ;

			item = oLayout.AddItem( L"ValueOnlyDemo" ) ;
			item.PutAttribute( siUIValueOnly, true ) ;
		oLayout.EndGroup() ;


		item = oLayout.AddItem( L"NormalSlider" ) ;
		item.PutAttribute( siUIDecimals, (LONG)1 ) ;

		item = oLayout.AddItem( L"LogarithmicSliderDemo" ) ;
		item.PutAttribute( siUILogarithmic, true ) ;
		item.PutAttribute( siUIDecimals, (LONG)5 ) ;
		item.PutAttribute( siUISyncSlider, true ) ;	// Already true by default
		item.PutAttribute( siUINoSlider, false ) ;	// Already false by default

	oLayout.AddTab( L"File Controls" ) ;
		
		item = oLayout.AddItem( L"ExportFilename", L"Export Filename", siControlFilePath ) ;
		// Export is the default option, so this control will give an "overwrite?" warning if
		// the file already exists.
		// Other attributes exist that are not demonstrated,
		// See the siPPGItemAttribute enum documentation for more info

		// If the parameter value is empty that the browser will open in the Models
		// subdirectory of the current project
		item.PutAttribute( siUIInitialDir, L"project" ) ;
		item.PutAttribute( siUISubFolder, L"Models" ) ;

		// This decides what file extentions to display
		item.PutAttribute( siUIFileFilter, L"Text files (*.txt *.text)|*.txt:*.text|All Files (*.*)|*.*||" ) ;

		item = oLayout.AddItem( L"ImportFilename", L"Import Filename", siControlFilePath ) ;
		item.PutAttribute( siUIFileMustExist, true ) ;
		item.PutAttribute( siUIOpenFile, true ) ;

		// This is a specific default directory
#ifndef unix 
		item.PutAttribute( siUIInitialDir, L"c:\\temp" ) ;
#else
		item.PutAttribute( siUIInitialDir, L"/var/tmp" ) ;
#endif

		item.PutAttribute( siUIFileFilter, L"xsiaddon files (*.xsiaddon)|*.xsiaddon|spdl files (*.spdl)|*.spdl|All Files (*.*)|*.*||" ) ;

		item = oLayout.AddItem( L"ImportImage", L"Import Image", siControlFilePath ) ;
		// We specify siUIFileMustExist and siUIOpenFile to tell Softimage that this is an import 
		// file path and hence we expect an existing file is selected
		item.PutAttribute( siUIFileMustExist, true ) ;
		item.PutAttribute( siUIOpenFile, true ) ;
		// Rather than a huge LONG, hardcoded file filter just specify this to get all supported image formats
		item.PutAttribute( siUIImageFile, true ) ;

		// Rather than picking a file, this control lets you pick a directory
		item = oLayout.AddItem( L"FolderPicker", L"Pick Folder", siControlFolder ) ;
		item.PutAttribute( siUIInitialDir, L"user" ) ;
		item.PutAttribute( siUIInitialDir, L"Application/spdl" ) ;

	oLayout.AddTab( L"Text" ) ;

		oLayout.AddGroup( L"Multi-Line Edit" ) ;
			//Make it 180 pixels tall
			oLayout.AddString( L"MultiLine", L"", true, 180 ) ;

			//Check box to control whether the multi-line control
			//is enabled or not.  See the logic code below.
			oLayout.AddItem( L"EnableEdit", L"Enable Edit" ) ;
		oLayout.EndGroup() ;

		oLayout.AddGroup( L"Current Pass Info" ) ;
			// Use a row to put both the edit and the button on the same line
			oLayout.AddRow() ;
				// The value of this disabled control will be filled 
				// in by the logic when the button is pressed
				item = oLayout.AddItem( L"CurrentPass" ) ;

				// Specify a larger value so that the button doesn't take
				// an unfair share of the width
				item.PutWidthPercentage( 80 ) ;

				item = oLayout.AddButton( L"MyButton", L"Find pass" ) ;
				item.PutWidthPercentage( 20 ) ;
			oLayout.EndRow() ;
		oLayout.EndGroup() ;

	oLayout.AddTab( L"Colors" ) ;

		oLayout.AddStaticText( L"Each color channel is a separate parameter:" ) ;

		// Note how we only mention the first parameter of our
		// group of 4 parameters that make up the color
		oLayout.AddColor( L"MyRGBAColorR", L"MyRGBAColor", true ) ;
		oLayout.AddColor( L"MyRGBColorR", L"MyRGBColor", false ) ;

	oLayout.AddTab( L"Options" ) ;
		oLayout.AddGroup() ;
			// The layout code to deal with Combo Boxes, Radios, Bitmasks
		    // is practically identical

			CValueArray comboItems( 6 ) ;
			comboItems[0] = L"Item 1"; comboItems[1] = (LONG)5 ;
			comboItems[2] = L"Item 2"; comboItems[3] = (LONG)6 ;
			comboItems[4] = L"Item 3"; comboItems[5] = (LONG)8 ;

			oLayout.AddEnumControl( L"ComboDemo", comboItems, L"", siControlCombo ) ;
		
			CValueArray InitialDynamicComboItems( 2 ) ;
			InitialDynamicComboItems[0] = L"Default"; InitialDynamicComboItems[1] = L"Default" ;

			oLayout.AddEnumControl( L"DynamicCombo", InitialDynamicComboItems, L"", siControlCombo ) ;

			// NOTE: Previous versions had a limitation where 
			// the values had to match the index (0,1,2...) This is now fixed.
			CValueArray radioItems( 6 ) ;
			radioItems[0] = L"Val 0"; radioItems[1] = (LONG)0 ;
			radioItems[2] = L"Val 11"; radioItems[3] = (LONG)11 ;	// (Default)
			radioItems[4] = L"Val 222"; radioItems[5] = (LONG)222;

			oLayout.AddEnumControl( L"RadioBtnDemo", radioItems, L"", siControlRadio ) ;

			CValueArray listItems( 6 ) ;
			listItems[0] = L"Item 1"; listItems[1] = (LONG)1 ;
			listItems[2] = L"Item 2"; listItems[3] = (LONG)2 ;
			listItems[4] = L"Item 3"; listItems[5] = (LONG)3 ;

			item = oLayout.AddEnumControl( L"ListBoxDemo", listItems, L"", siControlListBox ) ;
			item.PutAttribute( siUICY, (LONG)80 ) ;
			item.PutAttribute( siUIStyle, (LONG)0x0101 ) ;
		oLayout.EndGroup() ;

		oLayout.AddGroup( L"Bitmask Demo" ) ;
			oLayout.AddRow() ;
				CValueArray bitmaskItems( 6 ) ;
				bitmaskItems[0] = L"Flag 1"; bitmaskItems[1] = (LONG)1 ;
				bitmaskItems[2] = L"Flag 4"; bitmaskItems[3] = (LONG)4 ;
				bitmaskItems[4] = L"Flag 16"; bitmaskItems[5] = (LONG)16 ;
				
				item = oLayout.AddEnumControl( L"BitMaskDemo", bitmaskItems, L"", siControlCheck ) ;
				item.PutAttribute( siUINoLabel, true ) ;

				// Show this explaination text next to the control
				// Static text can be split into multiple lines by using \r\n
				// (If necessary you can tweak the size of the static text,
				// by setting siUICX, and siUICY)
				oLayout.AddStaticText( CString( L"Bitfields are an efficient way\r\n" 
						L"to store many boolean\r\n" 
						L"values in a single integer" ) ) ;

			oLayout.EndRow() ;
		oLayout.EndGroup() ;

		oLayout.AddSpacer() ;

		oLayout.AddGroup( L"Spdl logic controlled" ) ;
			oLayout.AddItem( L"Selection" ) ;
		oLayout.EndGroup() ;

	oLayout.AddTab( L"Check Boxes" ) ;
	
		// Show how to put 2 groups side by side with
		// unequal distribution

		oLayout.AddRow() ;
			oLayout.AddGroup( L"Letter", true, 40 ) ;
				oLayout.AddItem( L"Check1" ) ;
				oLayout.AddItem( L"Check2" ) ;
			oLayout.EndGroup() ;


			oLayout.AddGroup( L"Number", true, 60 ) ;
				oLayout.AddItem( L"CheckA" ) ;
				oLayout.AddItem( L"CheckB" ) ;
			oLayout.EndGroup() ;
	
		oLayout.EndRow() ;
		
		// This is a useful trick for saving space
		// It is a boolean parameter but
		// it displays itself as a button, so there is
		// no space wasted by a label
		item = oLayout.AddItem( L"CheckButton", L"", L"dscontrol" ) ;
		item.PutAttribute( L"class", L"button" ) ;
		item.PutAttribute( siUICX, (LONG)120 ) ;
		item.PutAttribute( siUICaption, L"Invert" ) ;
		// The style is based on these Windows style codes
		// #define BS_AUTOCHECKBOX     0x00000003L
		// #define BS_PUSHLIKE         0x00001000L
		item.PutAttribute( siUIStyle, (LONG)0x00001003 ) ;
		item.PutAttribute( siUINoLabel, true ) ;


	oLayout.AddTab( L"Images" ) ;

		oLayout.AddGroup( L"Image Clips Demo" ) ;
			item = oLayout.AddItem( L"ImageClipControl", L"Image Clip 1", siControlImageClip ) ;
			item = oLayout.AddItem( L"ImageClipControlNoClip", L"Image Clip 2", siControlImageClip ) ;
				item.PutAttribute( siUIShowClip, false ) ;
		oLayout.EndGroup() ;
		
		oLayout.AddGroup( L"Static Bitmap Demo" ) ;
			item = oLayout.AddItem( L"StaticBitmapControl", L"Static Bitmap", siControlBitmap ) ;
				item.PutAttribute( siUIFilePath, app.GetInstallationPath( siFactoryPath ) + L"/XSISDK/examples/workgroup/Addons/PSetUIDemo/Data/Bitmaps/logo.bmp" ) ;
		oLayout.EndGroup() ;

	return CStatus::OK;	
}

XSIPLUGINCALLBACK CStatus UsdPrimitive_PPGEvent( const CRef& io_Ctx )
{
	// This callback is called when events happen in the user interface
	// This is where you implement the "logic" code.

	// This code only processes the events that it is interested in,
	// other events are ignored
	
	// If the value of a parameter changes but the UI is not shown then this
	// code will not execute.  Also this code is not re-entrant, so any changes
	// to parameters inside this code will not result in further calls to this function
	
	Application app ;

	// The context object is used to determine exactly what happened
	// We don't use the same "PPG" object that is used from Script-based logic code 
	// but through the C++ API we can achieve exactly the same functionality.
	PPGEventContext ctx( io_Ctx ) ;

	PPGEventContext::PPGEvent eventID = ctx.GetEventID() ;

	if ( eventID == PPGEventContext::siOnInit )
	{
		// This event meant that the UI was just created.
		// It gives us a chance to set some parameter values.
		// We could even change the layout completely at this point.

		// For this event Source() of the event is the CustomProperty object

		CustomProperty prop = ctx.GetSource() ;	
		prop.PutParameterValue( L"Selection", app.GetSelection().GetCount() > 0 ) ; 

		app.LogMessage( L"Inspected object " + prop.GetName() + L" has " + 
						CValue( prop.GetParameters().GetCount() ).GetAsText() + L" parameters" ) ;


		prop.PutParameterValue( L"MultiLine", CString( L"From C++ use \\r\\n\r\nto break a string into multiple lines." 
								L"\r\n\r\n\t\\t is useful too\r\n" 
								L"Use \\\" and \\\' to produce \" and \'" ));

		//Demonstrate how to use the PPGLayout to populate the items inside the ComboBox
		PPGLayout oPPGLayout = prop.GetPPGLayout();		
		PPGItem oPPGItem = oPPGLayout.GetItem(L"DynamicCombo");

		//Replacing the Default string that was originally defined at the DefineLayout Callback
		CValueArray newDynamicComboItem( 4 ) ;
			newDynamicComboItem[0] = L"StringLabel1"; newDynamicComboItem[1] = L"StringValue1" ;
			newDynamicComboItem[2] = L"StringLabel2"; newDynamicComboItem[3] = L"StringValue2" ;

		oPPGItem.PutUIItems(newDynamicComboItem);

		//Setting the value for the DynamicCombo to the second value "StringValue2"
		prop.GetParameter(L"DynamicCombo").PutValue(L"StringValue2");

		//Redraw the PPG to show the new combo items
		ctx.PutAttribute(L"Refresh",true);
	}
	else if ( eventID == PPGEventContext::siButtonClicked )
	{
		// If there are multiple buttons 
		// we can use this attribute to figure out which one was clicked.

		// NOTE: the CheckButton in this PPG is actually a parameter
		// and fires a siParameterChange event rather than 
		// a siButtonClicked event

		CValue buttonPressed = ctx.GetAttribute( L"Button" ) ;	

		if ( buttonPressed.GetAsText() == L"MyButton" )
		{
			// Invoke Softimage's CurrentPass command
			CValue currentPass ;
			CValueArray args(0) ;
			app.ExecuteCommand( L"GetCurrentPass", args, currentPass ) ;

			// GetCurrentPass returns a Pass object but because
			// we only need the name we can use this base class.
			SIObject pass = (CRef)currentPass ;  // use operator CRef()

			CString strCurrentPass = pass.GetName() ;
			app.LogMessage( L"Current render pass " + strCurrentPass ) ;

			CustomProperty prop = ctx.GetSource() ;	
			prop.PutParameterValue( L"CurrentPass", strCurrentPass ) ; 	
		}
	}
	else if ( eventID == PPGEventContext::siTabChange )
	{
		// We will be called when the PPG is first opened
		// and every time the tab changes

		// Retrieve the label of the tab that is now active
		CValue tabLabel = ctx.GetAttribute( L"Tab" ) ;

		if ( tabLabel == L"File Controls" )
		{
			app.LogMessage( L"File control tab is now visible" ) ;				
		}
	}
	else if ( eventID == PPGEventContext::siParameterChange )
	{
		// For this event the Source of the event is the parameter
		// itself
		Parameter changed = ctx.GetSource() ;	

		// But we have no trouble getting at the CustomProperty
		// that owns the changed parameter
		CustomProperty prop = changed.GetParent() ;	
		
		CString   paramName = changed.GetScriptName() ; 

		if ( paramName == L"NoSliderDemo" )
		{
			//You can provide custom reaction code when an parameter changes
			app.LogMessage( L"New value for NoSliderDemo parameter: " + changed.GetValue().GetAsText() ) ;
		}
		else if ( paramName == L"EnableEdit" )
		{
			Parameter multiLineParam = prop.GetParameters().GetItem( L"MultiLine" ) ;

			// We toggle the read-only flag based on the EnableEdit boolean
			multiLineParam.PutCapabilityFlag( siReadOnly, prop.GetParameterValue( L"EnableEdit" ) );
		}
		else if ( paramName == L"MyRGBColorG" )
		{
			// The logic for dealing with colors is basically the same
			// as any other parameter, we just handle each channel of the color
			// as a separate parameter.

			// In thi case we force the blue to follow the Green
			prop.PutParameterValue( L"MyRGBColorB", prop.GetParameterValue( L"MyRGBColorG" ) ) ;
		}
		else 
		{
			// this can be extended to add any other custom logic we want for parameters changing
		}
		
		app.LogMessage( L"PSetUIDemoCPP_PPGEvent called for ParameterChange: " 
						+ prop.GetName() + L" " + paramName ) ;
	}

	return CStatus::OK ;
}

/*
SICALLBACK XSI::CStatus PSetUIDemoCPP_Menu_Init( XSI::CRef& in_ref )
{
	Context ctxt = in_ref;
	Menu menu = ctxt.GetSource();

	// adds a calback menu item: the supplied callback function is called
	// by Softimage when the user selects the menu item.
	MenuItem item;
	CStatus st = menu.AddCallbackItem( L"PSetUIDemo - C++ API Version", L"OnPSetUIDemoCPPMenuClicked", item );

	return st ;
}

XSIPLUGINCALLBACK XSI::CStatus OnPSetUIDemoCPPMenuClicked( XSI::CRef& )
{	
	Application app;

	CValueArray addpropArgs(5) ;
	addpropArgs[0] = L"PSetUIDemoCPP"; // Type of Property
	addpropArgs[3] = L"PSetUIDemoCPP"; // Name for the Property

	if ( app.GetSelection().GetCount() == 0 )
	{		
		// No selection so create the object at the scene root
		addpropArgs[1] = L"Scene_Root";
	}

	CValue retVal ;
	CStatus st = app.ExecuteCommand( L"SIAddProp", addpropArgs, retVal ) ;

	if ( st.Succeeded() )
	{
		// Inspect the first of the newly created Properties
		//(there could be more than one if more than one object was
		//selected, so newObjects is actually a CValueArray)
		CValueArray vArray = retVal;
		CValue newObjects = vArray[0] ;

		CValueArray inspectobjArgs(5) ;	
		inspectobjArgs[0] = newObjects ; 

		app.ExecuteCommand( L"InspectObj", inspectobjArgs, retVal ) ;
		return CStatus::OK ;
	}
	else
	{
		// Wrong kind of object selected
		return CStatus::Fail;
	}

}
*/
