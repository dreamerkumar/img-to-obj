#include "stdafx.h"
#ifndef _DEFINITIONS_CPP_
#define _DEFINITIONS_CPP_


#include "definitions.h"

///////////////////FLTPOINT3D///////////////////////////////////////////////
FLTPOINT3D::FLTPOINT3D() {
	x = y = z = 0.0f;
}

FLTPOINT3D::FLTPOINT3D( float inX, float inY, float inZ ) {
	x = inX; y = inY; z = inZ;
}

FLTPOINT3D::operator = ( FLTPOINT3D inP ) {
	x = inP.x ; y = inP.y ; z = inP.z ; 
}

FLTPOINT3D FLTPOINT3D::operator + ( FLTPOINT3D inP ) {
	FLTPOINT3D outP;
	outP.x = x + inP.x ; outP.y = y + inP.y ; outP.z = z + inP.z ; 
	return outP;
}

FLTPOINT3D FLTPOINT3D::operator * ( float inK ) {
	FLTPOINT3D outP;
	outP.x = x * inK; outP.y = y * inK; outP.z = z * inK; 
	return outP;
}

FLTPOINT3D FLTPOINT3D::operator / ( float inK ) {
	FLTPOINT3D outP;
	outP.x = x / inK; outP.y = y / inK; outP.z = z / inK; 
	return outP;
}

/////////////////////////////RGBCOLOR/////////////////////////////////////////
RGBCOLOR::RGBCOLOR() 
	{red = blue = green = 0 ; }

BOOL RGBCOLOR::operator == ( RGBCOLOR inC ) {
	if(red == inC.red && green == inC.green && blue == inC.blue ) 
		return true ;
	else 
		return false;
}

BOOL RGBCOLOR::operator != ( RGBCOLOR inC ) {
	if(red == inC.red && green == inC.green && blue == inC.blue ) 
		return false;
	else 
		return true;
}

///////////////////////////VECTOR/////////////////////////////////////////////
VECTOR::VECTOR() 
	{i = j = k = 0.0f;}

VECTOR::VECTOR(float inI, float inJ, float inK) 
	{i = inI; j = inJ; k = inK;}

VECTOR::VECTOR (FLTPOINT3D inP) 
	{i = inP.x ; j = inP.y ; k = inP.z ;}

VECTOR::operator = ( VECTOR inV ) {	
	i = inV.i; j = inV.j; k =inV.k; 
}

VECTOR VECTOR::operator + ( VECTOR inV ) {	
	VECTOR outV;
	outV.i = i + inV.i;	outV.j = j + inV.j;	outV.k = k + inV.k; 
	return outV ;
}

VECTOR VECTOR::operator - ( VECTOR inV ) {	
	VECTOR outV;
	outV.i = i - inV.i;	outV.j = j - inV.j;	outV.k = k - inV.k; 
	return outV ;
}

VECTOR VECTOR::operator * ( VECTOR inV ) {	
	VECTOR outV;
	outV.i = j*inV.k - inV.j*k ; 
	outV.j = inV.i*k - inV.k*i ;	
	outV.k = i*inV.j - j*inV.i ;
	return outV ;
}

VECTOR VECTOR::operator * (float inK ) {	
	VECTOR outV;
	outV.i = i * inK;	outV.j = j * inK;	outV.k = k * inK; 
	return outV ;
}

VECTOR VECTOR::operator / (float inK ) {//check for null value 
	VECTOR outV;                //before calling this function	
	outV.i = i / inK;	outV.j = j / inK;	outV.k = k / inK; 
	return outV ;
}

float VECTOR::modulus() {
	return (float)sqrt(i*i + j*j + k*k);
}
///////////////////////////CUBECORNERS///////////////////////////////////////////
CUBECORNERS::CUBECORNERS() {
	 
	//Set all the flags to zero
	 backTopLeft      = false ;
	 backBottomLeft   = false ;
	 backBottomRight  = false ;
	 backTopRight     = false ;

	 frontTopLeft     = false ;
	 frontBottomLeft  = false ;
	 frontBottomRight = false ;
	 frontTopRight    = false ;
}

CUBECORNERS::operator = ( CUBECORNERS inCb ) {	
	 
	 backTopLeft      = inCb.backTopLeft;
	 backBottomLeft   = inCb.backBottomLeft;
	 backBottomRight  = inCb.backBottomRight ;
	 backTopRight     = inCb.backTopRight;

	 frontTopLeft     = inCb.frontTopLeft;
	 frontBottomLeft  = inCb.frontBottomLeft ;
	 frontBottomRight = inCb.frontBottomRight;
	 frontTopRight    = inCb.frontTopRight;
}
/////////////////////////////////////////////////////////////////////////////////
//Function
int getIntFromFloat(float inFltVal) {
	CString str;
	str.Format("%f", inFltVal);

	return atoi(str);
}
#endif