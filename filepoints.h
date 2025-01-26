#ifndef _FILEPOINTS_H_
#define _FILEPOINTS_H_
#include <afxext.h>
#include "errorConstants.h"
#include "definitions.h"

class filePoints : public CObject
{
	
	DECLARE_SERIAL(filePoints)

public: 
	filePoints();
	filePoints(CString inStrCuboidLocatn);
	void retrieveValues(TARGETCUBOID &inT);
	~filePoints();	
	UINT checkError();//called to check the error
					 //after file creation
	UINT setPoint(UINT inX, UINT inY, UINT inZ) ;

	UINT setPointRanges(UINT inUintX1, UINT inUintX2, UINT inUintY, UINT inUintZ) ;

	void updateImageCount(); //Increases the stored count of images processed by mold file 
							//by 1
private:
	UINT cuboidWidth;
	UINT cuboidHeight;
	UINT cuboidThickness;
	UINT totalBytes;
	UINT error;
	CFile fp;
	BYTE read() ;
	UINT readInt();
	float readFloat();
	void write(BYTE inByte)	;
	void write(UINT inVal) ;
	void write(float inVal);

	UINT getBitBytePos(UINT inUintX, UINT inUintY, UINT inUintZ, 
		UINT &outUintBitPos, UINT &outUintBytePos);
	void setInvalidBits(BYTE &inOutBtTarget, BYTE btStartBit, BYTE btEndBit);
	BYTE * g_btXAxisData;
	
};

#endif