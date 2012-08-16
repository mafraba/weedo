/* 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include "gabor2d.h"

#define PATH "pics/PTOA0124.png"

int main(int argc, char** argv)
{
    IplImage* img = cvLoadImage(PATH, CV_LOAD_IMAGE_COLOR);
    cvNamedWindow("Example1", CV_WINDOW_AUTOSIZE);
    cvShowImage("Example1", img);
    
    // create a 2D gabor filter
    CvMat* gabor = create_gabor_filter_2d(3, 9, 0);
    
    // Release original image
    cvReleaseImage(&img);
    cvDestroyWindow("Example1");

    return (EXIT_SUCCESS);
}

