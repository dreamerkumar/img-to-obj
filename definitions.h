#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include "math.h"
#include <afxext.h>

//Variable definition

//Common naming  conventions:
//All input parameters should have prefix in.
//All output parameters should have prefix out.
//All structures should be defined in capital letters without underscores.
//All constants should be in capital letters and can have underscores. 

#define MOLD_HEADER_SIZE		141 
//41 bytes are used as of now while the rest have been kept for future use
#define MOLD_POS_FOR_IMAGE_COUNT 37
//Camera range:
struct CAMERARATIO {
	float xRatio;
	float yRatio;
	float xRangeAtInfinity;
	float yRangeAtInfinity;
};

//CAMERARATIO cameraRatio;

//Note : A floating type number takes 4 bytes of memory and has 7 digits of precision.
//Similarly a double takes 8 bytes of memory and has 15 digits of precision. 

//Colour range :
struct COLORRANGE {
	unsigned char rangeType;
	unsigned char min1;
	unsigned char min2;
	unsigned char min3;
	unsigned char max1;
	unsigned char max2;
	unsigned char max3;
};

//COLORRANGE colorRange;
//Possible  values of  colorRange. rangeType

#define FIXED_RGB                   0
#define FIXED_HSL                   1
#define VARIABLE_RGB                2
#define VARIABLE_HSL                3
#define VARIABLE_R                  4
#define VARIABLE_G                  5
#define VARIABLE_B                  6
#define VARIABLE_H                  7
#define VARIABLE_S                  8
#define VARIABLE_L                  9

//min1, min2, min3, max1, max2, max3 will have values from 0 to 255 if colours are of type RGB and 0 through 240 for HSL.

//Note : An integer type takes up two bytes of memory. A char type takes only 1 byte of memory.

//TARGETCUBOID targetCuboid;

//xPoints, yPoints and zPoints specify the number of points along the three axes and 
//can have values from 2 to 1000. 

//minx, maxx, miny, maxy, minz, maxz can have values from - 500  to + 500.
struct TARGETCUBOID {

	UINT uintXPoints;
	UINT uintYPoints;
	UINT uintZPoints;
	float fltMinx;
	float fltMiny;
	float fltMinz;
	float fltMaxx;
	float fltMaxy;
	float fltMaxz;
};

//Points in 3d space:
//although this is defined as a class for operator overloading
//I will be using it more like a struct so it's name is in 
//capital letters
class FLTPOINT3D {//FLTPOINT3D fltPoint3d; Can be used to define the points in cms.

public: 
	float x;
	float y;
	float z;
	FLTPOINT3D() ;
	FLTPOINT3D( float inX, float inY, float inZ ) ;
	operator = ( FLTPOINT3D inP ) ;
	FLTPOINT3D operator + ( FLTPOINT3D inP ) ;
	FLTPOINT3D operator * ( float inK ) ;
	FLTPOINT3D operator / ( float inK ) ;	

}; 


struct INTPOINT3D {
	UINT x;
	UINT y;
	UINT z;
}; 
//INTPOINT3D intPoint3d;

//Can be used to define the coordinate points of the target cuboid.
//(Since the cuboid is of maximum 1001 cm and there can be not more than 20 points per cm, this value will not exceed 20020 which is in the int range).

//Color
class RGBCOLOR {
public:
	BYTE red;
	BYTE blue;
	BYTE green;
	RGBCOLOR() ;
	BOOL operator == ( RGBCOLOR inC ) ; 
	BOOL operator != ( RGBCOLOR inC ) ; 
};

//RGBCOLOR rgbColor;

//Image Parameters:
struct IMAGEPARAMS  {
	RGBCOLOR InvalidColor;
	UINT height;
	UINT width;
	unsigned char extraBytes;
	UINT leftPadding;
	UINT rightPadding;
	UINT topPadding;
	UINT bottomPadding;
	FLTPOINT3D cameraLocation;
	FLTPOINT3D lookingAt; 
	BOOLEAN flgAtInfinity;
};
//IMAGEPARAMS imageParams; 

//Vector
//although this is defined as a class for operator overloading
//I will be using it more like a struct so it's name is in 
//capital letters
class VECTOR { 
public :
	VECTOR();
	VECTOR(float inI, float inJ, float inK);
	VECTOR (FLTPOINT3D inP);
	float i;
	float j;
	float k;
	operator = ( VECTOR inV ) ;
	VECTOR operator + ( VECTOR inV ) ; 
	VECTOR operator - ( VECTOR inV ) ; 
	VECTOR operator * ( VECTOR inV ) ; //vector cross product
	VECTOR operator * (float inK   ) ;//scalar multiplcation
	VECTOR operator / (float inK   ) ;//check for null value before calling this function
	float  modulus();
	
};


//Cuboid Info:
struct CUBOIDFILEINFO {
	UINT x, y, z; //Number of points along the three axes
	float minx, maxx, miny, maxy, minz, maxz; //Positions of the cuboid corners
};//TODO: this structure is almost similar to struct TARGETCUBOID. We can use a single one 

//Cube Index used by fileCubes:
struct CUBEINDEX {
	UINT x, y, z; //Index
	BYTE face; //Face that is already processed
};

//CUBEINDEX cubeIndex;

//Cube corner values 
class CUBECORNERS {

public:
	CUBECORNERS();
	operator = ( CUBECORNERS inCb ) ;
	BOOL backTopLeft ;
	BOOL backBottomLeft ;
	BOOL backBottomRight ;
	BOOL backTopRight ; 

	BOOL frontTopLeft ;
	BOOL frontBottomLeft ;
	BOOL frontBottomRight ;
	BOOL frontTopRight ;

};  

//CUBECORNERS cubeCorners;

#define MINX			1
#define MINY			2
#define MINZ			3
#define MAXX			4
#define MAXY			5
#define MAXZ			6
#define MINX_MINY_MAXZ  7


#define BACK_TOP_LEFT      1 
#define BACK_BOTTOM_LEFT   2 
#define BACK_BOTTOM_RIGHT  3 
#define BACK_TOP_RIGHT     4 
#define FRONT_TOP_LEFT     5
#define FRONT_BOTTOM_LEFT  6 
#define FRONT_BOTTOM_RIGHT 7
#define FRONT_TOP_RIGHT    8



#define SECURITY_STRING_IMGTOOBJ "S^&S$G#D#G)(G(*^D<ytSggDnFn"


struct PROCESS_INFO {
	PROCESS_INFO() {
		g_blnCancelProcess = false;
		g_fltPercentComplete = 0.0f;
		g_uintTotalCount = 0;
		g_strInputFile = "";
		g_strOutputFile = "";
		g_strSecurity = "";

	}
private:
	BOOL g_blnCancelProcess;
	UINT g_uintTotalCount;
	float g_fltPercentComplete;
	CString g_strSecurity;

public:
	CString g_strInputFile;
	CString g_strOutputFile;
	
	void setSecurity(CString inStrSecurity) {
		g_strSecurity = inStrSecurity;
	}

	CString getSecurity() {
		return g_strSecurity;
	}
	
	void cancelProcess() {
		g_blnCancelProcess = true;
	}
		
	float getCompletedPercent() {
		return g_fltPercentComplete;
	}
	
	BOOL isCancelled() {
		return g_blnCancelProcess;
	}

	void initMaxNumber(UINT inUintMaxNumber) {
		g_uintTotalCount = inUintMaxNumber;
	}
	
	void setNewPercent(UINT inUintCompleted) {
		VERIFY(g_uintTotalCount > 0 && inUintCompleted <= g_uintTotalCount);
		float fltPercentComplete = ((float)inUintCompleted/(float)g_uintTotalCount)*100.0f;
		VERIFY(fltPercentComplete >= g_fltPercentComplete);
		g_fltPercentComplete = fltPercentComplete;
	}
};


struct PROCESS_MLD_INFO : PROCESS_INFO {

	BYTE *imgPointer; 
	IMAGEPARAMS imgParams;
	CAMERARATIO cameraRatio;
	
	UINT uintNewXPts, uintNewYPts; //If the first slide is being processed and these points 
								 //are other than zero the cuboid X and Y points are modified
								//with the new values
	PROCESS_MLD_INFO() {
		uintNewXPts = uintNewYPts = 0;		
	}
};

struct PLANE_EQUATION_VALUES {
	//The equation of the plane is expressed as lx+my+nz+k =0
	float fltL, fltM, fltN, fltK;
	
	PLANE_EQUATION_VALUES () {
		fltL = fltM = fltN = fltK  = 0.0f;
	}
	
	PLANE_EQUATION_VALUES(float inFltL, float inFltM, float inFltN, float inFltK) {
		fltL= inFltL; 
		fltM= inFltM;
		fltN= inFltN;
		fltK= inFltK;
	}
	operator = (PLANE_EQUATION_VALUES inPlane) {
		fltL = inPlane.fltL;
		fltM = inPlane.fltM;
		fltN = inPlane.fltN;
		fltK = inPlane.fltK;
	}
};

struct CUBOID_PLANES {
	PLANE_EQUATION_VALUES leftPlane, rightPlane, topPlane, 
		bottomPlane, frontPlane, backPlane;
};
/////////////////////////////////////////////////////////////////////////////////
//Function
int getIntFromFloat(float inFltVal);

#endif
