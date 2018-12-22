#include "IRDeltaMush.h"

void GetGeometryPointData ( CDoubleArray &in_GeoPointPosition, ULONG &in_PolyCount, CLongArray	&in_PolyPointCount, CLongArray &in_PolyPointIndice,  std::vector<DMPoint_t> *io_ArrayPoint )
{
	//Loop over polygon to get the geometry points position and points neigbor index. 
	ULONG nPolyOffset = 0;
	//Loop over polygon.
	for ( ULONG iPoly=0 ; iPoly<in_PolyCount ; iPoly++ )
	{
		LONG	currentPointNumber = in_PolyPointCount[iPoly];
		//Loop over Points per polygon.
		for ( ULONG iPnt=0 ; iPnt<(ULONG)currentPointNumber ; iPnt++, nPolyOffset++ )
		{
			//Get the current Point index.
			LONG	currentPointIndex = in_PolyPointIndice[nPolyOffset];

			//Get the neigbor edges point index.
			NeigborsEdgePointIndex	currentNeigborIndex;
			if ( iPnt == 0 )
			{
				currentNeigborIndex.p1 = in_PolyPointIndice[nPolyOffset+1];
				currentNeigborIndex.p2 = in_PolyPointIndice[nPolyOffset+(currentPointNumber-1)];
			}
			else if ( iPnt == in_PolyPointCount[iPoly]-1 )
			{
				currentNeigborIndex.p1 = in_PolyPointIndice[nPolyOffset-(currentPointNumber-1)];
				currentNeigborIndex.p2 = in_PolyPointIndice[nPolyOffset-1];
			}
			else
			{
				currentNeigborIndex.p1 = in_PolyPointIndice[nPolyOffset+1];
				currentNeigborIndex.p2 = in_PolyPointIndice[nPolyOffset-1];
			}

			//Update array Point Data
			io_ArrayPoint->at(currentPointIndex).position.Set ( (float)in_GeoPointPosition[currentPointIndex*3],
																(float)in_GeoPointPosition[currentPointIndex*3+1],
																(float)in_GeoPointPosition[currentPointIndex*3+2] );

			io_ArrayPoint->at(currentPointIndex).smoothPosition = io_ArrayPoint->at(currentPointIndex).position;

			io_ArrayPoint->at(currentPointIndex).arrayNeigborsEdgePointIndex.push_back ( currentNeigborIndex );

			io_ArrayPoint->at(currentPointIndex).switchResult = false;
		}
	}
};

void UpdateGeometryPointData(CDoubleArray &in_GeoPointPosition, std::vector<DMPoint_t>& io_ArrayPoint)
{
	//Loop over polygon to get the geometry points position
	ULONG currentPointIndex = 0;

	//Loop over points
	for (ULONG iPnt = 0; iPnt<io_ArrayPoint.size(); iPnt++)
	{
		//Update array Point Data
		io_ArrayPoint.at(iPnt).position.Set(
			(float)in_GeoPointPosition[iPnt * 3],
			(float)in_GeoPointPosition[iPnt * 3 + 1],
			(float)in_GeoPointPosition[iPnt * 3 + 2]);

		io_ArrayPoint.at(iPnt).smoothPosition = io_ArrayPoint.at(iPnt).position;
		io_ArrayPoint.at(iPnt).switchResult = false;
	}
};

void SmoothPointPositionData ( LONG in_SmoothIteration, std::vector<DMPoint_t> *io_ArrayPoint )
{
	LONG	whileCounter = 0;
	LONG	p1Index;// p2Index ;
	ULONG	iPnt, iNei, neigborDataCount;

	while ( whileCounter < in_SmoothIteration )
	{
		//Loop over the point data for compute smoothing.
		for ( iPnt=0 ; iPnt<io_ArrayPoint->size() ; iPnt++ )
		{
			io_ArrayPoint->at(iPnt).smoothCache.SetNull();
			neigborDataCount = (ULONG)io_ArrayPoint->at(iPnt).arrayNeigborsEdgePointIndex.size();
			//Loop over the current point neigbor data.
			for ( iNei=0 ; iNei<neigborDataCount ; iNei++ )
			{
				p1Index = io_ArrayPoint->at(iPnt).arrayNeigborsEdgePointIndex.at(iNei).p1 ;
				//p2Index = io_ArrayPoint->at(iPnt).arrayNeigborsEdgePointIndex.at(iNei).p2 ;
				io_ArrayPoint->at(iPnt).smoothCache.AddInPlace ( io_ArrayPoint->at(p1Index).smoothPosition );
			}

			io_ArrayPoint->at(iPnt).smoothCache.ScaleInPlace ( 1.0f / (float)neigborDataCount );
		}

		//Loop over the point for update the smoothposition.
		for ( iPnt=0 ; iPnt<io_ArrayPoint->size() ; iPnt++ )
		{
			io_ArrayPoint->at(iPnt).smoothPosition = io_ArrayPoint->at(iPnt).smoothCache ;
		}

		whileCounter++;
	}
};

void ComputePointReferenceFrame ( std::vector<DMPoint_t> *io_ArrayPoint, ULONG &in_PointIndex )
{
	XSI::MATH::CVector3f	P0, P1, P2, vP0P1, vP0P2, vNormal, offsetVector;

	//Compute Point Normal.
	P0 = io_ArrayPoint->at(in_PointIndex).smoothPosition;

	for ( ULONG iNei=0 ; iNei<io_ArrayPoint->at(in_PointIndex).arrayNeigborsEdgePointIndex.size() ; iNei++ )
	{
		P1 = io_ArrayPoint->at(io_ArrayPoint->at(in_PointIndex).arrayNeigborsEdgePointIndex.at(iNei).p1).smoothPosition ;
		P2 = io_ArrayPoint->at(io_ArrayPoint->at(in_PointIndex).arrayNeigborsEdgePointIndex.at(iNei).p2).smoothPosition ;

		vP0P1.Sub ( P1, P0 );
		vP0P2.Sub ( P2, P0 );

		vP0P1.NormalizeInPlace();
		vP0P2.NormalizeInPlace();

		vNormal.Cross ( vP0P1, vP0P2 );
		vNormal.NormalizeInPlace();

		io_ArrayPoint->at(in_PointIndex).normal.AddInPlace ( vNormal );
	}

	io_ArrayPoint->at(in_PointIndex).normal.ScaleInPlace ( 1.0f / (float)io_ArrayPoint->at(in_PointIndex).arrayNeigborsEdgePointIndex.size() );
	io_ArrayPoint->at(in_PointIndex).normal.NormalizeInPlace();

	//Compute Tangent and BiNormal.
	io_ArrayPoint->at(in_PointIndex).tangent.Sub ( io_ArrayPoint->at(in_PointIndex).smoothPosition, io_ArrayPoint->at(io_ArrayPoint->at(in_PointIndex).arrayNeigborsEdgePointIndex.at(io_ArrayPoint->at(in_PointIndex).arrayNeigborsEdgePointIndex.size()-1).p2).smoothPosition );

	io_ArrayPoint->at(in_PointIndex).biNormal.Cross ( io_ArrayPoint->at(in_PointIndex).tangent, io_ArrayPoint->at(in_PointIndex).normal );
	io_ArrayPoint->at(in_PointIndex).tangent.Cross ( io_ArrayPoint->at(in_PointIndex).normal, io_ArrayPoint->at(in_PointIndex).biNormal );

	io_ArrayPoint->at(in_PointIndex).tangent.NormalizeInPlace();
	io_ArrayPoint->at(in_PointIndex).biNormal.NormalizeInPlace();

	//Update Point Refernce Frame.
	io_ArrayPoint->at(in_PointIndex).referenceFrame.Set (	io_ArrayPoint->at(in_PointIndex).tangent.GetX(), io_ArrayPoint->at(in_PointIndex).tangent.GetY(), io_ArrayPoint->at(in_PointIndex).tangent.GetZ(),
															io_ArrayPoint->at(in_PointIndex).normal.GetX(), io_ArrayPoint->at(in_PointIndex).normal.GetY(), io_ArrayPoint->at(in_PointIndex).normal.GetZ(),
															io_ArrayPoint->at(in_PointIndex).biNormal.GetX(), io_ArrayPoint->at(in_PointIndex).biNormal.GetY(), io_ArrayPoint->at(in_PointIndex).biNormal.GetZ() );

};

void ComputePointMushInitData ( std::vector<DMPoint_t> *io_ArrayPoint )
{
	for ( ULONG iPnt=0 ; iPnt<io_ArrayPoint->size() ; iPnt++ )
	{
		ComputePointReferenceFrame ( io_ArrayPoint, iPnt );

		//Compute the offset vector and final transform if needed.

		io_ArrayPoint->at(iPnt).offsetVector.Sub ( io_ArrayPoint->at(iPnt).position, io_ArrayPoint->at(iPnt).smoothPosition  );

		io_ArrayPoint->at(iPnt).transform.Set ( io_ArrayPoint->at(iPnt).tangent.GetX(), io_ArrayPoint->at(iPnt).tangent.GetY(), io_ArrayPoint->at(iPnt).tangent.GetZ(), 0.0f,
												io_ArrayPoint->at(iPnt).normal.GetX(), io_ArrayPoint->at(iPnt).normal.GetY(), io_ArrayPoint->at(iPnt).normal.GetZ(), 0.0f,
												io_ArrayPoint->at(iPnt).biNormal.GetX(), io_ArrayPoint->at(iPnt).biNormal.GetY(), io_ArrayPoint->at(iPnt).biNormal.GetZ(), 0.0f,
												io_ArrayPoint->at(iPnt).offsetVector.GetX(), io_ArrayPoint->at(iPnt).offsetVector.GetY(), io_ArrayPoint->at(iPnt).offsetVector.GetZ(), 1.0f );
	}
};

void ComputePointMushData ( std::vector<DMPoint_t> *io_ArrayPoint, CDataArray2DMatrix4f &in_DmInitData )
{
	CDataArray2DMatrix4f::Accessor	arrayDmInitData = in_DmInitData[0] ;
	XSI::MATH::CMatrix3f			offsetRotation ;
	XSI::MATH::CVector3f			offsetVector;

	for ( ULONG iPnt=0 ; iPnt<io_ArrayPoint->size() ; iPnt++ )
	{
		ComputePointReferenceFrame ( io_ArrayPoint, iPnt );

		offsetRotation.Set (	arrayDmInitData[iPnt].GetValue(0,0), arrayDmInitData[iPnt].GetValue(0,1),arrayDmInitData[iPnt].GetValue(0,2),
								arrayDmInitData[iPnt].GetValue(1,0), arrayDmInitData[iPnt].GetValue(1,1),arrayDmInitData[iPnt].GetValue(1,2),
								arrayDmInitData[iPnt].GetValue(2,0), arrayDmInitData[iPnt].GetValue(2,1),arrayDmInitData[iPnt].GetValue(2,2) );

		offsetVector.Set ( arrayDmInitData[iPnt].GetValue(3,0), arrayDmInitData[iPnt].GetValue(3,1),arrayDmInitData[iPnt].GetValue(3,2) );

		offsetRotation.Invert ( offsetRotation );
		offsetRotation.MulInPlace ( io_ArrayPoint->at(iPnt).referenceFrame );

		offsetVector.MulByMatrix3InPlace ( offsetRotation );
		
		io_ArrayPoint->at(iPnt).smoothCache.Add ( io_ArrayPoint->at(iPnt).smoothPosition, offsetVector );
	}

	for ( ULONG iPnt=0 ; iPnt<io_ArrayPoint->size() ; iPnt++ )
	{
		io_ArrayPoint->at(iPnt).smoothPosition = io_ArrayPoint->at(iPnt).smoothCache ;
	}
};