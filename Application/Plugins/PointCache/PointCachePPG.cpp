// PointCachePPG.Cpp
//------------------------------------
#include "PointCache.h"
#include <xsi_ppglayout.h>
#include <xsi_ppgitem.h>
#include <xsi_ppgeventcontext.h>
#include <xsi_userdatablob.h>

CStatus PointCacheOp_WriteBlob(UserDataBlob in_blob,CString in_filepath);
CStatus PointCacheOp_WriteFile(UserDataBlob in_blob,CString in_fullname);
void PointCachePPG_RebuildLayout(CustomProperty in_ppg);

void PointCachePPG_RebuildLayout(CustomOperator in_op)
{
	PPGLayout oLayout = in_op.GetPPGLayout();
	PPGItem oItem;

	oLayout.Clear();
	oLayout.AddTab(L"Main");
	oLayout.AddItem(L"Mute",L"Mute");
	oLayout.AddGroup(L"PointCache");
		oItem = oLayout.AddItem( L"CacheFile", L"Cache File", siControlFilePath);
		oItem.PutAttribute(siUIFileFilter,L"bmx files (*.bpc)|*.bpc:*.bpc|All Files (*.*)|*.*||") ;

	oLayout.EndGroup();

	oLayout.AddGroup(L"Time");
		oLayout.AddGroup(L"Cache Infos");
			oLayout.AddRow();
				oItem = oLayout.AddItem( L"CacheStartFrame", L"Start Frame");
				oItem.PutAttribute( siUINoSlider , true ); 
				oItem = oLayout.AddItem( L"CacheEndFrame", L"End Frame");
				oItem.PutAttribute( siUINoSlider , true ); 
			oLayout.EndRow();
		oLayout.EndGroup();
		oLayout.AddItem( L"UseCustomPlayback", L"Use Custom Playback");
		if(in_op.GetParameterValue(L"UseCustomPlayback")==false)
		{
			oLayout.AddItem( L"TimeOffset", L"Time Offset");
			oLayout.AddItem( L"TimeWarp", L"Time Warp");
		}
		else oLayout.AddItem( L"CustomFrame", L"Custom Frame");
	oLayout.EndGroup();

	oLayout.AddGroup(L"Interpolation");
		CValueArray InterpolationItems(6);
		InterpolationItems[0] = L"Linear";
		InterpolationItems[1] = 0;
		InterpolationItems[2] = L"Cubic";
		InterpolationItems[3] = 1;
		InterpolationItems[4] = L"Hermite";
		InterpolationItems[5] = 2;
		oLayout.AddEnumControl(L"Interpolation",InterpolationItems,L"Type", siControlCombo );
		oLayout.AddRow();
			oLayout.AddItem(L"HermiteTension",L"Tension");
			oLayout.AddItem(L"HermiteBias",L"Bias");
		oLayout.EndRow();
	oLayout.EndGroup();

	/*
	oLayout.AddTab(L"Extra");
	oLayout.AddGroup(L"Blob");
				oLayout.AddItem( L"ReadFromBlob", L"Read From Blob");
				oLayout.AddRow();
					CString CacheState = L"Blob state ---> ";

					if(in_op.GetParameterValue(L"BlobValid")==false)
					{
						CacheState += L"empty.";
					}
					else
					{
						CacheState += L"ok.";
					}
					oLayout.AddStaticText(CacheState,120,22);
					oLayout.AddSpacer(0,0);
					oLayout.AddButton(L"WriteBlobToFile",L"Write Blob To File");
				oLayout.EndRow();
			oLayout.EndGroup();
	*/
}

XSIPLUGINCALLBACK CStatus PointCacheOp_PPGEvent( const CRef& in_ctxt )
{
	Application app ;
	PPGEventContext ctxt( in_ctxt ) ;

	PPGEventContext::PPGEvent eventID = ctxt.GetEventID() ;

	if ( eventID == PPGEventContext::siOnInit )
    {
        CustomOperator oProp = ctxt.GetSource() ;
		PointCachePPG_RebuildLayout(oProp);
        ctxt.PutAttribute(L"Refresh",true);
    }

	else if ( eventID == PPGEventContext::siButtonClicked )
	{
		CValue buttonPressed = ctxt.GetAttribute( L"Button" ) ;	
		
/*
		if(buttonPressed.GetAsText() == L"WriteBlobToFile")
		{
			CustomOperator oOp = ctxt.GetSource();
			UserDataBlob oBlob = oOp.GetPortAt(0,1,0).GetTarget();
			X3DObject oMesh(oBlob.GetParent3DObject());
			if(PointCacheOp_WriteFile(oBlob,oMesh.GetFullName()) == CStatus::OK)
			{
				CString sProjectDir = Application().GetInstallationPath( siProjectPath );
				CString sCacheDir = CUtils::BuildPath(sProjectDir, L"EPCFiles");
				CString sCacheFile = CUtils::BuildPath(sCacheDir, oMesh.GetFullName()+L".epc");
				oOp.PutParameterValue(L"CacheFile",sCacheFile);
				oOp.PutParameterValue(L"ReadFromBlob",false);

				PointCachePPG_RebuildLayout(oOp);
				ctxt.PutAttribute(L"Refresh",true);
			}
		}
*/

	}

	else if ( eventID == PPGEventContext::siParameterChange )
    {
        Parameter changed = ctxt.GetSource() ;
        CustomOperator oOp = changed.GetParent() ;

		/*
        if ( changed.GetName() == L"ReadFromBlob" )
        {
			if(changed.GetValue() == true)//Write File To Blob
			{
				UserDataBlob oBlob = oOp.GetPortAt(0,1,0).GetTarget();
				CString oCache(oOp.GetParameterValue(L"CacheFile"));
				if(PointCacheOp_WriteBlob(oBlob,oCache) == CStatus::OK)
				{
					oOp.PutParameterValue(L"BlobValid",true);
					oOp.PutParameterValue(L"ReadFromBlob",true);
					CString emptyPath;
					oOp.PutParameterValue(L"CacheFile",emptyPath);
					PointCachePPG_RebuildLayout(oOp);
					ctxt.PutAttribute(L"Refresh",true);
				}
			}
			else
			{
				UserDataBlob oBlob = oOp.GetPortAt(0,1,0).GetTarget();
				oBlob.Clear();
				oOp.PutParameterValue(L"ReadFromBlob",false);
				oOp.PutParameterValue(L"BlobValid",false);
				PointCachePPG_RebuildLayout(oOp);
				ctxt.PutAttribute(L"Refresh",true);
			}
        }
		*/

		if ( changed.GetName() == L"UseCustomPlayback" )
		{
			PointCachePPG_RebuildLayout(oOp);
			ctxt.PutAttribute(L"Refresh",true);
		}
	}

	return CStatus::OK ;
}
/*
CStatus PointCacheOp_WriteBlob(UserDataBlob in_blob,CString in_filepath)
{
	Application().LogMessage(L"Write Cache To Blob...");

	fstream fs;	
	const char* filepath = in_filepath.GetAsciiString() ;

	struct stat results;
	if (stat(filepath, &results) == 0)
	{
		fs.open(filepath,ios::binary|ios::in);
		fs.seekg(0,ios::beg);

		long r_nb;
		long r_sf;
		long r_ef;

		fs.read((char*) &r_nb, sizeof(long));
		fs.read((char*) &r_sf, sizeof(long));
		fs.read((char*) &r_ef, sizeof(long));

		long r_t =  r_ef - r_sf +1;
		long r_l = 3 + 3*r_nb*r_t;

		float* datas = new float[r_l];

		datas[0] = (float)r_nb;
		datas[1] = (float)r_sf;
		datas[2] = (float)r_ef;

		float* r_p = new float[3*r_nb*r_t];
		fs.read((char*) r_p, 3*r_nb*r_t*sizeof(float));

		for(long a=0;a<r_nb*r_t;a++)
		{
			datas[a*3+3] = r_p[3*a];
			datas[a*3+4] = r_p[3*a+1];
			datas[a*3+5] = r_p[3*a+2];
		}
		
		in_blob.PutValue( (const unsigned char*)datas, r_l*sizeof(float)) ;
		fs.close();
		delete[]datas;
		delete[]r_p;

		Application().LogMessage(L"Done...");

		return CStatus::OK;
	}

	else
	{
		Application().LogMessage(L"Failed : Connect a cache file first...");
		return CStatus::Fail;
	}
	 
}

CStatus PointCacheOp_WriteFile(UserDataBlob in_blob,CString in_fullname)
{
	Application().LogMessage(L"Write Blob To File...");
	CString sProjectDir = Application().GetInstallationPath( siProjectPath );
	CString sCacheDir = CUtils::BuildPath(sProjectDir, L"BMXFiles");

	if(!in_blob.IsEmpty())
	{
		const unsigned char * pBuffer = NULL ;
		unsigned int cntBuffer = 0 ;

		in_blob.GetValue( pBuffer, cntBuffer ) ;
		float* blobdatas = (float*)pBuffer;

		if(b_CreateFolder(sCacheDir.GetAsciiString()) == true)
		{
			Application().LogMessage(L"Folder ---> "+ CUtils::BuildPath(sProjectDir,L"BMXFiles")+L" created...");
		}

		CString filepathstr = CUtils::BuildPath(sCacheDir, in_fullname +L".bmx");
		const char* filepath = filepathstr.GetAsciiString() ;

		fstream out_file(filepath,ios::out|ios::binary|ios::trunc); 
			
		long nb = (long)blobdatas[0];
		long sf = (long)blobdatas[1];
		long ef = (long)blobdatas[2];

		out_file.write((char*) &nb, sizeof(long));
		out_file.write((char*) &sf, sizeof(long));
		out_file.write((char*) &ef, sizeof(long));
		long t = ef - sf + 1;

		out_file.write((char*) &blobdatas[3],t*nb*3* sizeof(float));
		out_file.close();
		in_blob.Clear();

		Application().LogMessage(L"Done...");
		return CStatus::OK;
	}

	else
	{
		Application().LogMessage(L"Blob Empty!! ---> Operation aborted for "+ in_fullname);
		return CStatus::Fail;
	}
}
*/
