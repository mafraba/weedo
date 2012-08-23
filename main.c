/* 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
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
    puts("Loading image...");
    CvMat* img = cvLoadImageM(PATH, CV_LOAD_IMAGE_COLOR);
    show(ORIGINAL_IMAGE_WINDOW_NAME, img);

    // Generate a Gabor filter bank
    puts("Generating Gabor filter bank...");
    FilterBank filter_bank;
    generate_gabor_filter_bank(&filter_bank,
                               4, spatial_frequencies,
                               4, orientations,
                               1, bandwidths);

    // Separate each channel
    puts("Separating channels...");
    CvMat *ch1 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    CvMat *ch2 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    CvMat *ch3 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    cvSplit(img, ch1, ch2, ch3, NULL);

    // Apply the filter bank on each
    puts("Applying filters...");
    printf("Source: %dx%d\n", ch1->rows, ch1->cols);
    CvMat **flt_1 = malloc(filter_bank.size * sizeof (CvMat*));
    apply_filter_bank(&filter_bank, ch1, flt_1);

    cvWaitKey(0);

    // Should do some cleanup here... :_(

    return (EXIT_SUCCESS);
}

