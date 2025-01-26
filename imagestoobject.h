#ifndef _IMAGESTOOBJECT_H_
#define _IMAGESTOOBJECT_H_


//Note from MSDN

//If you are building an extension DLL, and exporting using a .DEF file, place the following code at the beginning and end of your header files that contain the exported classes:
//#undef AFX_DATA
//#define AFX_DATA AFX_EXT_DATA
// <body of your header file>
//#undef AFX_DATA
//#define AFX_DATA
//These lines ensure that MFC variables that are used internally or that are added to your classes are exported (or imported) from your extension DLL. 
//Leaving out these four lines may cause your DLL to compile or link incorrectly or cause an error when the client application links to the DLL.

//I wont need to do this because I am not exporting any classes


#include "definitions.h"
#include "filepoints.h"
#include "errorConstants.h"

class imagesToObject {

private:
	TARGETCUBOID targetCuboid;
	BOOL flgCamInCuboid ;
	BOOL flgCamAtInfinity ;
	float dNear, dFar; 	
	float dX, dY, dZ; //the distance between two points on the target cuboid
	float xFirst, yFirst, zFirst ;
	float xLast, yLast, zLast ;
	FLTPOINT3D arrCuboidFt[4];
	FLTPOINT3D arrCuboidBk[4];	
	FLTPOINT3D arrFarPlane[4];
	FLTPOINT3D arrNearPlane[4];
	filePoints *filePts;
	UINT error;

	CUBOID_PLANES boundingPlanes;
		
public:	
	
	imagesToObject(PROCESS_MLD_INFO *inData) ;
	~imagesToObject();
	UINT processImage(); 
	UINT checkError() { return error; }//called to check the error
									//after class creation
	
private:
	PROCESS_MLD_INFO *dataPtr;
	
	float fltProximityValue; //Added on 29-Jan-2004 See the top of the cpp file for details
	
	UINT setNearFarRectangle(FLTPOINT3D incameraLocation,FLTPOINT3D inLookingAt, 
					BOOLEAN inFlgAtInfinity ) ;
	UINT setInvalid( UINT inX1, UINT inY, UINT inX2,
					UINT inXPixels, UINT inYPixels, FLTPOINT3D inCameraLocation);

	float valFromPlaneEqn(FLTPOINT3D inP, float inC1,float  inC2, float inC3,float  inK) ;
	
	BOOL sameSigns(float inVal1, float inVal2 ) ;
	float modulus(float inVal) ;

	BOOL withinFrustum(FLTPOINT3D inP,FLTPOINT3D inArrFt[4], FLTPOINT3D inArrBk[4]) ;
	BOOL withinPyramid(FLTPOINT3D inP,FLTPOINT3D inEnd,FLTPOINT3D inArrBk[4]);
	BOOL insideOfPlane(FLTPOINT3D inPoint,FLTPOINT3D inP1,FLTPOINT3D inP2,FLTPOINT3D inP3 );
	UINT getClosestPtIndex(float inFltPosition, BOOL blnStartPt);
	
	//Functions added in Mar -04----------------------------------------------
	BOOL withinFrustum(FLTPOINT3D inP);
	BOOL withinPyramid(FLTPOINT3D inP);
	BOOL insideOfPlane(FLTPOINT3D inPoint, PLANE_EQUATION_VALUES inPlnValues );
	void getPlane(FLTPOINT3D inP1, FLTPOINT3D inP2, FLTPOINT3D inP3, 
		PLANE_EQUATION_VALUES &outPlnValues );	
	//-------------------------------------------------------------------------
	
	FLTPOINT3D getMiddlePoint(FLTPOINT3D inP1, FLTPOINT3D inP2, float inD1ByD2) ;
	FLTPOINT3D getMiddlePoint(FLTPOINT3D inP1, FLTPOINT3D inP2, float inM, float inMPlusN) ;
	FLTPOINT3D getMiddlePoint(FLTPOINT3D inP1, FLTPOINT3D inP2, UINT inM, UINT inMPlusN);
	
	float Min(float , float, float , float, float ); //five parameters
	float Max(float , float, float , float, float );
	float Min(float , float, float , float , float, float , float , float);//8 params
	float Max(float , float, float , float , float, float , float , float);
};


#endif







