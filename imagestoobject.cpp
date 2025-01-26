#include "stdafx.h"
#include "imagestoobject.h"

//Added on 29-Jan-2004
#define FLT_PROXIMITY_CONST 100.0f //The fraction of the inter point distance that should be
//used to obtain the proximity value. 
//Proximity value: Henceforth, even if a point is found
//to be outside of a plane, but not farther than the proximity value, we will assume it to be
//inside of the plane. This has been done so that no point is left out just because of an 
//erroneous float value obtained as a result of a number of arithmetic operations.


//Constructor 
imagesToObject:: imagesToObject(PROCESS_MLD_INFO *inData) {
	
	//Get the handle to the data object
	dataPtr = inData;


	filePts = NULL ; //NULL used while deleting the filePts object
	
	
	try {
		//initialize the filePoints object
		filePts = new filePoints(dataPtr->g_strInputFile);

		
		if(filePts->checkError() != SUCCESS)  {
			error = IMAGES_TO_OBJECT_CONST + filePts->checkError() ;
			return;
		} 

		filePts->retrieveValues(targetCuboid);

	} catch(CException *e) {
		error = IMAGES_TO_OBJECT_CONST + FILE_ACCESS_ERROR;
		e->Delete();
		return;
	}


	

	if(filePts->checkError() != SUCCESS)  {
		error = IMAGES_TO_OBJECT_CONST + filePts->checkError() ;
		return;
	} 

	if( targetCuboid.uintXPoints == 0 ||  targetCuboid.uintYPoints == 0 ||  targetCuboid.uintZPoints == 0 ) {
		error = IMAGES_TO_OBJECT_CONST + NULL_DIMENSION_FOR_CUBOID;
		return;
	}

	/* Modified on 28th Jan 2004: Rather than offsetting the points from the corners, now I
	   am placing them right from one corner to the other. 
	//calculate the distance between two points on the target cuboid
	dX = (targetCuboid.fltMaxx-targetCuboid.fltMinx) /  ((float)targetCuboid.uintXPoints) ;
	dY = (targetCuboid.fltMaxy-targetCuboid.fltMiny) /  ((float)targetCuboid.uintYPoints) ;
	dZ = (targetCuboid.fltMaxz-targetCuboid.fltMinz) /  ((float)targetCuboid.uintZPoints) ;

	//Find the positions of the first cuboid point along the three axes
	xFirst = targetCuboid.fltMinx + dX / 2.0f;
	yFirst = targetCuboid.fltMiny + dY / 2.0f;
	zFirst = targetCuboid.fltMinz + dZ / 2.0f;

	//Find the positions of the last cuboid point along the three axes
	xLast  = targetCuboid.fltMaxx - dX / 2.0f;
	yLast  = targetCuboid.fltMaxy - dY / 2.0f;
	zLast  = targetCuboid.fltMaxz - dZ / 2.0f;
	*/
	
	//Beginnning of modified code=======================================
	dX = (targetCuboid.fltMaxx-targetCuboid.fltMinx) /  ((float)(targetCuboid.uintXPoints -1)) ;
	dY = (targetCuboid.fltMaxy-targetCuboid.fltMiny) /  ((float)(targetCuboid.uintYPoints -1)) ;
	dZ = (targetCuboid.fltMaxz-targetCuboid.fltMinz) /  ((float)(targetCuboid.uintZPoints -1)) ;

	//Find the positions of the first cuboid point along the three axes
	xFirst = targetCuboid.fltMinx;
	yFirst = targetCuboid.fltMiny;
	zFirst = targetCuboid.fltMinz;

	//Find the positions of the last cuboid point along the three axes
	xLast  = targetCuboid.fltMaxx;
	yLast  = targetCuboid.fltMaxy;
	zLast  = targetCuboid.fltMaxz;
	//End of modified code==============================================

	//Store the corners of the cuboid in an array
	//We start from the top left in anticlockwise direction
	arrCuboidFt[0] = FLTPOINT3D(targetCuboid.fltMinx,targetCuboid.fltMaxy,targetCuboid.fltMaxz);

	arrCuboidFt[1] = FLTPOINT3D(targetCuboid.fltMinx,targetCuboid.fltMiny,targetCuboid.fltMaxz);								

	arrCuboidFt[2] = FLTPOINT3D(targetCuboid.fltMaxx,targetCuboid.fltMiny,targetCuboid.fltMaxz);							

	arrCuboidFt[3] = FLTPOINT3D(targetCuboid.fltMaxx,targetCuboid.fltMaxy,targetCuboid.fltMaxz);								

	arrCuboidBk[0] = FLTPOINT3D(targetCuboid.fltMinx,targetCuboid.fltMaxy,targetCuboid.fltMinz);

	arrCuboidBk[1] = FLTPOINT3D(targetCuboid.fltMinx,targetCuboid.fltMiny,targetCuboid.fltMinz);

	arrCuboidBk[2] = FLTPOINT3D(targetCuboid.fltMaxx,targetCuboid.fltMiny,targetCuboid.fltMinz);

	arrCuboidBk[3] = FLTPOINT3D(targetCuboid.fltMaxx,targetCuboid.fltMaxy,targetCuboid.fltMinz);
	
	
	//Added on 29-Jan-2004. See top of the file for details.
	fltProximityValue = __min(dX, __min(dY, dZ))/FLT_PROXIMITY_CONST;
	
	
	error = SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////
UINT imagesToObject::processImage() {

	/////////////////Variable definitions//////////////////////////////////////////////////
	UINT xPixels = dataPtr->imgParams.width - dataPtr->imgParams.leftPadding 
		- dataPtr->imgParams.rightPadding ;
	UINT yPixels = dataPtr->imgParams.height -dataPtr->imgParams.topPadding 
		- dataPtr->imgParams.bottomPadding ;

	UINT i, j; //counter variables
	UINT recLength ; //length of an invalid rectangle in pixels
	RGBCOLOR currentClr; 
	UINT ret ; //used for getting return values from functions
	///////////////////////////////////////////////////////////////////////////////////////
	
	if( error != SUCCESS ) //error occured in the constructor
		return error;
		
	ret = setNearFarRectangle(dataPtr->imgParams.cameraLocation,dataPtr->imgParams.lookingAt, 
		dataPtr->imgParams.flgAtInfinity) ;
	if (ret != SUCCESS)
		return ret ;	
	
	//The pointer to the image is now pointing to the BOTTOM left pixel 
	//We need to move up the area of bottom padding
	
	dataPtr->imgPointer += dataPtr->imgParams.bottomPadding * 
		( dataPtr->imgParams.width *3 + dataPtr->imgParams.extraBytes)  ;
	
	//Assuming that the bitmap has been defined using 24 bit color scheme,each pixel
	//will occupy three bytes. Added to this, there might be some extra bytes after 
	//each horizontal line to align the image on a 32 bit boundary 
	
	recLength = 0 ; //length of the invalid rectangle in terms of number of pixels
	
	UINT uintPixelsProcessed = 0;

	dataPtr->initMaxNumber(xPixels*yPixels);
	
	for( j = 1 ; j<= yPixels; j++) {
				
		for(i = 1 ; i <= xPixels; i++) {		

			uintPixelsProcessed++;
		
			if(dataPtr->isCancelled())
				return FAILURE;
			
			if(i == 1 ) //move forward by the amount of left padding on the image
				dataPtr->imgPointer += dataPtr->imgParams.leftPadding*3 ;
			
			currentClr.red    = dataPtr->imgPointer[0];
			currentClr.green  = dataPtr->imgPointer[1];
			currentClr.blue   = dataPtr->imgPointer[2];
			dataPtr->imgPointer += 3;
			
			if(currentClr == dataPtr->imgParams.InvalidColor) 
				recLength ++;//increase the length of the invalid rectangle if the current 
							 //pixel is invalid	

			if(recLength > 0 ) {
				if(currentClr != dataPtr->imgParams.InvalidColor) {//no more invalid pixel in the sequence					
					ret = setInvalid(i-1-recLength, j, i-1, xPixels, yPixels, dataPtr->imgParams.cameraLocation);
						//i-recLength to move to the start of invalid rectangle and -1 because
						//right now we have moved one pixel ahead of the invalid rectangle
					if (ret != SUCCESS)
						return ret ;						
					recLength = 0 ; 
				} else if ( i == xPixels) {//boundary is reached
					ret = setInvalid(i-recLength, j, i, xPixels, yPixels, dataPtr->imgParams.cameraLocation);
					if (ret != SUCCESS)
						return ret ;
					recLength = 0 ; 
				}
			}			
			dataPtr->setNewPercent(uintPixelsProcessed);
		} //end of a horizontal line
		dataPtr->imgPointer += dataPtr->imgParams.rightPadding*3;
		//bypass the extra pixels used for alignment
		dataPtr->imgPointer += dataPtr->imgParams.extraBytes;		

	}//end of all pixels before the top padding starts
	
	
	//Update the mold file with the new number of files processed. 
	try {
		filePts->updateImageCount();
	} catch(CException *e) {
		e->Delete();
		return IMAGES_TO_OBJECT_PROCESS_IMAGE + FILE_ACCESS_ERROR;
	}
	
	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////

UINT imagesToObject ::setNearFarRectangle(FLTPOINT3D inCameraLocation,FLTPOINT3D inLookingAt, 
									  BOOLEAN inFlgAtInfinity) {
	
	//////////////////Variable Declaration////////////////////////////////////////////////////
	//constants for the plane equation
	float l;
	float m;
	float n;
	float k ; //constant for the plane passing through inCameraLocation
	float kFarPlane; //constant in the far plane equation
	float kNearPlane ; //constant in the near plane equation
			
	float sign; 
	float tempVal; 
	BOOL flgFound; //flag to indicate whether the target cuboid is within the aperture of the
					//image taken
	float maxConst; //Stores the maximum value that should be added to the equation of plane
					//passing through inCameraLocation, so that a far plane is decided  
					//which covers up all the cuboid points to be processed
	UINT i ; //Counter variable
	
	VECTOR vDirection, upDirection, rightDirection;//directions on the far plane
	float dtr, t; //used in calculating distance of midPt from the camera location
	FLTPOINT3D midPt, midPtNear; //the point at the center of the far rectangle
	float xScope, yScope; //half lengths of the area covered by the camera on Far rectangle
	float distance ; //distance of midPt from the camera location
	/////////////////////////////////////////////////////////////////////////////////////////
		
	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~step 1~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Find the equation of the plane passing through the inCameraLocation. Equation is in  */
	/* the form x l + y m + z n + k = 0                                                     */
	
	//get the direction of view
	vDirection = VECTOR(inLookingAt) - VECTOR(inCameraLocation);

	if(vDirection.modulus() == 0 ) 
		return IMAGES_TO_OBJECT_SET_NEAR_FAR_RECTANGLE + LOOKING_VECTOR_NULL ;
	
	//divide by modulus to get the unit vector
	vDirection = vDirection/ vDirection.modulus() ;
	
	//directional cosines of the plane
	l = vDirection.i ;
	m  = vDirection.j ;
	n  = vDirection.k ;
	
	//constant for the equation of plane 
	k = - (inCameraLocation.x*l + inCameraLocation.y*m + inCameraLocation.z*n);


	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~step 2~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Get the equation of the far plane parallel to this plane and enclosing all the       */
	/* Cuboid points and the CameraLocation on the same side                                */
	
	//target cuboid points which are in range should lie on the same side of the plane passing
	//through CameraLocation
	sign = valFromPlaneEqn(inLookingAt, l, m, n, k);
	
	
	//If any of the target cuboid points gives the same sign, then the cuboid has to be 
	//processed, otherwise it lies outside the purview of the taken snapshot
	flgFound = false;

	for ( i= 0; i<= 7; i++) {

		if ( i <= 3 ) 
			tempVal = valFromPlaneEqn( arrCuboidFt[i], l, m, n, k);
		else
			tempVal = valFromPlaneEqn( arrCuboidBk[i-4], l, m, n, k);
			//i-4 is done to traverse from 0 to 3 for back face after traversing through 
			//the front face 
		if (sameSigns( sign,tempVal)) {
			if(flgFound == false) {
				maxConst = tempVal;
				flgFound = true;
			}
			else if ( modulus(maxConst) < modulus(tempVal) ) 
				maxConst = tempVal ;
		}
	}
		
	
	if (flgFound ) 
		kFarPlane = k - maxConst; // constant for the equation of the far plane 
	else 
		return IMAGES_TO_OBJECT_SET_NEAR_FAR_RECTANGLE + IMAGE_SHOT_OUT_OF_TARGET ; 



	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ step 3 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Make the bounding rectangle on the far plane. We need to find out the four        */
	/* directions from the central point on the plane along which the four corner points */
	/* lie. If the view direction would have been along the -ve z axis, then the upward  */
	/* direction would have been the positive direction of y axis and moving towards     */
	/* right would mean moving in the positive direction of x axis.                      */
	

	//check whether the vector is along the x, y or z axis
	if(vDirection.i != 0 && vDirection.j == 0 && vDirection.k == 0 ) { 
		//aligned along the x axis
		upDirection = VECTOR(0.0f,1.0f,0.0f) ;		
	
		if(vDirection.i > 0 ) 
			rightDirection = VECTOR(0.0f,0.0f,1.0f);
		else
			rightDirection = VECTOR(0.0f,0.0f,-1.0f);
	
	} else if(vDirection.j != 0 && vDirection.i == 0 && vDirection.k == 0 ) { 
		//aligned along the y axis
		if(vDirection.j > 0 ) 
			upDirection = VECTOR(0.0f,0.0f,1.0f);
		else
			upDirection = VECTOR(0.0f,0.0f,-1.0f);
		
		rightDirection  = VECTOR(1.0f,0.0f,0.0f);
			
	
	} else if(vDirection.k != 0 && vDirection.j == 0 && vDirection.i == 0 ) { 
		//aligned along the z axis
		upDirection = VECTOR(0.0f,1.0f,0.0f);
					
		if(vDirection.k > 0 ) 
			rightDirection = VECTOR(-1.0f,0.0f,0.0f);
		else
			rightDirection = VECTOR(1.0f,0.0f,0.0f);		
		
	
	} else if(vDirection.i == 0 && vDirection.j != 0 && vDirection.k != 0 ) {
		//In the yz plane
		if(vDirection.k < 0 ) 
			rightDirection = VECTOR( 1.0f, 0.0f, 0.0f) ; 
		else
			rightDirection = VECTOR(-1.0f, 0.0f, 0.0f) ; 
		
		upDirection = rightDirection * vDirection ; 
	
	} else if (vDirection.i !=0 && vDirection.j == 0 && vDirection.k != 0 ) {
		//In the xz plane
		upDirection = VECTOR( 0.0f , 1.0f , 0.0f ) ; 
		
		rightDirection = vDirection * upDirection ; 

	}  else if (vDirection.i !=0 && vDirection.j != 0 && vDirection.k == 0 ) {
		//In the xy plane
		if(vDirection.i > 0 ) 
			rightDirection = VECTOR( 0.0f, 0.0f, 1.0f ) ; 
		else 
			rightDirection = VECTOR( 0.0f, 0.0f,-1.0f ) ; 
			
		upDirection = rightDirection * vDirection ; 

	} else if( vDirection.i !=0 && vDirection.j != 0 && vDirection.k != 0 ) {
		//Not along the axis, neither along the three planes
		
		if( (vDirection.k < 0 && vDirection.i > 0) 
				||  ( vDirection.k > 0 && vDirection.i < 0) ) 
			upDirection = VECTOR(vDirection.i, 0.0f, 0.0f) * vDirection ;
		else 
			upDirection = vDirection * VECTOR(vDirection.i, 0.0f, 0.0f) ;
			
		rightDirection = vDirection * upDirection;
	}	

		
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Step 4~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Find the co-ordinates of the point at which the perpendicular line from the         */
	/* CameraLocation intersects the far plane. Since the point lies on the line it should */
	/* satisfy the following equations : x = x1 + l t; y = y1 + m t; z = z1 + n t;         */
	/* Putting the values for the point in the equation of the plane: xl+ym+zn+kFarPlane=0 */
	/* we can get the value of t.                                                          */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	dtr = l*l + m*m + n*n;
	
	if (dtr == 0 ) 
		return IMAGES_TO_OBJECT_SET_NEAR_FAR_RECTANGLE + DIVISION_BY_ZERO ; 

	t = -(kFarPlane + inCameraLocation.x*l + inCameraLocation.y*m 
					+ inCameraLocation.z*n)/dtr;	

	midPt = FLTPOINT3D(inCameraLocation.x + l*t, inCameraLocation.y + m*t,
				inCameraLocation.z + n*t);
		
	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Step 5~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/*~Adjust the magnitude of the vectors so that they cover the required area in        ~*/
	/*~the far plane																      ~*/
	
	if(inFlgAtInfinity ) {
		
		if(dataPtr->cameraRatio.xRangeAtInfinity <= 0 
			|| dataPtr->cameraRatio.yRangeAtInfinity <= 0 ) 
			return IMAGES_TO_OBJECT_SET_NEAR_FAR_RECTANGLE + INVALID_RANGE_AT_INFINITY_VALUES ;
		
		//Correction done on 28th Jan 2004. The infinite range values should also be 
		//divided by two
		//xScope = dataPtr->cameraRatio.xRangeAtInfinity;
		//yScope = dataPtr->cameraRatio.yRangeAtInfinity;
		xScope = dataPtr->cameraRatio.xRangeAtInfinity/2.0f;
		yScope = dataPtr->cameraRatio.yRangeAtInfinity/2.0f;


	} else {
		distance  = (VECTOR(midPt) - VECTOR(inCameraLocation)).modulus() ;
		xScope    = (distance * dataPtr->cameraRatio.xRatio)/2.0f ;
		yScope    = (distance * dataPtr->cameraRatio.yRatio)/2.0f ; 
	}
	
	if(upDirection.modulus() != 0 && rightDirection.modulus() != 0) { 
			upDirection	   = upDirection    * ( yScope/upDirection.modulus() ) ;
			rightDirection = rightDirection * ( xScope/rightDirection.modulus() ) ;
	} else 
			return IMAGES_TO_OBJECT_SET_NEAR_FAR_RECTANGLE + DIVISION_BY_ZERO ; 
	
	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Step 6~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Using the vector equations and this point, find the corners of the far plane        */
	
	//top left
	arrFarPlane[0].x =  midPt.x + upDirection.i  - rightDirection.i  ;
	arrFarPlane[0].y =  midPt.y + upDirection.j  - rightDirection.j  ;
	arrFarPlane[0].z =  midPt.z + upDirection.k  - rightDirection.k  ;
	//bottom left
	arrFarPlane[1].x =  midPt.x - upDirection.i  - rightDirection.i  ;
	arrFarPlane[1].y =  midPt.y - upDirection.j  - rightDirection.j  ;
	arrFarPlane[1].z =  midPt.z - upDirection.k  - rightDirection.k  ;
	//bottom right
	arrFarPlane[2].x =  midPt.x - upDirection.i  + rightDirection.i  ;
	arrFarPlane[2].y =  midPt.y - upDirection.j  + rightDirection.j  ;
	arrFarPlane[2].z =  midPt.z - upDirection.k  + rightDirection.k  ;
	//top right
	arrFarPlane[3].x =  midPt.x + upDirection.i  + rightDirection.i  ;
	arrFarPlane[3].y =  midPt.y + upDirection.j  + rightDirection.j  ;
	arrFarPlane[3].z =  midPt.z + upDirection.k  + rightDirection.k  ;

	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Step 7~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Find distance of the Near Plane from Camera if it lies outside the cuboid. ~~~~~~~~*/	
	
	if(withinFrustum(inCameraLocation, arrCuboidFt, arrCuboidBk) ) 
		flgCamInCuboid = true ;
	else {
		flgCamInCuboid = false ;

		//find the distance of the near plane from the camera location

		for ( i= 0; i<= 7; i++) {

			if ( i <= 3 ) 
				tempVal = valFromPlaneEqn( arrCuboidFt[i], l, m, n, 
										kFarPlane );
			else
				tempVal = valFromPlaneEqn( arrCuboidBk[i-4], l, m, n, 
										kFarPlane );
			if(i == 0) 
				maxConst = tempVal;
			else if ( modulus(maxConst) < modulus( tempVal) ) 
					maxConst = tempVal ;			
		}
		
		kNearPlane = kFarPlane - maxConst; //constant for the near plane	
		
		dNear = modulus(k - kNearPlane); //distances of near and far plane 
		dFar  = modulus(k - kFarPlane);	 //from the CameraLocation	
	}
	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Step 8~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/*~~~~~ If the object is at infinity then define the corners of the near plane ~~~~~~~*/
	if(inFlgAtInfinity) {
		
		flgCamAtInfinity = true ;
		
		if(flgCamInCuboid )
			return IMAGES_TO_OBJECT_SET_NEAR_FAR_RECTANGLE + CAMERA_AT_INFINITY_INSIDE_CUBOID ;
		
		//get the middle point of the near plane 
		midPtNear = getMiddlePoint(inCameraLocation, midPt, dNear, dFar ) ;

		/* Using the vector equations and this point, find the corners of the near plane  */
	
		//top left
		arrNearPlane[0].x =  midPtNear.x + upDirection.i  - rightDirection.i  ;
		arrNearPlane[0].y =  midPtNear.y + upDirection.j  - rightDirection.j  ;
		arrNearPlane[0].z =  midPtNear.z + upDirection.k  - rightDirection.k  ;
		//bottom left
		arrNearPlane[1].x =  midPtNear.x - upDirection.i  - rightDirection.i  ;
		arrNearPlane[1].y =  midPtNear.y - upDirection.j  - rightDirection.j  ;
		arrNearPlane[1].z =  midPtNear.z - upDirection.k  - rightDirection.k  ;
		//bottom right
		arrNearPlane[2].x =  midPtNear.x - upDirection.i  + rightDirection.i  ;
		arrNearPlane[2].y =  midPtNear.y - upDirection.j  + rightDirection.j  ;
		arrNearPlane[2].z =  midPtNear.z - upDirection.k  + rightDirection.k  ;
		//top right
		arrNearPlane[3].x =  midPtNear.x + upDirection.i  + rightDirection.i  ;
		arrNearPlane[3].y =  midPtNear.y + upDirection.j  + rightDirection.j  ;
		arrNearPlane[3].z =  midPtNear.z + upDirection.k  + rightDirection.k  ;

	} else 
		flgCamAtInfinity = false ;
	
	return SUCCESS; 	
}

///////////////////////////////////////////////////////////////////////////////////////////////

UINT imagesToObject::setInvalid( UINT inX1, UINT inY, UINT inX2,
						UINT inXPixels, UINT inYPixels, FLTPOINT3D inCameraLocation) {
		
	/////////////////Variable definitions//////////////////////////////////////////////////
	
	FLTPOINT3D frstmFt[4]; //end points of the frustum or pyramid
	FLTPOINT3D frstmBk[4];
	
	FLTPOINT3D upFLeft, upFRight, downFLeft, downFRight;//points at the same height on the 
							//far rectangle as that of the corners of the invalid rectangle
	FLTPOINT3D upNLeft, upNRight, downNLeft, downNRight;//points at the same height on the 
							//far rectangle as that of the corners of the invalid rectangle

	float miny, maxy, minz, maxz; //defines the limit within which cuboid points
								//are to be considered	
	UINT y1, z1; //first indexes from where we start traversing the cuboid points
	UINT y2, z2; //Last indexes 
	float y, z ;   //variables used while traversing
	float y1Pos, z1Pos ;
	BOOL flgInside;   //through the cuboid points 
	UINT iy, iz ;  //for invalid check
	UINT ret ; //used to get the return value from function
	///////////////////////////////////////////////////////////////////////////////////////
	

	/*~~~~~~~~~~~~~~~~~~~~~~~ Define the back face on the far rectangle ~~~~~~~~~~~~~~~~~*/

	/*~~~~~~~~~~ First get the heights of the four sides of the invalid rectangle ~~~~~~~*/
	
	
	if(inY == inYPixels) { //top most pixel
		upFLeft  = arrFarPlane[0];
		upFRight = arrFarPlane[3];		
	
	} else {
		upFLeft  = getMiddlePoint(arrFarPlane[1], arrFarPlane[0], inY, inYPixels) ; 
		upFRight = getMiddlePoint(arrFarPlane[2], arrFarPlane[3], inY, inYPixels) ; 
	}
	
	
	
	if(inY == 1) { //inY is one for the bottommost pixel
		downFLeft  = arrFarPlane[1];
		downFRight = arrFarPlane[2];
	
	} else {
		downFLeft  = getMiddlePoint(arrFarPlane[1], arrFarPlane[0], inY-1, inYPixels) ;
		downFRight = getMiddlePoint(arrFarPlane[2], arrFarPlane[3], inY-1, inYPixels) ;
	}	
	
		
	/*~~~~ Now move these points horizontally to make the actual invalid rectangle ~~~*/
	
	if( inX1 == 0 ) {
		frstmBk[0] = upFLeft ;
		frstmBk[1] = downFLeft;
	
	} else {
		frstmBk[0] = getMiddlePoint(upFLeft, upFRight, inX1, inXPixels  ) ;
		frstmBk[1] = getMiddlePoint(downFLeft, downFRight, inX1, inXPixels);
	}

	if( inX2 == inXPixels ) {
		frstmBk[3] = upFRight ;
		frstmBk[2] = downFRight;
	
	} else {
		frstmBk[3] = getMiddlePoint(upFLeft,   upFRight,   inX2, inXPixels );
		frstmBk[2] = getMiddlePoint(downFLeft, downFRight, inX2, inXPixels );
	
	}//end of back face -----------------------------------------------------------------

	
	//Front face 
	if(flgCamAtInfinity) {
			
		/*~~~~ First get the heights of the four sides of the invalid rectangle ~~~*/
				
		if(inY == inYPixels) { //top most pixel
			upNLeft  = arrNearPlane[0];
			upNRight = arrNearPlane[3];		
		
		} else {
			upNLeft  = getMiddlePoint(arrNearPlane[1], arrNearPlane[0], inY, inYPixels) ; 
			upNRight = getMiddlePoint(arrNearPlane[2], arrNearPlane[3], inY, inYPixels) ; 
		}
		
		
		
		if(inY == 1) { //inY is one for the bottommost pixel
			downNLeft  = arrNearPlane[1];
			downNRight = arrNearPlane[2];
		
		} else {
			downNLeft  = getMiddlePoint(arrNearPlane[1], arrNearPlane[0], inY-1, inYPixels) ;
			downNRight = getMiddlePoint(arrNearPlane[2], arrNearPlane[3], inY-1, inYPixels) ;
		}	
		
			
		/*~~~~ Now move these points horizontally to make the actual invalid rectangle ~~~*/
		
		if( inX1 == 0 ) {
			frstmFt[0] = upNLeft ;
			frstmFt[1] = downNLeft;
		
		} else {
			frstmFt[0] = getMiddlePoint(upNLeft, upNRight, inX1, inXPixels  ) ;
			frstmFt[1] = getMiddlePoint(downNLeft, downNRight, inX1, inXPixels);
		}

		if( inX2 == inXPixels ) {
			frstmFt[3] = upNRight ;
			frstmFt[2] = downNRight;
		
		} else {
			frstmFt[3] = getMiddlePoint(upNLeft,   upNRight,   inX2, inXPixels );
			frstmFt[2] = getMiddlePoint(downNLeft, downNRight, inX2, inXPixels );
		
		}

	
	} else if(!flgCamInCuboid) { //camera outside the target cuboid but not at infinity
		
		//use the distance formula
		frstmFt[0] = getMiddlePoint( inCameraLocation, frstmBk[0], dNear, dFar);
		frstmFt[1] = getMiddlePoint( inCameraLocation, frstmBk[1], dNear, dFar);
		frstmFt[2] = getMiddlePoint( inCameraLocation, frstmBk[2], dNear, dFar);
		frstmFt[3] = getMiddlePoint( inCameraLocation, frstmBk[3], dNear, dFar);
	}//-----End of front face -----------------------------------------------------------
		
	
	
	//Calculate the equations of the bounding planes
	if(flgCamInCuboid) {
		
		getPlane(frstmBk[3], frstmBk[2], frstmBk[1], boundingPlanes.backPlane);//back
		getPlane(inCameraLocation, frstmBk[2], frstmBk[3], boundingPlanes.rightPlane);//right
		getPlane(inCameraLocation, frstmBk[3], frstmBk[0], boundingPlanes.topPlane);//top
		getPlane(inCameraLocation, frstmBk[0], frstmBk[1], boundingPlanes.leftPlane);//left
		getPlane(inCameraLocation, frstmBk[1], frstmBk[2], boundingPlanes.bottomPlane);//bottom

	} else {

		 getPlane(frstmFt[0], frstmFt[1], frstmFt[2], boundingPlanes.frontPlane );//front
		 getPlane(frstmBk[3], frstmBk[2], frstmBk[1], boundingPlanes.backPlane );//back
		 getPlane(frstmFt[2], frstmBk[2], frstmBk[3], boundingPlanes.rightPlane );//right
		 getPlane(frstmFt[3], frstmBk[3], frstmBk[0], boundingPlanes.topPlane );//top
		 getPlane(frstmFt[0], frstmBk[0], frstmBk[1], boundingPlanes.leftPlane );//left
		 getPlane(frstmFt[1], frstmBk[1], frstmBk[2], boundingPlanes.bottomPlane );//bottom
	}

	//get the coordinate ranges within which the invalid frustum or pyramid lies
	if(!flgCamInCuboid) {
		//minx = Min(frstmFt[0].x, frstmFt[1].x, frstmFt[2].x, frstmFt[3].x, 
		//		   frstmBk[0].x, frstmBk[1].x, frstmBk[2].x, frstmBk[3].x );
		
	//	maxx = Max(frstmFt[0].x, frstmFt[1].x, frstmFt[2].x, frstmFt[3].x, 
		//		   frstmBk[0].x, frstmBk[1].x, frstmBk[2].x, frstmBk[3].x );
		
		miny = Min(frstmFt[0].y, frstmFt[1].y, frstmFt[2].y, frstmFt[3].y, 
				   frstmBk[0].y, frstmBk[1].y, frstmBk[2].y, frstmBk[3].y );
		
		maxy = Max(frstmFt[0].y, frstmFt[1].y, frstmFt[2].y, frstmFt[3].y, 
				   frstmBk[0].y, frstmBk[1].y, frstmBk[2].y, frstmBk[3].y );
		
		minz = Min(frstmFt[0].z, frstmFt[1].z, frstmFt[2].z, frstmFt[3].z, 
				   frstmBk[0].z, frstmBk[1].z, frstmBk[2].z, frstmBk[3].z );
		
		maxz = Max(frstmFt[0].z, frstmFt[1].z, frstmFt[2].z, frstmFt[3].z, 
				   frstmBk[0].z, frstmBk[1].z, frstmBk[2].z, frstmBk[3].z );
	}
	else {
		//minx = Min(inCameraLocation.x,
		//		   frstmBk[0].x, frstmBk[1].x, frstmBk[2].x, frstmBk[3].x );
		
		//maxx = Max(inCameraLocation.x,
		//		   frstmBk[0].x, frstmBk[1].x, frstmBk[2].x, frstmBk[3].x );
		
		miny = Min(inCameraLocation.y,
				   frstmBk[0].y, frstmBk[1].y, frstmBk[2].y, frstmBk[3].y );
		
		maxy = Max(inCameraLocation.y,
				   frstmBk[0].y, frstmBk[1].y, frstmBk[2].y, frstmBk[3].y );
		
		minz = Min(inCameraLocation.z,
				   frstmBk[0].z, frstmBk[1].z, frstmBk[2].z, frstmBk[3].z );
		
		maxz = Max(inCameraLocation.z,
				   frstmBk[0].z, frstmBk[1].z, frstmBk[2].z, frstmBk[3].z );
	}	
	
	
	//check whether min values lie outside target cuboid
	if( //minx > xLast || maxx < xFirst || 
		miny > yLast || maxy < yFirst || 
		minz > zLast || maxz < zFirst )
			return SUCCESS ; //no points effected
	
	//set the starting and ending values within range of target cuboid
	
	/*Modified on 29-Jan-2004 Using proximity value in the calculations
	//min values
	if(minx <= xFirst) 
		x1 = 1 ; 
	else 
		x1 = 1 + getIntFromFloat ((minx - xFirst) /dX) ;
		
	if(miny <= yFirst) 
		y1 = 1 ;
	else 
		y1 = 1 + getIntFromFloat ((miny - yFirst) /dY) ;
	
	if(minz <= zFirst) 
		z1 = 1 ;
	else 
		z1 = 1 + getIntFromFloat ((minz - zFirst) /dZ) ;
	*/
//	if(minx <= xFirst + fltProximityValue) 
//		x1 = 1 ; 
//	else 
	//	x1 = 1 + getIntFromFloat ((minx - (xFirst + fltProximityValue)) /dX) ;
		
	if(miny <= yFirst) 
		y1 = 1 ;
	else 
		y1 = 1 + getIntFromFloat ((miny - (yFirst + fltProximityValue)) /dY) ;
	
	if(minz <= zFirst) 
		z1 = 1 ;
	else 
		z1 = 1 + getIntFromFloat ((minz - (zFirst + fltProximityValue)) /dZ) ;
	//End of modification==============================================
	
//	x1Pos = xFirst + dX * ( (float)(x1-1) ) ;
	y1Pos = yFirst + dY * ( (float)(y1-1) ) ;
	z1Pos = zFirst + dZ * ( (float)(z1-1) ) ;
	
	/*Modified on 29-Jan-2004 Using proximity value in the calculations
	//max values
	if(maxx >= xLast)
		x2 = targetCuboid.uintXPoints;
	else
		x2 = 1 + getIntFromFloat ((maxx - xFirst) /dX) ;


	if(maxy >= yLast)
		y2 = targetCuboid.uintYPoints ;
	else
		y2 = 1 + getIntFromFloat ((maxy - yFirst) /dY) ;


	if(maxz >= zLast)
		z2 = targetCuboid.uintZPoints ;
	else
		z2 = 1 + getIntFromFloat ((maxz - zFirst) /dZ) ;
	*/
//	if(maxx >= xLast - fltProximityValue)
//		x2 = targetCuboid.uintXPoints ;
//	else
//		x2 = 1 + getIntFromFloat ((maxx - (xFirst - fltProximityValue)) /dX) ;


	if(maxy >= yLast)
		y2 = targetCuboid.uintYPoints ;
	else
		y2 = 1 + getIntFromFloat ((maxy - (yFirst - fltProximityValue)) /dY) ;


	if(maxz >= zLast)
		z2 = targetCuboid.uintZPoints ;
	else
		z2 = 1 + getIntFromFloat ((maxz - (zFirst - fltProximityValue)) /dZ) ;
	//End of modification==============================================	

	UINT uintInvalidPtX1, uintInvalidPtX2; //X1 and X2 specify the starting and ending indices
	//of the range of invalid points found in a horizontal line at a particular y and z value


	FLTPOINT3D startPoint, endPoint, testPoint;
	BYTE btPointsFound, btTotalPlanes, btCtr; 
	PLANE_EQUATION_VALUES curPlane;
	float fltTempPt;

	if(!flgCamInCuboid)
		btTotalPlanes = 6;
	else
		btTotalPlanes = 5;
	
	//traverse through the array of points to set the points as invalid	
	
	for(z = z1Pos, iz = z1; iz<= z2; z+= dZ, iz++ ) { //all pts
		
		//Since the line is along x axis, the y and z value for the point of 
		//intersection will remain the same
		
		testPoint.z = z; 

		for(y = y1Pos, iy = y1; iy<= y2; y+= dY, iy++) { //pts along x-y plane	
			
			if(dataPtr->isCancelled())
				return FAILURE;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//Major change: Checking each point along any axis is taking too much of time which is 
//undesirable. Now onwards, I have decided to change this to find all the invalid points at 
//one go. First we find the points of intersection of the line with the faces of the invalid
//region, such that the points of intersection lie inside the invalid region. We should always
//get two such points if there exists an invalid region for the particular value of y and z.
//
//After identifying the points of intersection, we will set all the points that lie in between
//them as invalid.
				
				
			btPointsFound = 0;
			//Since the line is along x axis, the y and z value for the point of 
			//intersection will remain the same
			testPoint.y = y;
			
			

			//Try the point of intersection with the bounding planes. We should get two such
			//points
			for(btCtr = 1; btCtr <= btTotalPlanes; btCtr++) {
				switch(btCtr) {
				case 1: 
					curPlane = boundingPlanes.leftPlane; 
					break;
				case 2: 
					curPlane = boundingPlanes.rightPlane; 
					break;
				case 3: 
					curPlane = boundingPlanes.topPlane; 
					break;
				case 4: 
					curPlane = boundingPlanes.bottomPlane; 
					break;
				case 5: 
					curPlane = boundingPlanes.backPlane; 
					break;
				case 6: 
					curPlane = boundingPlanes.frontPlane; 
					break;
				}

				if(curPlane.fltL != 0.0f) { //Not parallel to the X axis
			
					//Find the x value of the point of intersection
					testPoint.x = (curPlane.fltM*y 
						+ curPlane.fltN*z + curPlane.fltK)/
						(-curPlane.fltL);

					if(!flgCamInCuboid) 
						flgInside = withinFrustum(testPoint);	
					else
						flgInside = withinPyramid(testPoint);	
					
					if(flgInside && btPointsFound == 1) { //compare for duplicate points
						if( fabs(testPoint.x - startPoint.x ) <= fltProximityValue)
							//The points can be assumed to be the same
							flgInside = false;
					}
					if(flgInside) {	
						btPointsFound++;
						if(btPointsFound == 2)
							endPoint = testPoint;
						else
							startPoint = testPoint;
					}
				}
			
				if(btPointsFound == 2)
					break; //Don't look further if two points are already found
			}	


			if(btPointsFound > 0 ) { //Find invalid points and set their status in the file
				if(btPointsFound == 2) {
					if(endPoint.x < startPoint.x) { //Swap the points
						fltTempPt = endPoint.x;
						endPoint.x = startPoint.x;
						startPoint.x = fltTempPt;
					}
				} else {
					//Just one point has been found
					endPoint = startPoint;
				}

				uintInvalidPtX1 = getClosestPtIndex(startPoint.x, true);
				if(uintInvalidPtX1 == 0)
					continue;

			
				uintInvalidPtX2 = getClosestPtIndex(endPoint.x, false);
				if(uintInvalidPtX2 == 0 || (uintInvalidPtX1 > uintInvalidPtX2))
					continue;
			
	
				try {
					ret = filePts->setPointRanges(uintInvalidPtX1, uintInvalidPtX2, iy, iz); 
												//set the points as invalid in the file
				} catch (CException *e) {
					e->Delete();
					return IMAGES_TO_OBJECT_SET_INVALID + FILE_ACCESS_ERROR;			
				}
				
				if(ret != SUCCESS ) 
					return IMAGES_TO_OBJECT_SET_INVALID + ret;			
			}



		} //y	
	} //z
	
	return SUCCESS ;
}
/////////////////////////////////////////////////////////////////////////////////////////////////

UINT imagesToObject::getClosestPtIndex(float inFltPosition, BOOL blnStartPt) {
	
	float fltDivVal, fltRemainder;
	UINT uintReturnVal;

	if(blnStartPt) {
		if(inFltPosition >= targetCuboid.fltMinx) {
			
			if(inFltPosition > targetCuboid.fltMaxx) 
				uintReturnVal = 0; //No points can be set as invalid
			else {
				fltDivVal = (inFltPosition - targetCuboid.fltMinx)/dX;

				uintReturnVal = getIntFromFloat(fltDivVal);
		
				fltRemainder = fltDivVal - (float)uintReturnVal;

				if(fltRemainder > fltProximityValue) 
					uintReturnVal++;
				
				uintReturnVal++; //All point indices start from one and not from zero
			}
		} else 
			uintReturnVal=1;
	} else {
		if(inFltPosition <= targetCuboid.fltMaxx) {
			if(inFltPosition < targetCuboid.fltMinx) 
				uintReturnVal = 0; //No points can be set as invalid
			else {

				fltDivVal = (inFltPosition - targetCuboid.fltMinx)/dX;

				uintReturnVal = getIntFromFloat(fltDivVal);
		
				fltRemainder = fltDivVal - (float)uintReturnVal;
				
				if((1.0f - fltRemainder) <= fltProximityValue) 
					uintReturnVal++;	

				uintReturnVal++; //All point indices start from one and not from zero
			}
		} else 
			uintReturnVal = targetCuboid.uintXPoints; //Assign the maximum value
	}
	
	return uintReturnVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

float imagesToObject::valFromPlaneEqn(FLTPOINT3D inP, float inC1,float  inC2, 
									  float inC3,float  inK) {

	return inP.x*inC1 + inP.y*inC2 + inP.z*inC3 + inK;

}

/////////////////////////////////////////////////////////////////////////////////////////////////

BOOL imagesToObject::sameSigns(float inVal1, float inVal2 ) {
	if ( inVal1 < 0 && inVal2 < 0 ) 
		return true;
	else if ( inVal1 >= 0 && inVal2 >= 0 ) 
		return true;
	else 
		return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

float imagesToObject::modulus(float inVal) {
	if (inVal < 0 )
		return -inVal;
	else 
		return inVal;
}



/////////////////////////////////////////////////////////////////////////////////////////////////

BOOL imagesToObject::withinFrustum(FLTPOINT3D inP,FLTPOINT3D inArrFt[4],
								   FLTPOINT3D inArrBk[4]) {

	if(!insideOfPlane(inP, inArrFt[0], inArrFt[1], inArrFt[2]) )//front
		return false ;	
	else if (!insideOfPlane(inP, inArrBk[3], inArrBk[2], inArrBk[1]) )//back
		return false ;
	else if (!insideOfPlane(inP, inArrFt[2], inArrBk[2], inArrBk[3]) )//right
		return false ;
	else if (!insideOfPlane(inP, inArrFt[3], inArrBk[3], inArrBk[0]) )//top
		return false ;
	else if (!insideOfPlane(inP, inArrFt[0], inArrBk[0], inArrBk[1]) )//left
		return false ;
	else if (!insideOfPlane(inP, inArrFt[1], inArrBk[1], inArrBk[2]) )//bottom
		return false ;
	else 
		return true ;
}	

/////////////////////////////////////////////////////////////////////////////////////////////////

BOOL imagesToObject::withinFrustum(FLTPOINT3D inP) {

	if(!insideOfPlane(inP, boundingPlanes.frontPlane))//front
		return false;	
	else if (!insideOfPlane(inP, boundingPlanes.backPlane) )//back
		return false;
	else if (!insideOfPlane(inP, boundingPlanes.rightPlane) )//right
		return false;
	else if (!insideOfPlane(inP, boundingPlanes.topPlane) )//top
		return false;
	else if (!insideOfPlane(inP, boundingPlanes.leftPlane) )//left
		return false;
	else if (!insideOfPlane(inP, boundingPlanes.bottomPlane) )//bottom
		return false;
	else 
		return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

BOOL imagesToObject::withinPyramid(FLTPOINT3D inP,FLTPOINT3D inEnd,
								   FLTPOINT3D inArrBk[4]) {
	
	if (!insideOfPlane(inP, inArrBk[3], inArrBk[2], inArrBk[1]) )//back
		return false ;
	else if (!insideOfPlane(inP, inEnd, inArrBk[2], inArrBk[3]) )//right
		return false ;
	else if (!insideOfPlane(inP, inEnd, inArrBk[3], inArrBk[0]) )//top
		return false ;
	else if (!insideOfPlane(inP, inEnd, inArrBk[0], inArrBk[1]) )//left
		return false ;
	else if (!insideOfPlane(inP, inEnd, inArrBk[1], inArrBk[2]) )//bottom
		return false ;
	else 
		return true ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

BOOL imagesToObject::withinPyramid(FLTPOINT3D inP) {
	
	if (!insideOfPlane(inP, boundingPlanes.backPlane))//back
		return false ;
	else if (!insideOfPlane(inP, boundingPlanes.rightPlane))//right
		return false ;
	else if (!insideOfPlane(inP, boundingPlanes.topPlane))//top
		return false ;
	else if (!insideOfPlane(inP, boundingPlanes.leftPlane))//left
		return false ;
	else if (!insideOfPlane(inP, boundingPlanes.bottomPlane))//bottom
		return false ;
	else 
		return true ;

}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL imagesToObject::insideOfPlane(FLTPOINT3D inPoint, 
								   PLANE_EQUATION_VALUES inPlnValues ){
	if((inPoint.x * inPlnValues.fltL + inPoint.y * inPlnValues.fltM + 
		inPoint.z * inPlnValues.fltN + inPlnValues.fltK)<= fltProximityValue ) 
		return true ;
	else 
		return false ;	
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void imagesToObject::getPlane(FLTPOINT3D inP1, FLTPOINT3D inP2, FLTPOINT3D inP3, 
								   PLANE_EQUATION_VALUES &outPlnValues ){
	
	VECTOR normal = ( VECTOR(inP2) - VECTOR(inP1) ) * ( VECTOR(inP3) - VECTOR(inP1) ) ;
	outPlnValues  = PLANE_EQUATION_VALUES(normal.i, normal.j, normal.k, 
		- (inP1.x*normal.i + inP1.y*normal.j + inP1.z* normal.k));
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL imagesToObject::insideOfPlane(FLTPOINT3D inPoint,FLTPOINT3D inP1,FLTPOINT3D inP2,
								   FLTPOINT3D inP3 ){
	
	float l, m, n, k ;
	VECTOR normal = ( VECTOR(inP2) - VECTOR(inP1) ) * ( VECTOR(inP3) - VECTOR(inP1) ) ;
		
	l = normal.i;
	m = normal.j;
	n = normal.k;
	k = - (inP1.x*l + inP1.y*m + inP1.z*n);

	/* Modified on 29-Jan-04. Even if the point is outside but close enough to the plane, 
	//we will consider it to be inside of it.=================================================
	if(valFromPlaneEqn(inPoint, l, m, n, k) <= 0 ) 
		return true ;
	else 
		return false ;
	*/
	if(valFromPlaneEqn(inPoint, l, m, n, k) <= fltProximityValue ) 
		return true ;
	else 
		return false ;
	

	//End of modification=====================================================================
}
/////////////////////////////////////////////////////////////////////////////////////////////////

FLTPOINT3D imagesToObject::getMiddlePoint(FLTPOINT3D inP1, FLTPOINT3D inP2, float inM, 
										  float inMPlusN) {
	return getMiddlePoint(inP1, inP2, inM/(inMPlusN - inM));
	
}


/////////////////////////////////////////////////////////////////////////////////////////////////

FLTPOINT3D imagesToObject::getMiddlePoint(FLTPOINT3D inP1, FLTPOINT3D inP2, UINT inM, 
										  UINT inMPlusN) {
	return getMiddlePoint(inP1, inP2,(float)inM/(float)(inMPlusN - inM) ) ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

FLTPOINT3D imagesToObject::getMiddlePoint(FLTPOINT3D inP1, FLTPOINT3D inP2, float inD1ByD2) {
	
	float m, n;
	m = inD1ByD2 ; 
	n = 1.0f ;	
	return (inP2 * m + inP1 * n ) / (m + n) ;	
}

/////////////////////////////////////////////////////////////////////////////////////////////////

float imagesToObject::Min(float inA, float inB,float inC, 
						  float inD, float inE) {
	float outVal = inA ;
	
	if(outVal > inB)
		outVal = inB;
	
	if(outVal > inC)
		outVal = inC;
	
	if(outVal > inD)
		outVal = inD;
	
	if(outVal > inE)
		outVal = inE;
	
	return outVal;
} 

/////////////////////////////////////////////////////////////////////////////////////////////////

float imagesToObject::Min(float inA, float inB,float inC, 
						  float inD, float inE,float inF, 
						  float inG, float inH) {
	float outVal = inA ;
	
	if(outVal > inB)
		outVal = inB;
	
	if(outVal > inC)
		outVal = inC;
	
	if(outVal > inD)
		outVal = inD;
	
	if(outVal > inE)
		outVal = inE;
	
	if(outVal > inF)
		outVal = inF;
	
	if(outVal > inG)
		outVal = inG;
	
	if(outVal > inH)
		outVal = inH;
	
	return outVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

float imagesToObject::Max(float inA, float inB,float inC, 
						  float inD, float inE) {
	float outVal = inA;

	if(outVal < inB )
		outVal = inB ;

	if(outVal < inC )
		outVal = inC ;

	if(outVal < inD )
		outVal = inD ;

	if(outVal < inE )
		outVal = inE ;

	return outVal ; 
} 

/////////////////////////////////////////////////////////////////////////////////////////////////

float imagesToObject::Max(float inA, float inB,float inC, 
						  float inD, float inE,float inF, 
						  float inG, float inH) {
	float outVal = inA;

	if(outVal < inB )
		outVal = inB ;

	if(outVal < inC )
		outVal = inC ;

	if(outVal < inD )
		outVal = inD ;

	if(outVal < inE )
		outVal = inE ;

	if(outVal < inF )
		outVal = inF ;

	if(outVal < inG )
		outVal = inG ;

	if(outVal < inH )
		outVal = inH ;	
	
	return outVal ; 	
}
//////////////////////////////////////////////////////////////////////////////////////////////
imagesToObject::~imagesToObject() {
	if(filePts != NULL)
		delete filePts ;
}

////////////////////////////////// END ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
