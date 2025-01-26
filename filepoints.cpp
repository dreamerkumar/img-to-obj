#include "stdafx.h"
#include "filepoints.h"

IMPLEMENT_SERIAL(filePoints, CObject, 1)

///////////////////////////////////////////////////////////////////////////////////////////////

//default constructor
filePoints::filePoints() { 
	cuboidWidth = cuboidHeight = cuboidThickness = 0;
	error = FILE_POINTS_CONST + FILE_NOT_OPEN;
	//fp.m_hFile = NULL;
	g_btXAxisData = NULL;
}

//main constructor
filePoints::filePoints(CString inStrCuboidLocatn) {
	
	g_btXAxisData = NULL;

	//open the existing file
	if(!fp.Open(inStrCuboidLocatn, CFile::modeReadWrite | CFile::shareExclusive)) {
		//set the flag to false
		error = FILE_POINTS_CONST + FILE_ACCESS_ERROR;			
		//fp.m_hFile = NULL;
		return ;
	} 
	
	if(fp.GetLength() <= (UINT)(MOLD_HEADER_SIZE) ) {
		    error = FILE_POINTS_CONST + FILE_SIZE_DOES_NOT_MATCH;
			fp.Close();
			//fp.m_hFile = NULL;
			return ;
	} 
	
	error = SUCCESS;
}

void filePoints::retrieveValues(TARGETCUBOID &inT) {

	/////////////////////////Variable Declaration////////////////////////////////////////
	UINT totalBits, rem;
	////////////////////////////////////////////////////////////////////////////////////

	//Read the values from the file 
	fp.SeekToBegin();

	inT.uintXPoints = readInt();
	inT.uintYPoints = readInt();
	inT.uintZPoints = readInt();
	
	inT.fltMinx = readFloat();	
	inT.fltMaxx = readFloat();
	inT.fltMiny = readFloat();
	inT.fltMaxy = readFloat();
	inT.fltMinz = readFloat();
	inT.fltMaxz = readFloat();

	BYTE snapSlideOptn = read();
	if(snapSlideOptn != 1) {
		error = FILE_POINTS_RETRIEVE_VALUES + MOLD_TYPE_NOT_FOR_SNAPS;
		fp.Close();
		//fp.m_hFile = NULL;
		return ;	
	}

	/*~~~~~~~~~~~~set the cuboid values~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	cuboidWidth = inT.uintXPoints; cuboidHeight = inT.uintYPoints; cuboidThickness = inT.uintZPoints;

	//find the size of the file in bits
	totalBits = cuboidWidth * cuboidHeight * cuboidThickness; 
	
	//increase the value to make it divisible by 8
	rem = totalBits%8 ;
	if(rem != 0 )
		totalBits = totalBits + (8 - rem);
	
	//find the total number of bytes required to store the points
	totalBytes = totalBits/8;
	
	if(fp.GetLength() != (UINT)(MOLD_HEADER_SIZE + totalBytes) ) {
		error = FILE_POINTS_RETRIEVE_VALUES + FILE_SIZE_DOES_NOT_MATCH;
		fp.Close();
		//fp.m_hFile = NULL;
		return ;

	} 

	UINT uintBytesAlongX = (inT.uintXPoints - inT.uintXPoints%8)/8 + 3;	
	
	try {
		//Allocate memory for storing points along X axis temporarily		
		g_btXAxisData = (BYTE *)malloc(uintBytesAlongX); 
		if(g_btXAxisData == NULL) {
				error = FILE_POINTS_RETRIEVE_VALUES + MEMORY_ALLOCATION_FAILURE;
				fp.Close();
		}
		
	
	} catch ( CException *e) {
		g_btXAxisData = NULL;
		e->Delete();
		error = FILE_POINTS_RETRIEVE_VALUES + MEMORY_ALLOCATION_FAILURE;
		fp.Close();
		//fp.m_hFile = NULL;
		return;
	}
	
	if(g_btXAxisData == NULL) {
		error = FILE_POINTS_RETRIEVE_VALUES + MEMORY_ALLOCATION_FAILURE;		
		fp.Close();
		//fp.m_hFile = NULL;
		return;
	}
	
	error = SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
UINT filePoints::setPoint(UINT inX, UINT inY, UINT inZ) {


	/*~~~~~~~~~Variable declaration~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	UINT bytePos, zeroBitPos ;
	BYTE readByte, byteToWrite ;
	UINT uintReturnCode;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	uintReturnCode = getBitBytePos(inX, inY, inZ, zeroBitPos, bytePos);
	
	if (uintReturnCode != SUCCESS)//error occured in the constructor
		return uintReturnCode;	
	
	
	fp.Seek((bytePos - 1), CFile::begin);//If we want to read the nth byte then we
							//need to bypass n-1 bytes
	
	//get the byte from the file
	readByte = read();

	//set the appropriate bit to zero to make it invalid 
	switch(zeroBitPos) {
	case 8 :
		byteToWrite = 254;
		break;
	case 7 :
		byteToWrite = 253;
		break;
	case 6 :
		byteToWrite = 251;
		break;
	case 5 :
		byteToWrite = 247;
		break;
	case 4 :
		byteToWrite = 239;
		break;
	case 3 :
		byteToWrite = 223;
		break;
	case 2 :
		byteToWrite = 191;
		break;
	case 1 :
		byteToWrite = 127;
		break;
	}

	byteToWrite = readByte & byteToWrite ; //the particular bit pos will be set to zero
										   // while the other bits will remain as they are	
	if(readByte != byteToWrite) {
		//move back to the previous position
		fp.Seek(-1, CFile::current);

		//write to the file
		write(byteToWrite);
	}
	
	return SUCCESS;
}

UINT filePoints::setPointRanges(UINT inUintX1, UINT inUintX2, UINT inUintY, UINT inUintZ)  {

	if(inUintX1 == inUintX2) 
		return setPoint(inUintX1, inUintY, inUintZ);

	//Temporary code :~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//for(UINT uintCtr = inUintX1; uintCtr <= inUintX2; uintCtr++) 		
	//	setPoint(uintCtr, inUintY, inUintZ);
	//return SUCCESS;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	UINT uintBitPos1, uintBytePos1, uintBitPos2, uintBytePos2;
	
	//Get the position of the bits and the bytes
	UINT uintReturnCode = getBitBytePos(inUintX1, inUintY, inUintZ, uintBitPos1, uintBytePos1);
	
	if (uintReturnCode != SUCCESS)//error occured in the constructor
		return uintReturnCode;	
	
	uintReturnCode = getBitBytePos(inUintX2, inUintY, inUintZ, uintBitPos2, uintBytePos2);
	
	if (uintReturnCode != SUCCESS)//error occured in the constructor
		return uintReturnCode;	
	
	
	fp.Seek((uintBytePos1 -1), CFile::begin);//if we want to read the nth byte then we
							//need to bypass n-1 bytes
	
	UINT uintTotBytes = uintBytePos2 + 1 - uintBytePos1; //The total number of bytes traversed
	
	if(fp.Read(g_btXAxisData, uintTotBytes) != uintTotBytes)
		throw new CException();

	if(uintBytePos1 == uintBytePos2) {//All bits lie in a single byte

		//Set the appropriate bits in the single byte
		if(uintBitPos1 == 1 && uintBitPos2 == 8)
			*g_btXAxisData = 0;
		else
			setInvalidBits(*g_btXAxisData, uintBitPos1, uintBitPos2);
	} else { 
		
		//Set the bits of the first byte
		if(uintBitPos1 == 1)
			*g_btXAxisData = 0;
		else
			setInvalidBits(*g_btXAxisData, uintBitPos1, 8);		
		
		//Set all the bytes that lie in between the first and the last byte to zero
		if(uintTotBytes > 2)
			memset((g_btXAxisData+1), 0, uintTotBytes -2);
		
		//Set the bits of the last byte
		if(uintBitPos2 == 8)
			*(g_btXAxisData + uintTotBytes - 1) = 0;
		else
			setInvalidBits(*(g_btXAxisData + uintTotBytes - 1), 1, uintBitPos2);				
	}

	fp.Seek(-(int)uintTotBytes, CFile::current);//Move back to the original position to write
												//the byte				
	
	//Write the bits back to the file
	fp.Write(g_btXAxisData, uintTotBytes);

	return SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the particular bits in the bytes to zero
//The start and the end positions are defined from left to right
void filePoints::setInvalidBits(BYTE &inOutBtTarget, BYTE btStartBit, BYTE btEndBit) {
	
	VERIFY(btStartBit > 0 && btStartBit < 9 && btEndBit > 0 && btEndBit < 9 
		&& btStartBit <= btEndBit);

	BYTE btMyByte = 255;
	
	BYTE btSubtract[8] = {128, 64, 32, 16, 8, 4, 2, 1};

	for(BYTE btCtr = btStartBit; btCtr <= btEndBit; btCtr++) 
		btMyByte-= btSubtract[btCtr-1];	

	inOutBtTarget = inOutBtTarget & btMyByte;

}

///////////////////////////////////////////////////////////////////////////////////////////////
//Description: Returns the corresponding byte for a particular bit.
UINT filePoints::getBitBytePos(UINT inUintX, UINT inUintY, UINT inUintZ, 
							   UINT &outUintBitPos, UINT &outUintBytePos) {
	
	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~DESCRIPTION OF FILE FORMAT~~~~~~~~~~~~~~~~~~~~~*/
	/*The way points are stored in the file:
	The points are stored the way the cuboid exists in the 
	coordinate system. First the point (1,1,1) is stored. Then we
	increase x and store points till (cuboidWidth, 1, 1).We repeat
	this series from y index 1 to cuboidHeight. So the first plane
	at z =1 is stored. We then store bits for z = 2 and so on till
	cuboidThickness.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	if (!(error == SUCCESS))//error has occured before
		return error;
	else if(inUintX <= 0 || inUintY <= 0 || inUintZ <= 0 )
		return FILE_POINTS_SET_POINT + NEGATIVE_VALUES;	
	
	//Find the position in terms of bits
	
	UINT uintBits	= ( (inUintZ - 1) * cuboidWidth * cuboidHeight ) 
			+ ( (inUintY - 1) * cuboidWidth  )
			+ inUintX; 
	
	outUintBitPos = uintBits%8; 
	
	if(outUintBitPos == 0 )
		outUintBitPos = 8;
	else
		uintBits = uintBits + 8 - outUintBitPos ;//if it is not divisible by 8 then 
		//we make it so, by suitable addition 
	
    //Find the byte pos
	outUintBytePos = uintBits/8; 
	
	if(outUintBytePos > totalBytes) 
		return FILE_POINTS_SET_POINT + BYTE_POS_EXCEEDED ;
	
	outUintBytePos += MOLD_HEADER_SIZE ; //add MOLD_HEADER_SIZE to bypass the cuboid 
					//information stored at the beginning of the file
	return SUCCESS;	
}
///////////////////////////////////////////////////////////////////////////////////////////////

BYTE filePoints::read() {
	
	BYTE outByte;
	CArchive ar (&fp, CArchive::load);
	ar >> outByte;
	return outByte;
}

///////////////////////////////////////////////////////////////////////////////////////////////

UINT filePoints::readInt() {
	
	UINT outVal;
	CArchive ar (&fp, CArchive::load);
	ar >> outVal;
	return outVal;
}
///////////////////////////////////////////////////////////////////////////////////////////////
float filePoints::readFloat() {
	float fltOutVal;
	CArchive ar (&fp, CArchive::load);
	ar >> fltOutVal;
	return fltOutVal;

}
///////////////////////////////////////////////////////////////////////////////////////////////

void filePoints::write(UINT inVal)	{
		
	CArchive ar(&fp, CArchive::store);
	ar << inVal;			
}
///////////////////////////////////////////////////////////////////////////////////////////////

void filePoints::write(float inVal)	{
		
	CArchive ar(&fp, CArchive::store);
	ar << inVal;			
}

///////////////////////////////////////////////////////////////////////////////////////////////

void filePoints::write(BYTE inByte)	{
		
	CArchive ar(&fp, CArchive::store);
	ar << inByte;			
}

///////////////////////////////////////////////////////////////////////////////////////////////

UINT filePoints::checkError() { 
	return error ;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////updateImageCount: Increases the stored count of images processed by mold file by 1//////

void filePoints::updateImageCount() {
		
	UINT imgCount;
	fp.Seek(MOLD_POS_FOR_IMAGE_COUNT, CFile::begin);
	imgCount = readInt();
	imgCount++; //Increment the count by 1

	//move back to the previous position
	fp.Seek(-4, CFile::current);

	//write to the file
	write(imgCount);

}
///////////////////////////////////////////////////////////////////////////////////////////////
filePoints::~filePoints() {
	//if( error != FILE_POINTS_CONST + FILE_NOT_OPEN || 
	//	error != FILE_POINTS_CONST + FILE_ACCESS_ERROR || 
	//	error != FILE_POINTS_CONST + FILE_CREATION_ERROR 
	
	//If the file is open, then we have to close it before exiting
	
	if(fp.m_hFile != (UINT)(-1)) //This the check done in the CFile close file function. 
		fp.Close();              //We make this check to avoid an assertion failure
	
	if(g_btXAxisData != NULL)
		free(g_btXAxisData); 
}
////////////////////////////////// END ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////