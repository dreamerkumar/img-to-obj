#ifndef _ERRORCONSTANTS_H_
#define _ERRORCONSTANTS_H_ 
#define FAILURE 	  						 1
#define SUCCESS								 0
//Vishal 11-Jan-2004 ( interchanged the values for success and failure)
#define DIVISION_BY_ZERO					10
#define LOOKING_VECTOR_NULL					11 
#define FILE_CREATION_ERROR					12
#define NEGATIVE_VALUES						13
#define FILE_NOT_OPEN						14 
#define BYTE_POS_EXCEEDED					15 
#define IMAGE_SHOT_OUT_OF_TARGET			16
#define NULL_DIMENSION_FOR_CUBOID			17
#define CAMERA_AT_INFINITY_INSIDE_CUBOID	18
#define INVALID_RANGE_AT_INFINITY_VALUES    19
#define FILE_ACCESS_ERROR					20
#define FILE_SIZE_DOES_NOT_MATCH			21
#define FILE_PARAMS_DO_NOT_MATCH            22
#define VALUE_EXCEEDED						23
#define INSUFFICIENT_POINTS					24
#define INDEX_OUT_OF_RANGE                  25
#define VERTICES_NOT_THREE_FOR_TRIANGLE     26
#define	MOLD_TYPE_NOT_FOR_SNAPS			    27
//Additions for the slide option
#define MEMORY_ALLOCATION_FAILURE		    28
#define MOLD_TYPE_NOT_FOR_SLIDES			29
#define INVALID_IMAGE_DIMENSIONS			30
#define NOT_THE_FIRST_SLIDE					31
#define INVALID_NEW_XY_POINTS				32

//Additions on the 22nd feb, 04
#define NO_POLYGONS_GENERATED				33
#define	NO_MORE_SLIDES_POSSIBLE				34

//Addition on 19-Sep-04
#define INVALID_SECURITY_STRING				99


#define SLIDESTOOBJECT_ADDSLIDE						100000

#define POINTS_TO_POLYGONS_CONST	                  1000 
#define POINTS_TO_POLYGONS_PROCESS	                  2000 
#define POINTS_TO_POLYGONS_PROCESS_OBJECT	          3000 
#define POINTS_TO_POLYGONS_IS_VALID_CUBE	          4000 
#define POINTS_TO_POLYGONS_IS_CUBE_POSSIBLE	          5000 
#define POINTS_TO_POLYGONS_GET_CUBE	                  6000 
#define POINTS_TO_POLYGONS_STORE_TRIANGLE_TO_FILE	  7000 
#define POINTS_TO_POLYGONS_STORE_TRIANGLE	          8000 
#define POINTS_TO_POLYGONS_DRAW_SURFACES	          9000 
#define IMAGES_TO_OBJECT_CONST						 10000 
#define IMAGES_TO_OBJECT_SET_NEAR_FAR_RECTANGLE		 11000
#define IMAGES_TO_OBJECT_SET_INVALID                 12000
#define IMAGES_TO_OBJECT_PROCESS_IMAGE               13000

#define PLANEDATAPROCESSOR_INITIALIZE				 14000
#define PLANEDATAPROCESSOR_STORESLIDEDATA			 15000

#define FILE_POINTS_CONST			  100
#define FILE_POINTS_SET_POINT         200 
#define FILE_CUBES_CONST			  300	 
#define FILE_POLYGONS_CONST			  400
#define FILE_READ_POINTS_CONST        500
#define FILE_POINTS_RETRIEVE_VALUES	  600
#define FILEPOINTS_INITIALIZE         700				
#define FILEPOINTS_STORESLIDEDATA     800
#define CUBE_STATUS_INIT_CUBE_STATUS  900		

#endif

//10-Feb-04 Added the values for the slidestoobject Dll