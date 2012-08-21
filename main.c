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
    unsigned int bandwidths[1] = {8};
    float orientations[4] = {0, PI / 2, PI / 4, -PI / 4};
    float spatial_frequencies[4] = {1, 2, 3, 4};

    // Load and display original image
    IplImage* img = cvLoadImage(PATH, CV_LOAD_IMAGE_UNCHANGED);
    show(ORIGINAL_IMAGE_WINDOW_NAME, img);

    // Generate a Gabor filter bank
    FilterBank filter_bank;
    generate_gabor_filter_bank(&filter_bank,
                               4,
                               spatial_frequencies,
                               4,
                               orientations,
                               1,
                               bandwidths);

    // Apply the filter to the image
    /*
    IplImage *filtered = cvCloneImage(img);
    cvFilter2D(img, filtered, gabor, cvPoint(-1, -1));
    show(RESULT_WINDOW_NAME, filtered);

    // Show also each channel separately
    IplImage *ch1 = cvCreateImage(cvGetSize(img), img->depth, 1);
    IplImage *ch2 = cvCreateImage(cvGetSize(img), img->depth, 1);
    IplImage *ch3 = cvCreateImage(cvGetSize(img), img->depth, 1);
    cvSplit(filtered, ch1, ch2, ch3, NULL);
    show(REDF_WINDOW_NAME, ch1);
    show(GREENF_WINDOW_NAME, ch2);
    show(BLUEF_WINDOW_NAME, ch3);
     * */

    cvWaitKey(0);

    return (EXIT_SUCCESS);
}

