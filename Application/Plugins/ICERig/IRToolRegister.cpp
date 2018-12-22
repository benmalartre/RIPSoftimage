// IRToolRegister
#include "IRRegister.h"


SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"ICERigTools");
	in_reg.PutVersion(1,0);
	in_reg.RegisterTool(L"IRSetElement");
	in_reg.RegisterTool(L"IRSimpleBrush");
	in_reg.RegisterTool(L"IRManipulator");
	in_reg.RegisterEvent( L"IRSetElement_SelectionChanged", siOnSelectionChange );

	return CStatus::OK;
}
