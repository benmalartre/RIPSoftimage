///////////////////////////////////////////////////////////////////////////
//
// File: ToolHandle.cpp
//
// Description: Base class for handles
//
// Copyright 2010 Autodesk, Inc. All rights reserved.
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy
// form.
//
///////////////////////////////////////////////////////////////////////////
#include "IRHandle.h"

namespace ICERIG {
  IRHandle::IRHandle() :_selected(false) {}

  IRHandle::~IRHandle() {}

  double ToolHandle::GetViewScaleFactor(ToolContext& in_ctxt, const CVector3& in_Pos, double in_dViewSize) const
  {
    CVector3 l_Offset;
    if (in_ctxt.WorldToView(in_Pos, l_Offset) == CStatus::OK)
    {
      // Displace point 1-pixel in view-space
      l_Offset.PutX(l_Offset.GetX() + 1.0/*pixels*/);
      if (in_ctxt.ViewToWorld(l_Offset, l_Offset) == CStatus::OK)
      {
        l_Offset.SubInPlace(in_Pos);
        return l_Offset.GetLength() * in_dViewSize;
      }
    }
    return 0.0; // Projection failed
  }

  LONG ToolHandle::GetClosestDragAxis(ToolContext& in_ctxt, const CVector3& in_mouseVec, CLine in_axes[], double& out_projection) const
  {
    // Calculate axis that best matches the mouse drag direction in screen-space
    LONG result = -1; // No axis
    double maxProjection = kHandleDragThreshold;
    for (int i = 0; i < 3; ++i)
    {
      CVector3 initial, axis;
      if (in_ctxt.WorldToView(in_axes[i].GetOrigin(), initial) != CStatus::OK ||
        in_ctxt.WorldToView(in_axes[i].GetPosition(1.0), axis) != CStatus::OK)
        return result;

      // Convert to a 2D vector (without depth)
      axis.SubInPlace(initial).PutZ(0.0);
      double lenSquared = axis.GetLengthSquared();
      if (lenSquared > kHandleDragThreshold * kHandleDragThreshold)
      {
        double projectedLen = fabs(in_mouseVec.Dot(axis)) * axis.GetLength() / lenSquared;
        if (projectedLen > maxProjection)
        {
          result = i;
          out_projection = maxProjection = projectedLen;
        }
      }
    }
    return result;
  }

  double ToolHandle::GetClosestIncrement(double in_value, double in_increment) const
  {
    if (in_increment > 0.0)
    {
      double dInteger;
      double dFraction = modf(in_value / in_increment, &dInteger);
      in_value = (dInteger + (LONG)(dFraction + (dFraction < 0 ? -0.5 : 0.5))) * in_increment;
    }
    return in_value;
  }
}