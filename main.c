/* 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include "gabor2d.h"

#define PATH "pics/PTOA0124.png"

#define ORIGINAL_IMAGE_WINDOW_NAME "Original image"
#define FILTER_WINDOW_NAME "Filter (resized)"
#define RESULT_WINDOW_NAME "Filtered image"
#define REDF_WINDOW_NAME "Filtered image (red channel)"
#define BLUEF_WINDOW_NAME "Filtered image (blue channel)"
#define GREENF_WINDOW_NAME "Filtered image (green channel)"

void show(char* name, CvArr* img)
{
    cvNamedWindow(name, CV_WINDOW_AUTOSIZE);
    cvShowImage(name, img);
}

int main(int argc, char** argv)
{
    // Load and display original image
    IplImage* img = cvLoadImage(PATH, CV_LOAD_IMAGE_UNCHANGED);
    show(ORIGINAL_IMAGE_WINDOW_NAME, img);

    // Generate 2D gabor filter
    CvMat* gabor = create_gabor_filter_2d(2, 8, PI/3);

    // Display the filter
    IplImage *filter_display = cvCreateImage(cvSize(200, 200), IPL_DEPTH_32F, 1);
    //cvResize(gabor, filter_display, 2);
    //cvConvertScale(filter_display, filter_display, 0.5, 0.5);
    show(FILTER_WINDOW_NAME, gabor);

    // Apply the filter to the image
    IplImage *filtered = cvCloneImage(img);
    cvFilter2D(img, filtered, gabor, cvPoint(-1, -1));
    // Normalize and display the results
    //cvNormalize(filtered, filtered, 0, 255, CV_MINMAX, NULL);
    show(RESULT_WINDOW_NAME, filtered);
    // Show also each channel separately
    IplImage *ch1 = cvCreateImage(cvGetSize(img), img->depth, 1);
    IplImage *ch2 = cvCreateImage(cvGetSize(img), img->depth, 1);
    IplImage *ch3 = cvCreateImage(cvGetSize(img), img->depth, 1);
    cvSplit(filtered, ch1, ch2, ch3, NULL);
    show(REDF_WINDOW_NAME, ch1);
    show(GREENF_WINDOW_NAME, ch2);
    show(BLUEF_WINDOW_NAME, ch3);

    cvWaitKey(0);

    return (EXIT_SUCCESS);
}

