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
#define OUTPUT_PATH "/tmp/weedo"

void show(char* name, CvArr* img)
{
    cvNamedWindow(name, CV_WINDOW_AUTOSIZE);
    cvShowImage(name, img);
}

void output_filtered_images(char *prefix, unsigned int n, CvArr** imgs)
{
    for (int i = 0; i < n; i++)
    {
        char out_file_name[256];
        sprintf(out_file_name, "%s/%s_%s_%02d.png", OUTPUT_PATH, prefix, "FLT", i);
        puts(out_file_name);
        cvSaveImage(out_file_name, imgs[i], NULL);
    }
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
                               1, bandwidths,
                               4, spatial_frequencies,
                               4, orientations);

    // Separate each channel
    puts("Separating channels...");
    CvMat *ch1 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    CvMat *ch2 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    CvMat *ch3 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    cvSplit(img, ch1, ch2, ch3, NULL);

    // Apply the filter bank on each one of them
    puts("Applying filters...");
    printf("Source: %dx%d\n", ch1->rows, ch1->cols);
    CvMat **red_flt = malloc(filter_bank.size * sizeof (CvMat*));
    apply_filter_bank(&filter_bank, ch1, red_flt);
    CvMat **green_flt = malloc(filter_bank.size * sizeof (CvMat*));
    apply_filter_bank(&filter_bank, ch2, green_flt);
    CvMat **blue_flt = malloc(filter_bank.size * sizeof (CvMat*));
    apply_filter_bank(&filter_bank, ch3, blue_flt);

    // 
    puts("Outputting...");
    output_filtered_images("RED", filter_bank.size, red_flt);
    output_filtered_images("GREEN", filter_bank.size, green_flt);
    output_filtered_images("BLUE", filter_bank.size, blue_flt);

    cvWaitKey(0);

    // Should do some cleanup here... :_(

    return (EXIT_SUCCESS);
}

