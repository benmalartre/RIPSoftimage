/*

		MergeCurve Register.cpp

 */

#include <xsi_pluginitem.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>

using namespace XSI;



SICALLBACK XSILoadPlugin(PluginRegistrar& in_reg)
{
	// Plug-in details.
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"MergeCurve");
	in_reg.PutEmail(L"benmalartre@hotmail.com");
	in_reg.PutVersion(1, 0);

	// Plug-in items.
	PluginItem item;

	item = in_reg.RegisterCommand(L"MergeCurveCmd", L"MergeCurve");
	item = in_reg.RegisterCommand(L"MergeAddCurveCmd", L"MergeAddCurve");
	item = in_reg.RegisterCommand(L"MergeRemoveCurveCmd", L"MergeRemoveCurve");

	item = in_reg.RegisterMenu(siMenuTbModelCreateCurveID, L"MergeCurve", false, false);
	item = in_reg.RegisterOperator(L"MergeCurveOp");

	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin(const PluginRegistrar& in_reg)
{
	return CStatus::OK;
}
