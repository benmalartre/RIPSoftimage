/* \file menu_proxyCurve.cpp

	\brief bProxyCurveMenu menu callbacks definition.


	bProxyCurve
	Copyright (c) 2009  Christopher Crouzet  <christopher.crouzet@gmail.com>


	lmProxyCurve is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	any later version.

	lmProxyCurve is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with lmProxyCurve.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <xsi_context.h>
#include <xsi_menu.h>
#include <xsi_menuitem.h>

using namespace XSI;



SICALLBACK MergeCurve_Init(CRef& in_ctxt)
{
	Context ctxt ( in_ctxt );
	Menu menu ( ctxt.GetSource() );
	MenuItem menuItem;


	menu.AddCommandItem(L"MergeCurve", L"MergeCurveCmd", menuItem);


	return CStatus::OK;
}

