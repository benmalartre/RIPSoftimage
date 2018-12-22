/*--------------------------------------------------------------------
	IRTranslateHandle.cpp
---------------------------------------------------------------------*/
#include "IRTranslateHandle.h"

void IRTranslateHandle::MouseMove( ToolContext& in_ctxt )
{
	Application().LogMessage(L"IRTranslateHandle : MouseMove...");
	if(! in_ctxt.IsInteracting())
	{
		LONG x,y;
		in_ctxt.GetMousePosition( x, y );

		CLongArray points;
		points.Add( x );
		points.Add( y );

		_ctr = _elem->GetActiveCtr();
		//Application().LogMessage(_ctr.GetFullName());
		_tra = _ctr.GetKinematics().GetGlobal().GetTransform();
		GetAxis();
		GetActiveAxis(in_ctxt);

	}
}

void IRTranslateHandle::MouseDown( ToolContext& in_ctxt )
{
	if(!_ctr.IsValid())return;

	// Called when mouse button is pressed
	in_ctxt.GetMousePosition( _startX,_startY );

	if(GetClosestPositionOnAxis(in_ctxt, _startX, _startY, _initpos) != CStatus::OK)
		return;

	CRefArray r;
	r.Add(_ctr);
	in_ctxt.BeginTransformUpdate(r);
}

void IRTranslateHandle::MouseDrag( ToolContext& in_ctxt )
{
	if(!_ctr.IsValid())return;
	// Called when mouse is moved with a mouse button pressed
	LONG x, y;
	in_ctxt.GetMousePosition( x, y );

	CVector3 position;
	if(GetClosestPositionOnAxis(in_ctxt,x, y, position) != CStatus::OK)return ;

	// Compute relative translation
	CVector3 translation;
	translation.Sub( position,_initpos );

	//_tra.SetTranslation(translation);

	CRefArray r;
	r.Add(_ctr);
	// Apply the transform
	CTransformation xfo;
	xfo.SetTranslation( translation );
	in_ctxt.UpdateTransform( r, xfo, siTrn, siGlobal, siRelative, siXYZ );

	IRJoint* jnt = _elem->GetActiveJoint();
	jnt->SetPosition(_ctr.GetKinematics().GetGlobal().GetTransform().GetTranslation());

}

void IRTranslateHandle::MouseUp( ToolContext& in_ctxt )
{
	if(!_ctr.IsValid())return;

	CRefArray r;
	r.Add(_ctr);
	in_ctxt.EndTransformUpdate( r );
	in_ctxt.EnableSnapping( false );
}

void IRTranslateHandle::Draw( ToolContext& in_ctxt )
{
	// Called when a redraw occurs to allow the tool to show feedback
	if ( in_ctxt.IsActiveView() )
	{
		Application().LogMessage(L"Cls Ctr : "+_ctr.GetFullName());
		if ( _ctr.IsValid() )
		{
			in_ctxt.BeginViewDraw();
			DrawAxis(in_ctxt,IRAxisX);
			DrawAxis(in_ctxt,IRAxisY);
			DrawAxis(in_ctxt,IRAxisZ);
			in_ctxt.EndViewDraw();
		}
	}
}

void IRTranslateHandle::DrawAxis(ToolContext& in_ctxt,IRAxis axis)
{
	if(axis == _activeaxis)
		SetGLColor(IRSelectedHandleColor);
	else
	{
		switch(axis)
		{
			case IRAxisX:
				SetGLColor(IRXHandleColor);
				break;
			case IRAxisY:
				SetGLColor(IRYHandleColor);
				break;
			case IRAxisZ:
				SetGLColor(IRZHandleColor);
				break;
		}
	}
	CVector3 a;
	switch(axis)
	{
		case IRAxisX:
			a.Set(IRTranslateHandleSize,0.0,0.0);
			break;
		case IRAxisY:
			a.Set(0.0,IRTranslateHandleSize,0.0);
			break;
		case IRAxisZ:
			a.Set(0.0,0.0,IRTranslateHandleSize);
			break;
	}
		
	CMatrix4 vm, tm;
	in_ctxt.GetWorldToViewMatrix(vm);
	tm = _tra.GetMatrix4();
	tm.MulInPlace(vm);

	GLdouble mat[16];
	tm.Get(
		mat[0],  mat[1],  mat[2],  mat[3],
		mat[4],  mat[5],  mat[6],  mat[7],
		mat[8],  mat[9],  mat[10], mat[11],
		mat[12], mat[13], mat[14], mat[15]
	);
	glPushMatrix();
	glMultMatrixd(mat);

	glBegin( GL_LINES );
	glVertex3d(0.0,0.0,0.0);
	glVertex3d(a.GetX(),a.GetY(),a.GetZ());
	glEnd();

	glTranslated( a.GetX(),a.GetY(),a.GetZ());
	if(axis==IRAxisX)
		glRotatef(90.0,0.0,1.0,0.0);
	else if(axis == IRAxisY)
		glRotatef(-90.0,1.0,0.0,0.0);
	DrawCone(1.0);
	glPopMatrix();
}

void IRTranslateHandle::GetAxis()
{
	_origin = _tra.GetTranslation();
	CMatrix3 matrix = _tra.GetRotation().GetMatrix();
	CVector3 tangent(1.0,0.0,0.0);
	tangent.MulByMatrix3InPlace(matrix);
	_axis[0].Set(_origin,tangent);
	tangent.Set(0.0,1.0,0.0);
	tangent.MulByMatrix3InPlace(matrix);
	_axis[1].Set(_origin,tangent);
	tangent.Set(0.0,0.0,1.0);
	tangent.MulByMatrix3InPlace(matrix);
	_axis[2].Set(_origin,tangent);
}

void IRTranslateHandle::GetActiveAxis(ToolContext& in_ctxt)
{
	LONG x,y;
	in_ctxt.GetMousePosition(x,y);
	CVector3 o;
	in_ctxt.WorldToView(_origin,o);

	CVector3 mouse(x-o.GetX(),y-o.GetY(),0);
	_activeaxis = IRNoAxis; // No axis
	double maxProjection = IRHandleDragThreshold;

	for ( int i = 0; i < 3; i++ )
	{
		CVector3 initial, axis;
		if ( in_ctxt.WorldToView( _axis[i].GetOrigin(), initial ) != CStatus::OK ||
			 in_ctxt.WorldToView( _axis[i].GetPosition(1.0), axis ) != CStatus::OK )
			return;

		// Convert to a 2D vector (without depth)
		axis.SubInPlace( initial ).PutZ( 0.0 );
		double lenSquared = axis.GetLengthSquared();
		if ( lenSquared > IRHandleDragThreshold*IRHandleDragThreshold )
		{
			double projectedLen = fabs(mouse.Dot(axis)) * axis.GetLength() / lenSquared;
			if ( projectedLen > maxProjection )
			{
				_activeaxis = (IRAxis)i;
				maxProjection = projectedLen;
			}
		}
	}
}

CStatus IRTranslateHandle::GetClosestPositionOnAxis(ToolContext& in_ctxt,LONG x, LONG y,CVector3& io_pos)
{
	// Get world cursor ray
	CLine cursorRay;
	in_ctxt.EnableSnapping( false );
	if ( in_ctxt.GetWorldRay( x, y, cursorRay ) != CStatus::OK )
		return CStatus::False;

	// Will cursor intersect drag axis?
	if ( cursorRay.Parallel( _axis[_activeaxis] ) )
		return CStatus::False;

	// Compute intersection between cursor ray and drag axis
	io_pos = _axis[_activeaxis].GetClosestPoint( cursorRay );

	// Reject intersection points behind the camera
	CVector3 camera_pos;
	if ( in_ctxt.WorldToCamera( io_pos, camera_pos ) != CStatus::OK || camera_pos.GetZ() > 0.0 )
		return CStatus::False;

	else return CStatus::OK;
}