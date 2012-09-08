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

#define PATH "pics/PTOA0128.png"

#define ORIGINAL_IMAGE_WINDOW_NAME "Original image"
#define CHROMACITY_IMAGE_WINDOW_NAME "Chromacity"
#define OUTPUT_PATH "/tmp/weedo"

// Bandwidths
#define N_BANDWIDTHS 2
unsigned int bandwidths[N_BANDWIDTHS] = {8, 4};

// Orientations as recommended in [1]
#define N_ORIENTATIONS 4
float orientations[N_ORIENTATIONS] = {0, PI / 4, PI / 2, 3 * PI / 4};
//{0, PI / 6, 2 * PI / 6, 3 * PI / 6, 4 * PI / 6, 5 * PI / 6};

// Spatial frequencies
#define N_FREQS 3
float spatial_frequencies[N_FREQS] = {1, 2, 3};

// Number of clusters
#define K_CLUSTERS 4

void show(char* name, CvArr* img)
{
    printf("Displaying %s\n", name);
    fflush(stdout);
    cvNamedWindow(name, CV_WINDOW_AUTOSIZE);
    cvShowImage(name, img);
}

void output_filtered_images(char *prefix, unsigned int n, CvArr** imgs)
{
    int i = 0;
    for (int bw = 0; bw < N_BANDWIDTHS; bw++)
    {
        for (int frq = 0; frq < N_FREQS; frq++)
        {
            for (int orn = 0; orn < N_ORIENTATIONS; orn++)
            {
                char out_file_name[256];
                sprintf(out_file_name, "%s/%s_%s_%02d_%02.2f_%02.2f.png",
                        OUTPUT_PATH,
                        prefix,
                        "FLT",
                        bandwidths[bw],
                        spatial_frequencies[frq],
                        orientations[orn]);
                puts(out_file_name);
                cvSaveImage(out_file_name, imgs[i], NULL);
                i++;
            }
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

        img->data.ptr[i + 2] = r;
        img->data.ptr[i + 1] = g;
        img->data.ptr[i] = b;
    }

}

/* Takes N input matrices, then sorts that data into a single matrix
 * where each row represents a pixel, with N columns corresponding to the value
 * of that pixel in each one of the original matrices.
 * 
 * I.e., builds a sample vector from each pixel
 */
void sort_samples(unsigned int k, CvMat** images, CvMat** samples)
{
    // k is the number of images, i.e., the sample vector length
    // n_samples is the number of pixels
    unsigned int n_samples = images[0]->cols * images[0]->rows;
    *samples = cvCreateMat(n_samples // rows
                           , k // cols
                           , CV_32FC1);

    // each sample corresponds to a pixel, so for each row and column
    int smp_idx = 0;
    for (int row = 0; row < images[0]->rows; row++)
    {
        for (int col = 0; col < images[0]->cols; col++)
        {
            float *sample_ptr = (float*) ((*samples)->data.ptr + smp_idx * (*samples)->step);
            // Now for each sample we have 'k' values, one from each image
            for (int i = 0; i < k; i++)
            {
                *sample_ptr = CV_MAT_ELEM(*(images[i]), float, row, col);
                sample_ptr++;
            }
            smp_idx++;
        }
    }
}

void img_from_labels(CvMat* labels, CvMat* dst, CvScalar *color_tab)
{
    for (int row = 0; row < dst->rows; row++)
    {
        for (int col = 0; col < dst->cols; col++)
        {
            int i = CV_MAT_ELEM(*labels, int, row * dst->cols + col, 0);
            cvSet2D(dst, row, col, color_tab[i]);
        }
    }
}

int main(int argc, char** argv)
{
    // Load and display original image
    puts("Loading image...");
    CvMat* img = cvLoadImageM(PATH, CV_LOAD_IMAGE_COLOR);
    cvSmooth(img, img, CV_GAUSSIAN, 3, 0, 0, 0);
    CvMat* orig = cvCloneMat(img);

    chromacity(img);
    show(ORIGINAL_IMAGE_WINDOW_NAME, orig);
    show(CHROMACITY_IMAGE_WINDOW_NAME, img);

    // Generate a Gabor filter bank
    puts("Generating Gabor filter bank...");
    FilterBank filter_bank;
    generate_gabor_filter_bank(&filter_bank,
                               N_BANDWIDTHS, bandwidths,
                               N_FREQS, spatial_frequencies,
                               N_ORIENTATIONS, orientations);

    // Separate each channel
    puts("Separating channels...");
    CvMat *ch1 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    CvMat *ch2 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    CvMat *ch3 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    cvSplit(img, ch1, ch2, ch3, NULL);

    // Apply the filter bank on each one of them
    puts("Applying filters...");
    CvMat **results = (CvMat**) malloc(3 * filter_bank.size * sizeof (CvMat*));
    CvMat **red_flt = results;
    apply_filter_bank(&filter_bank, ch1, red_flt);
    CvMat **green_flt = results + filter_bank.size;
    apply_filter_bank(&filter_bank, ch2, green_flt);
    CvMat **blue_flt = results + 2 * filter_bank.size;
    apply_filter_bank(&filter_bank, ch3, blue_flt);

    //
    puts("Outputting...");
    output_filtered_images("RED", filter_bank.size, red_flt);
    output_filtered_images("GREEN", filter_bank.size, green_flt);
    output_filtered_images("BLUE", filter_bank.size, blue_flt);

    // Now sort the samples
    puts("Sorting...");
    CvMat *samples;
    sort_samples(3 * filter_bank.size, results, &samples);
    printf("Samples: %d(x%d)", samples->rows, samples->cols);
    fflush(stdout);

    // And cluster them
    printf("Clustering... ");

    CvScalar color_tab[8];
    color_tab[0] = CV_RGB(255, 0, 0);
    color_tab[1] = CV_RGB(0, 255, 0);
    color_tab[2] = CV_RGB(0, 0, 255);
    color_tab[3] = CV_RGB(0, 255, 255);
    color_tab[4] = CV_RGB(255, 0, 255);
    color_tab[5] = CV_RGB(255, 255, 0);
    color_tab[6] = CV_RGB(255, 255, 255);
    color_tab[7] = CV_RGB(0, 0, 0);

    CvMat *labels = cvCreateMat(samples->rows, 1, CV_32SC1);
    cvKMeans2(samples, K_CLUSTERS, labels,
              cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
              10, NULL, NULL, NULL, NULL);
    puts("done");
    fflush(stdout);

    CvMat *labeled_img = cvCreateMat(img->rows, img->cols, CV_8UC3);
    img_from_labels(labels, labeled_img, color_tab);
    show("Labels", labeled_img);

    CvMat *mix = cvClone(img);
    cvAddWeighted(orig, 0.9, labeled_img, 0.1, 0, mix);
    show("Mix", mix);
    cvWaitKey(0);
    cvWaitKey(0);
    cvWaitKey(0);
    // Should do some cleanup here... :_(

    return (EXIT_SUCCESS);
}

