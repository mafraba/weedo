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
    // IplImage* img = cvLoadImage(PATH, CV_LOAD_IMAGE_COLOR);

    // create a 2D gabor filter
    CvMat* gabor = create_gabor_filter_2d(3, 200, PI/4);
    
    IplImage *filter = cvCreateImage(cvSize(401,401), IPL_DEPTH_32F, 1);
    cvConvert(gabor, filter);
    
    cvNamedWindow("Example1", CV_WINDOW_AUTOSIZE);
    cvShowImage("Example1", filter);

    cvWaitKey(0);

    // Release original image
    cvReleaseImage(&filter);
    cvDestroyWindow("Example1");

    return (EXIT_SUCCESS);
}

