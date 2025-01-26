// imgtoobj.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>
#include "imagestoobject.h"
#include "errorConstants.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static AFX_EXTENSION_MODULE ImgtoobjDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("IMGTOOBJ.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(ImgtoobjDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(ImgtoobjDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("IMGTOOBJ.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(ImgtoobjDLL);
	}
	return 1;   // ok
}


extern "C" _declspec (dllexport) UINT processMold(PROCESS_MLD_INFO *inData) {
		
	//Validate the calling component
	if(inData->getSecurity() != SECURITY_STRING_IMGTOOBJ)
		return INVALID_SECURITY_STRING;
	
	imagesToObject *clsPtr;
	
	clsPtr = new imagesToObject(inData);

	UINT intRtnVal = clsPtr->checkError();
	
	if(intRtnVal != SUCCESS) {
		delete clsPtr;
		return intRtnVal;
	}

	intRtnVal = clsPtr->processImage();	
	delete clsPtr;
	return intRtnVal;	
}
