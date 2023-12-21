#ifndef U2I_EVENT_H
#define U2I_EVENT_H

#include <sicppsdk.h>
#include <xsi_ref.h>
#include <xsi_status.h>
#include <vector>

class U2IWindow;

SICALLBACK UsdICEValueChange_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdICESceneOpen_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdICEObjectAdded_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdICEObjectRemoved_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdICENewScene_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdICETimeChange_OnEvent(const XSI::CRef& in_ref);

#endif