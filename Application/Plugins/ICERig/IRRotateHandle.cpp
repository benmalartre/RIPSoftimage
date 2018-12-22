/*--------------------------------------------------------------------
	IRTranslateHandle.h
---------------------------------------------------------------------*/
#include "IRRotateHandle.h"

// Arcball helper functions (courtesy of Ken Shoemake/Graphic Gems IV)
static CVector3 MouseOnSphere( const CVector3 &in_point, const CVector3 &in_center, double in_radius )
{
	CVector3 result;
	result.Sub(in_point, in_center).ScaleInPlace(1.0 / in_radius);
    double norm = result.GetLengthSquared();
    if ( norm > 1.0 )
		result *= 1.0 / sqrt(norm);
    else
		result.PutZ( sqrt(1.0 - norm) );
	return result;
}

static CRotation RotFromBallPoints( const CVector3 &in_from, const CVector3 &in_to )
{
	return CRotation( CQuaternion(
		in_from.GetX()*in_to.GetX() + in_from.GetY()*in_to.GetY() + in_from.GetZ()*in_to.GetZ(),
		in_from.GetY()*in_to.GetZ() - in_from.GetZ()*in_to.GetY(),
		in_from.GetZ()*in_to.GetX() - in_from.GetX()*in_to.GetZ(),
		in_from.GetX()*in_to.GetY() - in_from.GetY()*in_to.GetX() )
	);
}

static CVector3 ConstrainToAxis( const CVector3 &in_point, const CVector3 &in_axis )
{
	CVector3 result, axis, onPlane;
	axis.Scale( in_axis.Dot(in_point), in_axis );
	onPlane.Sub( in_point, axis );

	double norm = onPlane.GetLengthSquared();
	if ( norm > 0.0 ) {
		result.Scale( 1.0/sqrt(norm), onPlane );
	} else if ( in_axis.GetZ() == 1.0 ) {
		result.Set( 1.0, 0.0, 0.0 );
	} else {
		result.Set( -in_axis.GetY(), in_axis.GetX(), 0.0 ).NormalizeInPlace();
	}
	return result;
}

void IRRotateHandle::MouseMove(ToolContext& in_ctxt)
{
	Application().LogMessage(L"IRRotateHandle Mouse Move");
}

void IRRotateHandle::MouseDown( ToolContext& in_ctxt )
{
	CRefArray ref;
	ref.Add(_elem->GetActiveCtr());
	in_ctxt.BeginTransformUpdate( ref );
	in_ctxt.GetMousePosition( m_startX, m_startY );
	m_dBallRadius = 0.0;
	m_activeConstraint = -1;

	double dScale = GetViewScaleFactor( in_ctxt, l_bbox.GetMidPoint(BBox::kBack, true/*world*/), kRotateStickLength );
	if ( dScale == 0.0 )
		return;

	// Initialize arcball rotation controller
	m_pivot = l_bbox.GetCenter( true/*world*/ );
	m_constraintAxes[0] = l_bbox.GetNormal( BBox::kRight, true/*world*/ );
	m_constraintAxes[1] = l_bbox.GetNormal( BBox::kTop,   true/*world*/ );

	// Find the end of the stick to compute the radius
	CVector3 extent = l_bbox.GetMidPoint( BBox::kBack );
	extent.PutZ( extent.GetZ() + dScale + dScale*kRotateStickSphereRadius/kRotateStickLength );
	extent = l_bbox.BBoxToWorld( extent ); // Convert to world-space
	double dRadius = extent.SubInPlace( m_pivot ).GetLength();

	if ( InitializeArcball( in_ctxt, m_startX, m_startY, m_pivot, dRadius ) != CStatus::OK )
		return;
}

void IRRotateHandle::MouseDrag( ToolContext& in_ctxt )
{/*

	LONG x, y;
	in_ctxt.GetMousePosition( x, y );
	CVector3 mouseVec( x-m_startX, y-m_startY, 0.0 );
	if ( mouseVec.GetLength() < kHandleDragThreshold )
		return;

	// Get current rotation value
	CRotation rot = GetArcballRotation( in_ctxt, x, y, &m_constraintAxis );

	// Negate rotation angle if negative scaling... maybe?

	// Round to the nearest rotation snap increment when control key pressed
	if ( in_ctxt.IsControlKeyDown() )
		rot = GetNearestRotationIncrement( rot );

	// Apply the transform
	CTransformation xfo;
	xfo.SetRotation( rot );
	in_ctxt.UpdateTransform( m_objects, xfo, siRot, siGlobal, siRelative, siXYZ, true, m_pivot );

	// Update the box transform
	CTransformation bboxXfo;
	bboxXfo.Mul( m_initialXfo, xfo );
	m_pTool->UpdateBBoxOrientation( bboxXfo.GetRotation() );
	*/
}

void IRRotateHandle::MouseUp( ToolContext& in_ctxt )
{
	/*
	in_ctxt.EndTransformUpdate( m_objects );
	m_objects = CRefArray();
	*/
}

void IRRotateHandle::Draw( ToolContext &in_ctxt )
{
	/*
	const BBox &l_bbox = m_pTool->GetBBox();
	if ( (IsSelected() && in_ctxt.IsInteracting() && in_ctxt.IsActiveView()) ||
		 l_bbox.GetApproxViewSize( in_ctxt ) > kRotateStickMinBBoxSize )
	{
		double l_dScale = GetViewScaleFactor( in_ctxt, l_bbox.GetMidPoint(BBox::kBack, true), kRotateWheelOuterRadius );
		if ( l_dScale != 0.0 )
		{
			GLdouble mat[16];
			CTransformation transfo = l_bbox.GetTransform();
			transfo.SetScalingFromValues(1.0, 1.0, 1.0); // Cancel out scaling
			transfo.GetMatrix4().Get(
				mat[0],  mat[1],  mat[2],  mat[3],
				mat[4],  mat[5],  mat[6],  mat[7],
				mat[8],  mat[9],  mat[10], mat[11],
				mat[12], mat[13], mat[14], mat[15]
			);
			glPushMatrix();
			glMultMatrixd( mat );
			ULONG color = IsSelected() ? kSelectedHandleColor : kRotateWheelColor;
			SetGLColor( color );
			bool l_bUseLighting = !IsSelected();
			if ( l_bUseLighting )
			{
				GLfloat blackColor[] = { 0.0, 0.0, 0.0, 1.0 };
				GLfloat diffuseColor[] = { RGBA_R(color)/255.0f, RGBA_G(color)/255.0f, RGBA_B(color)/255.0f, RGBA_A(color)/255.0f };
				GLfloat ambientColor[] = { diffuseColor[0]*0.2f, diffuseColor[1]*0.2f, diffuseColor[2]*0.2f, diffuseColor[3] };
				glMaterialfv( GL_FRONT, GL_AMBIENT, ambientColor );
				glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuseColor );
				glMaterialfv( GL_FRONT, GL_SPECULAR, blackColor );
				glMaterialf(  GL_FRONT, GL_SHININESS, 0.0 );
				glEnable( GL_LIGHTING );
			}
			glDepthMask( GL_TRUE );
			CVector3 l_Pos = l_bbox.GetMidPoint( BBox::kBack );
			glTranslated( l_Pos.GetX(), l_Pos.GetY(), l_Pos.GetZ() );
			glRotated( 90.0, 1.0, 0.0, 0.0 );
			DrawTorus( l_dScale/kRotateWheelOuterRadius*kRotateWheelInnerRadius, l_dScale, kRotateWheelSides, kRotateWheelRings );
			glDepthMask( GL_FALSE );
			if ( l_bUseLighting )
				glDisable( GL_LIGHTING );
			glPopMatrix();
		}
	}
	*/
}

/*
void IRRotateHandle::DrawTorus( double in_dInnerRadius, double in_dOuterRadius, int in_nSides, int in_nRings )
{

	double l_dR1 = in_dOuterRadius, l_dR2 = in_dInnerRadius;
	double l_dTwoPIRings = 2.0*PI/in_nRings, l_dTwoPISides = 2.0*PI/in_nSides;
	double l_dTheta = 0.0, l_dSinTheta = 0.0, l_dCosTheta = 1.0;
    for ( int i = 0; i < in_nRings; ++i ) 
	{
		double l_dTheta1 = (i+1)*l_dTwoPIRings;
		double l_dSinTheta1 = sin( l_dTheta1 );
		double l_dCosTheta1 = cos( l_dTheta1 );
		double l_dPhi = 0.0, l_dSinPhi = 0.0, l_dCosPhi = 1.0;
		for (int j = 0; j < in_nSides; j++)
		{
			double l_dPhi1 = (j+1)*l_dTwoPISides;
			double l_dSinPhi1 = sin( l_dPhi1 );
			double l_dCosPhi1 = cos( l_dPhi1 );

			glBegin(GL_QUADS);
				glNormal3d( l_dCosTheta*l_dCosPhi1, l_dSinPhi1, l_dSinTheta*l_dCosPhi1 );
				glVertex3d( l_dCosTheta*(l_dR1+l_dR2*l_dCosPhi1), l_dR2*l_dSinPhi1, l_dSinTheta*(l_dR1+l_dR2*l_dCosPhi1) );
				glNormal3d( l_dCosTheta1*l_dCosPhi1, l_dSinPhi1, l_dSinTheta1*l_dCosPhi1 );
				glVertex3d( l_dCosTheta1*(l_dR1+l_dR2*l_dCosPhi1), l_dR2*l_dSinPhi1, l_dSinTheta1*(l_dR1+l_dR2*l_dCosPhi1) );
				glNormal3d( l_dCosTheta1*l_dCosPhi, l_dSinPhi, l_dSinTheta1*l_dCosPhi );
				glVertex3d( l_dCosTheta1*(l_dR1+l_dR2*l_dCosPhi), l_dR2*l_dSinPhi, l_dSinTheta1*(l_dR1+l_dR2*l_dCosPhi) );
				glNormal3d( l_dCosTheta*l_dCosPhi, l_dSinPhi, l_dSinTheta*l_dCosPhi );
				glVertex3d( l_dCosTheta*(l_dR1+l_dR2*l_dCosPhi), l_dR2*l_dSinPhi, l_dSinTheta*(l_dR1+l_dR2*l_dCosPhi) );
			glEnd();

			l_dPhi    = l_dPhi1;
			l_dCosPhi = l_dCosPhi1;
			l_dSinPhi = l_dSinPhi1;
		}
		l_dTheta    = l_dTheta1;
		l_dCosTheta = l_dCosTheta1;
		l_dSinTheta = l_dSinTheta1;
    }
	
}

*/

LONG IRRotateHandle::GetClosestArcballAxis( ToolContext &in_ctxt, LONG x, LONG y, LONG numConstraints, const CVector3 *constraintAxes )
{
	if ( numConstraints < 1 || m_dBallRadius == 0.0 )
		return -1; // Error

	// Note: arcball is very efficient so we compute the rotation for each axis and take the largest...
	LONG nearest = 0;
	double maxAngle = 0.0;
	CVector3 origin( 0.0, 0.0, 0.0 );
	in_ctxt.WorldToCamera( origin, origin );
	CVector3 from = MouseOnSphere( CVector3(m_ballDownX, m_ballDownY, 0.0), m_ballCenter, m_dBallRadius );
	CVector3 to = MouseOnSphere( CVector3(x, y, 0.0), m_ballCenter, m_dBallRadius );
	for ( int i = 0; i < numConstraints; ++i )
	{
		CVector3 axis;
		double dAngle = 0.0;
		in_ctxt.WorldToCamera( constraintAxes[i], axis );
		axis.SubInPlace( origin ).NormalizeInPlace();
		CRotation rot = RotFromBallPoints( ConstrainToAxis(from, axis), ConstrainToAxis(to, axis) );
		rot.GetAxisAngle( dAngle );
		dAngle = fabs( dAngle );
		if ( dAngle > maxAngle ) {
			maxAngle = dAngle;
			nearest = i;
		}
	}
	return nearest;
}

CRotation IRRotateHandle::GetArcballRotation( ToolContext &in_ctxt, LONG x, LONG y, const CVector3 *constraintAxis )
{
	CRotation result;
	if ( m_dBallRadius == 0.0 )
		return result;

	// Get intitial and current points
	CVector3 from = MouseOnSphere( CVector3(m_ballDownX, m_ballDownY, 0.0), m_ballCenter, m_dBallRadius );
	CVector3 to = MouseOnSphere( CVector3(x, y, 0.0), m_ballCenter, m_dBallRadius );

	// Handle axis contraint
	if ( constraintAxis != NULL )
	{
		// Constraint must be converted to camera-space
		CVector3 origin( 0.0, 0.0, 0.0 ), axis;
		in_ctxt.WorldToCamera( origin, origin );
		in_ctxt.WorldToCamera( *constraintAxis, axis );
		axis.SubInPlace( origin ).NormalizeInPlace();
		from = ConstrainToAxis( from, axis );
		to = ConstrainToAxis( to, axis );
	}

	// Get rotation and convert to world-space
	double dAngle = 0.0;
	CRotation rot = RotFromBallPoints( from, to );
	CVector3 axis = rot.GetAxisAngle( dAngle );
	// Choose an arbitrary axis if it is undefined (dAngle == 0)
	if (axis.GetX() == 0.0 && axis.GetY() == 0.0 && axis.GetZ() == 0.0 )
		axis.PutX( 1.0 );
	
	dAngle *= 0.5; // Half rotation value to map 1:1 with mouse movement
	CVector3 origin( 0.0, 0.0, 0.0 );
	in_ctxt.CameraToWorld( origin, origin );
	in_ctxt.CameraToWorld( axis, axis );
	axis.SubInPlace( origin ).NormalizeInPlace();
	rot.SetFromAxisAngle( axis, dAngle );
	return rot;
}

CRotation IRRotateHandle::GetNearestRotationIncrement( const CRotation &in_rot )
{
	CRotation result( in_rot );
	CValue rotSnap( 10.0 );
	//Application().GetPreferences().GetPreferenceValue( L"TransformProperties.rotatesnap", rotSnap );

	double dAngle = 0.0;
	CVector3 axis = result.GetAxisAngle( dAngle );
	// Choose an arbitrary axis if it is undefined (dAngle == 0)
	if (axis.GetX() == 0.0 && axis.GetY() == 0.0 && axis.GetZ() == 0.0 )
		axis.PutX( 1.0 );

	dAngle = DegreesToRadians( GetClosestIncrement(RadiansToDegrees(dAngle), rotSnap) );
	result.SetFromAxisAngle( axis, dAngle );
	return result;
}