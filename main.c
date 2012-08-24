/*
 * Application of gabor filters to classify sky, terrain, corn and weeds
 * in agricultural images.
 * 
 * References:
 * 
 * [1] D. Clausi, M. Ed Jernigan, ”Designing Gabor filters for optimal texture 
 * separability,” Pattern Recognition, vol. 33, pp. 1835-1849, 2000.
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

unsigned int bandwidths[1] = {8};
// Orientations as recommended in [1]
float orientations[6] = {0, PI / 6, 2 * PI / 6, 3 * PI / 6, 4 * PI / 6, 5 * PI / 6};
// Spatial frequencies
float spatial_frequencies[4] = {1, 2, 3, 4};

void show(char* name, CvArr* img)
{
    cvNamedWindow(name, CV_WINDOW_AUTOSIZE);
    cvShowImage(name, img);
}

void output_filtered_images(char *prefix, unsigned int n, CvArr** imgs)
{
    int i = 0;
    for (int frq = 0; frq < 4; frq++)
    {
        for (int orn = 0; orn < 6; orn++)
        {
            char out_file_name[256];
            sprintf(out_file_name, "%s/%s_%s_%02d_%02.2f_%02.2f.png",
                    OUTPUT_PATH,
                    prefix,
                    "FLT",
                    bandwidths[0],
                    spatial_frequencies[frq],
                    orientations[orn]);
            puts(out_file_name);
            cvSaveImage(out_file_name, imgs[i], NULL);
            i++;
        }
    }
}

void chromacity(CvMat* img)
{
    int pixels = img->rows * img->cols;
    int channels = 3; // assumed

    for (int i = 0; i < pixels * channels; i += channels)
    {
        unsigned char red = img->data.ptr[i + 2];
        unsigned char green = img->data.ptr[i + 1];
        unsigned char blue = img->data.ptr[i];

        double total = red + green + blue;
        unsigned char r = round(red * 255.0 / total);
        unsigned char g = round(green * 255.0 / total);
        unsigned char b = round(blue * 255.0 / total);
        
        img->data.ptr[i+2] = r;
        img->data.ptr[i+1] = g;
        img->data.ptr[i] = b;
    }

}

int main(int argc, char** argv)
{
    // Load and display original image
    puts("Loading image...");
    CvMat* img = cvLoadImageM(PATH, CV_LOAD_IMAGE_COLOR);
    chromacity(img);
    show(ORIGINAL_IMAGE_WINDOW_NAME, img);

    // Generate a Gabor filter bank
    puts("Generating Gabor filter bank...");
    FilterBank filter_bank;
    generate_gabor_filter_bank(&filter_bank,
                               1, bandwidths,
                               4, spatial_frequencies,
                               6, orientations);

    // Separate each channel
    puts("Separating channels...");
    CvMat *ch1 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    CvMat *ch2 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    CvMat *ch3 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    cvSplit(img, ch1, ch2, ch3, NULL);

    // Apply the filter bank on each one of them
    puts("Applying filters...");
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

