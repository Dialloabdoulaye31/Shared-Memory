/**************************************************************************
*
* Title: xb3stereo
* Copyright: (C) 2006,2007,2008 Don Murray donm@ptgrey.com
*
* Description:
*
* Get an image set from a Bumblebee or Bumblebee2 via DMA transfer
* using libdc1394 and process it with the Triclops stereo
* library. Based loosely on 'grab_gray_image' from libdc1394 examples.
*
*-------------------------------------------------------------------------
* License: LGPL
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
*************************************************************************/

//=============================================================================
// Copyright © 2006,2007,2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information"). You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with Point Grey Research Inc.
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//
//=============================================================================

//=============================================================================
//
// server.cpp
//
//=============================================================================
// includes memoria compartida
#include <sys/shm.h>
#include <iostream>

//=============================================================================
// System Includes
//=============================================================================
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <dc1394/conversions.h>
#include <dc1394/control.h>
#include <dc1394/utils.h>


//=============================================================================
// PGR Includes
//=============================================================================
#include "pgr_registers.h"
#include "pgr_stereocam.h"
#include "pnmutils.h"

//=============================================================================


//=============================================================================
/*#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>*/

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/core/core.hpp"


//Size of the shared memory
#define SHMSZ    3686400
//#define SHMSZ    1228800 // 1 image



//using namespace cv;
using namespace std;




//=============================================================================
// a simple function to write a .pgm file
int
writePgm( char* szFilename,
unsigned char* pucBuffer,
int	width,
int	height )
{
  FILE* stream;
  stream = fopen( szFilename, "wb" );
  if( stream == NULL)
  {
     perror( "Can't open image file" );
     return 1;
  }

  fprintf( stream, "P5\n%u %u 255\n", width, height );
  fwrite( pucBuffer, width, height, stream );
  fclose( stream );
  return 0;
}

//=============================================================================
// a simple function to write a .ppm file
int
writePpm( char* szFilename,
unsigned char* pucBuffer,
int	width,
int	height )
{
  FILE* stream;
  stream = fopen( szFilename, "wb" );
  if( stream == NULL)
  {
     perror( "Can't open image file" );
     return 1;
  }

  fprintf( stream, "P6\n%u %u 255\n", width, height );
  fwrite( pucBuffer, 3*width, height, stream );
  fclose( stream );
  return 0;
}


//=============================================================================
// cleanup_and_exit()
// This is called when the program exits and destroys the existing connections
// to the 1394 drivers
void
cleanup_and_exit( dc1394camera_t* camera )
{
   dc1394_capture_stop( camera );
   dc1394_video_set_transmission( camera, DC1394_OFF );
   dc1394_camera_free( camera );
   exit( 0 );
}



//=============================================================================
// MAIN
//
int main( int argc, char *argv[] )
{

 dc1394camera_t* 	camera;
   dc1394error_t 	err;
  
  
 
 



//=============================================================================

key_t keyy;
    int shmid, i, j;
    char* shm;
    char* s;


 char ch;	

//Segment name
    keyy = 888;

    //Create the segment
    if ((shmid = shmget(keyy, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");

/*cout << "No consigo Id para memoria compartida" << endl;*/
		
        exit(1);
    }
    


// Attach the segment to the data space.
                if ((shm = (char*) shmat(shmid, NULL, 0)) == (char *) - 1) {

                 perror("shmat");

   /*cout << "No consigo memoria compartida" << endl;*/

                    exit(1);
                }








  
//===================================================================

   // Find cameras on the 1394 buses
dc1394_t * d;
   dc1394camera_list_t * list;
   unsigned int nThisCam;


   d = dc1394_new ();

     // Enumerate cameras connected to the PC
   err = dc1394_camera_enumerate (d, &list);

   if ( err != DC1394_SUCCESS )
   {
      fprintf( stderr, "Unable to look for cameras\n\n"
"Please check \n"
" - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded \n"
" - if you have read/write access to /dev/raw1394\n\n");
      return 1;
   }
   
   // get the camera nodes and describe them as we find them
   if (list->num == 0)	
   {
      fprintf( stderr, "No cameras found!\n");
      return 1;
   }
   


   printf( "There were %d camera(s) found attached to your PC\n", list->num );

    // Identify cameras. Use the first stereo camera that is found
    for ( nThisCam = 0; nThisCam < list->num; nThisCam++ )
    {
        camera = dc1394_camera_new(d, list->ids[nThisCam].guid);

        if(!camera)
        {
            printf("Failed to initialize camera with guid %llx", list->ids[nThisCam].guid);
            continue;
}	

printf( "Camera %d model = '%s'\n", nThisCam, camera->model );

        if ( isStereoCamera(camera))
        {

printf( "Using this camera\n" );
break;
}

dc1394_camera_free(camera);
   }

   if ( nThisCam == list->num )
   {
      printf( "No stereo cameras were detected\n" );
      return 0;
   }
   

   PGRStereoCamera_t stereoCamera;

   // query information about this stereo camera
   err = queryStereoCamera( camera, &stereoCamera );
   if ( err != DC1394_SUCCESS )
   {
      fprintf( stderr, "Cannot query all information from camera\n" );
      cleanup_and_exit( camera );
   }
   
   if ( stereoCamera.model != BUMBLEBEEXB3 )
   {
      fprintf( stderr, "Stereo camera found was not a BB XB3\n" );
      cleanup_and_exit( camera );
   }

   // override the default nBytesPerPixel to be 3, because we really do
   // want to run at 3 camera mode
   stereoCamera.nBytesPerPixel = 3;

   // set the capture mode
   printf( "Setting stereo video capture mode\n" );
   err = setStereoVideoCapture( &stereoCamera );
   if ( err != DC1394_SUCCESS )
   {
      fprintf( stderr, "Could not set up video capture mode\n" );
      cleanup_and_exit( stereoCamera.camera );
   }

   // have the camera start sending us data
   printf( "Start transmission\n" );
   err = startTransmission( &stereoCamera );
   if ( err != DC1394_SUCCESS )
   {
      fprintf( stderr, "Unable to start camera iso transmission\n" );
      cleanup_and_exit( stereoCamera.camera );
   }
   
  

 //===================================================================
   // Allocate all the buffers.
   // Unfortunately color processing is a bit inefficient because of the number of
   // data copies.  Color data needs to be
   // - de-interleaved into separate bayer tile images
   // - color processed into RGB images
   // - de-interleaved to extract the green channel for stereo (or other mono conversion)

   // size of buffer for all images at mono8
   unsigned int   nBufferSize = stereoCamera.nRows *
                                stereoCamera.nCols *
                                stereoCamera.nBytesPerPixel;
   // allocate a buffer to hold the de-interleaved images
   unsigned char* pucDeInterlacedBuffer = new unsigned char[ nBufferSize ];
   unsigned char* pucRGBBuffer = NULL;;
   unsigned char* pucGreenBuffer = NULL;

    TriclopsInput pShortInput;
   TriclopsInput pWideInput;

   //===================================================================
   //
   //
   // MAIN PROCESSING LOOP
   //
   //
   
 /*while(1)
{*/
   if ( stereoCamera.bColor )
   {
      unsigned char* pucRGBBuffer 	= new unsigned char[ 3 * nBufferSize ];
      unsigned char* pucGreenBuffer 	= new unsigned char[ nBufferSize ];
      unsigned char* pucRightRGB	= NULL;
      unsigned char* pucLeftRGB		= NULL;
      unsigned char* pucCenterRGB	= NULL;

      // get the images from the capture buffer and do all required processing
      // note: produces a TriclopsInput that can be used for stereo processing


      extractImagesColorXB3( &stereoCamera,
			  DC1394_BAYER_METHOD_NEAREST,
			  pucDeInterlacedBuffer,
			  pucRGBBuffer,
			  pucGreenBuffer,
			  &pucRightRGB,
			  &pucLeftRGB,
			  &pucCenterRGB,
			  &pShortInput,
			  &pWideInput);

   /*   if ( !writePpm( "right.ppm", pucRightRGB, stereoCamera.nCols, stereoCamera.nRows ) )
	 printf( "wrote right.ppm\n" );
       if ( !writePpm( "center.ppm", pucCenterRGB, stereoCamera.nCols, stereoCamera.nRows ) ) //added by abdoulaye
	 printf( "wrote center.ppm\n" );
       if ( !writePpm( "left.ppm", pucLeftRGB, stereoCamera.nCols, stereoCamera.nRows ) )
	 printf( "wrote left.ppm\n" );
 */
   

   }
  
 else
   {
      unsigned char* pucRightMono	= NULL;
      unsigned char* pucLeftMono	= NULL;
      unsigned char* pucCenterMono	= NULL;
      // get the images from the capture buffer and do all required processing
      // note: produces a TriclopsInput that can be used for stereo processing
      extractImagesMono( &stereoCamera,
			  pucDeInterlacedBuffer,
			  &pucRightMono,
			  &pucLeftMono,
			  &pucCenterMono,
			  &pShortInput );

  /*
      if ( !writePgm( "right.pgm", pucRightMono, stereoCamera.nCols, stereoCamera.nRows ) )
	 printf( "wrote right.pgm\n" );
      if ( !writePgm( "left.pgm", pucLeftMono, stereoCamera.nCols, stereoCamera.nRows ) )
	 printf( "wrote left.pgm\n" );

   */
   


}




//*****************************************************************************************************//


      // do stereo processing
   TriclopsError e;
   TriclopsContext triclops;

  /* printf( "Getting TriclopsContext from camera (slowly)... \n" );*/
   e = getTriclopsContextFromCamera( &stereoCamera, &triclops );
   if ( e != TriclopsErrorOk )
   {
      fprintf( stderr, "Can't get context from camera\n" );
      cleanup_and_exit( camera );
      return 1;
   }



/**/  // Cambiamos la resolución que viene por defecto 
  
 e = triclopsSetResolution(triclops, 960 ,1280);
   if ( e != TriclopsErrorOk )
   {
      fprintf( stderr, "Can't set context \n" );
      cleanup_and_exit( camera );
      return 1;
   }


/**/


 

/**/   // Trying to set an image buffer

/*unsigned char* image_center= new unsigned char[ 1280 * 960 ];
e=triclopsSetImageBuffer(triclops,image_center,TriImg_RECTIFIED,TriCam_TOP);
 if ( e != TriclopsErrorOk )
   {
      fprintf( stderr, "Can't set buffer \n" );
      cleanup_and_exit( camera );
      return 1;
   }*/

/**/

 


 /* printf( "...done\n" );    */

 

  // make sure we are in subpixel mode
   triclopsSetSubpixelInterpolation( triclops, 1 );
   e = triclopsRectify( triclops, &pShortInput );
   if ( e != TriclopsErrorOk )
   {
      fprintf( stderr, "triclopsRectify failed!\n" );
      triclopsDestroyContext( triclops );
      cleanup_and_exit( camera );
      return 1;
   }

   e = triclopsStereo( triclops );
   if ( e != TriclopsErrorOk )
   {
      fprintf( stderr, "triclopsStereo failed!\n" );
      triclopsDestroyContext( triclops );
      cleanup_and_exit( camera );
      return 1;
   }





 //===================================================================


   // get and save the rectified and disparity images
   TriclopsImage image;
  TriclopsImage image_right, image_middle, image_left;


   e = triclopsGetImage( triclops, TriImg_RECTIFIED, TriCam_RIGHT, &image_left);
   if ( e != TriclopsErrorOk )
   {
      fprintf( stderr, "triclopsGetImage LEFT failed!\n" );
      triclopsDestroyContext( triclops );
      cleanup_and_exit( camera );
      return 1;
   }

  IplImage* IRGB_left=cvCreateImage(cvSize(image_left.ncols,image_left.nrows),8,image_left.rowinc/image_left.ncols);
  memcpy(IRGB_left->imageDataOrigin,image_left.data,image_left.rowinc*image_left.nrows); 
  triclopsSaveImage( &image_left, "Left_rectified.pgm" );




   e = triclopsGetImage( triclops, TriImg_RECTIFIED, TriCam_LEFT, &image_middle);
   if ( e != TriclopsErrorOk )
   {
      fprintf( stderr, "triclopsGetImage MIDDLE failed!\n" );
      triclopsDestroyContext( triclops );
      cleanup_and_exit( camera );
      return 1;
   }

  IplImage* IRGB_middle=cvCreateImage(cvSize(image_middle.ncols,image_middle.nrows),8,image_middle.rowinc/image_middle.ncols);
  memcpy(IRGB_middle->imageDataOrigin,image_middle.data,image_middle.rowinc*image_middle.nrows); 
  triclopsSaveImage( &image_middle, "Middle_rectified.pgm" );



   e = triclopsRectify( triclops, &pWideInput );
   if ( e != TriclopsErrorOk )
   {
      fprintf( stderr, "triclopsRectify failed!\n" );
      triclopsDestroyContext( triclops );
      cleanup_and_exit( camera );
      return 1;
   }

   e = triclopsStereo( triclops );
   if ( e != TriclopsErrorOk )
   {
      fprintf( stderr, "triclopsStereo failed!\n" );
      triclopsDestroyContext( triclops );
      cleanup_and_exit( camera );
      return 1;
   }




   e = triclopsGetImage( triclops, TriImg_RECTIFIED, TriCam_LEFT, &image_right);
   if ( e != TriclopsErrorOk )
   {
      fprintf( stderr, "triclopsGetImage RIGHT failed!\n" );
      triclopsDestroyContext( triclops );
      cleanup_and_exit( camera );
      return 1;
   }

  IplImage* IRGB_right=cvCreateImage(cvSize(image_right.ncols,image_right.nrows),8,image_right.rowinc/image_right.ncols);
  memcpy(IRGB_right->imageDataOrigin,image_right.data,image_right.rowinc*image_right.nrows); 
  triclopsSaveImage( &image_right, "Right_rectified.pgm" );


   




TriclopsImage16 image16;

triclopsGetImage16( triclops, TriImg16_DISPARITY, TriCam_REFERENCE, &image16 );

triclopsGetImage16( triclops, TriImg16_DISPARITY, TriCam_RIGHT, &image16 );
 //triclopsSaveImage16( &image16, "Right_disparity.pgm" );
triclopsGetImage16( triclops, TriImg16_DISPARITY, TriCam_TOP, &image16 );
 //triclopsSaveImage16( &image16, "Center_disparity.pgm" );
triclopsGetImage16( triclops, TriImg16_DISPARITY, TriCam_LEFT, &image16 );
  // triclopsSaveImage16( &image16, "Left_disparity.pgm" );
  // printf( "wrote 'disparity.pgm'\n" );   


 



//===================================================================



//Write data on the shared memory
                s = shm;


for (int k=0; k<(1280*960); k++)
{
 s[k]=IRGB_right->imageData[k];
s[k+1280*960]= IRGB_middle->imageData[k];
s[k+1280*960*2]=IRGB_left->imageData[k];

//s[k]=IRGB_right->imageDataOrigin[k];
//s[k+1280*960]=IRGB_middle->imageDataOrigin[k];
//s[k+1280*960*2]=IRGB_left->imageDataOrigin[k];
   
}




 // display, uncomment if you want to see the images!

 

       /*   cvShowImage("IRGB_right",IRGB_right);
              cvShowImage("IRGB_middle",IRGB_middle);
              cvShowImage("IRGB_left",IRGB_left);

//cout << " filas " << image_right.nrows<<endl;
//cout << "colum" <<image_right.ncols<<endl;
//cout << "div" <<image_right.rowinc/image_right.ncols<<endl;
             
//cout << "rowinc" <<image_right.rowinc<<endl;
              cvWaitKey(0);
*/
             /* cvReleaseImage(&IRGB_right);
              cvReleaseImage(&IRGB_middle);
              cvReleaseImage(&IRGB_left);*/







//********************************** Comprobamos que funciona memoria compartida**************************//

/*for (char c = 'a'; c <= 'z'; c++)
        *s++ = c;
    *s = NULL;

    /*
     * Finally, we wait until the other process 
     * changes the first character of our memory
     * to '*', indicating that it has read what 
     * we put there.
     */
   /* while (*shm != '*')
        sleep(1);

    exit(0);*/

//******************************************************************************************************//



          		


/*} /***************///  End while





 //===================================================================


  printf( "Stop transmission\n" );
   //  Stop data transmission
   if ( dc1394_video_set_transmission( stereoCamera.camera, DC1394_OFF ) != DC1394_SUCCESS )
   {
      fprintf( stderr, "Couldn't stop the camera?\n" );
   }


   delete[] pucDeInterlacedBuffer;
   if ( pucRGBBuffer )
      delete[] pucRGBBuffer;
   if ( pucGreenBuffer )
      delete[] pucGreenBuffer;

   // close camera
   cleanup_and_exit( camera );



 //===================================================================


// Liberamos la memoria

if (((shmid = shmget(keyy, SHMSZ, 0666)) < 0)!=-1) {

shmdt ((char *)shm);
	
}




   return 0;
}
