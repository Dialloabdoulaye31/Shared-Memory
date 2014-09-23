//===================================================================

// Triclopsclient.cpp

//===================================================================

 #include <ros/ros.h>
 #include <image_transport/image_transport.h>
 #include <cv_bridge/cv_bridge.h>
 #include <sensor_msgs/image_encodings.h>
 #include <opencv2/imgproc/imgproc.hpp>
 #include <opencv2/highgui/highgui.hpp>
 #include <sensor_msgs/image_encodings.h>


//===================================================================
#include <sys/shm.h>
#include <cstdio>

//===================================================================

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//===================================================================


/*
static const char WINDOW[] = "Image window";*/

//Size of the shared memory
#define SHMSZ     3686400

using namespace std;




//=============================================================================
// MAIN
//


int main(int argc, char **argv)
 {
    

    int affichage = 1;
    int width = 1280;
    int height = 960;
 



//===================================================================

 int shmid, e,  i, j;
    key_t key;
    char *shm, *s;

 //Segment name
    key = 888;
    

    //Create the segment
    if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
       // perror("shmget");
cout << "No consigo Id para memoria compartida" << endl;
		
        exit(1);
    }

    //Attach the segment to our data space.
    if ((shm = (char*) shmat(shmid, NULL, 0)) == (char *) - 1) {
       // perror("shmat");


cout << "No consigo memoria compartida" << endl;
        exit(1);
    }




//===================================================================

int inc = width * height * 2 * 3;  //  width * height * 2 * 3
    ros::init(argc, argv, "Triclops_Data_publisher");
    ros::NodeHandle nh;
    // create the publishers
    ros::Publisher pub_right_image = nh.advertise<sensor_msgs::Image>("triclops/right_image_topic", 1);
    ros::Publisher pub_center_image = nh.advertise<sensor_msgs::Image>("triclops/center_image_topic", 1);
    ros::Publisher pub_left_image = nh.advertise<sensor_msgs::Image>("triclops/left_image_topic", 1);
   
   //Create the images

   //IplImage* IRGB_right = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3); 
    IplImage* IRGB_right = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1); 
    IplImage* IRGB_center = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    IplImage* IRGB_left = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
   
  //Create the ROS messages
    sensor_msgs::Image right_im;
    sensor_msgs::Image center_im;
    sensor_msgs::Image left_im;
   



  char r = 'r';
    char c = 'c';
    char l = 'l';
   

    char ch;
    int num = 0;
    int num_loop = 0;
 


int k=0;
 
 
 /* while (ch != 27)*/


 while(1) {
    
       //Read what the server put in the memory

        //Extraction of the rectified images
       int e = 0;
      // int k = 0;

      s = shm;

        for (k= 0; k < (1280 * 960 ); k ++) {
            
            IRGB_right->imageDataOrigin[k ] = s[k];
            IRGB_center->imageDataOrigin[k ] = s[k+1280*960]; 
            IRGB_left->imageDataOrigin[k ] = s[k + 1280*960*2];

            /*IRGB_right->imageData[k ] = s[k];
            IRGB_center->imageData[k ] = s[k+1280*960];
            IRGB_left->imageData[k ] = s[k + 1280*960*2];*/
            
    
            }




//******************************************************** Comprobamos q funciona memoria compartida*************//
     /* Now read what the server put in the memory.
     */
  /*  for (s = shm; *s != NULL; s++)
        putchar(*s);
    putchar('\n');

    /*
     * Finally, change the first character of the 
     * segment to '*', indicating we have read 
     * the segment.
     */
  /*  *shm = '*';

    exit(0);*/

//*****************************************************************************************************************//


  
 //===================================================================   

   
   if (affichage) {
            cvShowImage("Image Right on Client", IRGB_right);
            cvShowImage("Image Center on Client", IRGB_center);
            cvShowImage("Image Left on Client", IRGB_left);
            ch = cvWaitKey(10);
        }

 //===================================================================      


        (*IRGB_right).width = width;
        (*IRGB_right).height = height;
        cv::Mat right_imageMat(IRGB_right);
        cv_bridge::CvImage right_image_out_msg;
        right_image_out_msg.encoding = sensor_msgs::image_encodings::BGR8;
        right_image_out_msg.image = right_imageMat;
        right_image_out_msg.header.seq = r;
        right_image_out_msg.header.frame_id = r;
        right_image_out_msg.header.stamp = ros::Time::now();
        right_image_out_msg.toImageMsg(right_im);
        pub_right_image.publish(right_im);


       (*IRGB_center).width = width;
        (*IRGB_center).height = height;
        cv::Mat center_imageMat(IRGB_center);
        cv_bridge::CvImage center_image_out_msg;
        center_image_out_msg.encoding = sensor_msgs::image_encodings::BGR8;
        center_image_out_msg.image = center_imageMat;
        center_image_out_msg.header.seq = c;
        center_image_out_msg.header.frame_id = c;
        center_image_out_msg.header.stamp = ros::Time::now();
        center_image_out_msg.toImageMsg(center_im);
        pub_center_image.publish(center_im);


        (*IRGB_left).width = width;
        (*IRGB_left).height = height;
        cv::Mat left_imageMat(IRGB_left);
        cv_bridge::CvImage left_image_out_msg;
        left_image_out_msg.encoding = sensor_msgs::image_encodings::BGR8;
        left_image_out_msg.image = left_imageMat;
        left_image_out_msg.header.seq = l;
        left_image_out_msg.header.frame_id = l;
        left_image_out_msg.header.stamp = right_image_out_msg.header.stamp;
        left_image_out_msg.toImageMsg(left_im);
        pub_left_image.publish(left_im);   

        //num_loop++;
  


 } /******/   //end while

   
 exit(0);


 
//===================================================================


// Liberamos la memoria

if (((shmid = shmget(key, SHMSZ, 0666)) < 0)!=-1) {

shmdt ((char *)shm);
	}


 
  return 0;

}
