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

#define PATH "pics/05020068.JPG"

#define ORIGINAL_IMAGE_WINDOW_NAME "Original image"
#define PRETREATED_IMAGE_WINDOW_NAME "Pretreated"
#define OUTPUT_PATH "/tmp/weedo"

// ---------- OPCIONES -------------------------
// Eliminamos componente L (de espacio Lab) ?
#define IGNORAR_L 1
// Suavizamos imagen original?
#define SMOOTH_ORIGINAL 0

// Bandwidths
#define N_BANDWIDTHS 3
unsigned int bandwidths[N_BANDWIDTHS] = {4, 8, 16};

// Orientations
#define N_ORIENTATIONS 4
float orientations[N_ORIENTATIONS] = {0, PI / 4, PI / 2, 3 * PI / 4};
//{0, PI / 6, 2 * PI / 6, 3 * PI / 6, 4 * PI / 6, 5 * PI / 6};
//{0, PI / 2};

// Spatial frequencies
#define N_FREQS 2
float spatial_frequencies[N_FREQS] = {1, 1.5};

// Number of clusters
#define K_CLUSTERS 4

void show(char* name, CvArr* img)
{
    printf("Displaying %s\n", name);
    fflush(stdout);
    cvNamedWindow(name, CV_WINDOW_AUTOSIZE);
    cvShowImage(name, img);
    cvWaitKey(0);
}

void output_base_channels(CvMat *img)
{
    CvMat *ch1 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    CvMat *ch2 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    CvMat *ch3 = cvCreateMat(img->rows, img->cols, CV_8UC1);
    cvSplit(img, ch1, ch2, ch3, NULL);
    char out_file_name[256];
    sprintf(out_file_name, "%s/%s.png", OUTPUT_PATH, "base");
    cvSaveImage(out_file_name, img, NULL);
    sprintf(out_file_name, "%s/%s.png", OUTPUT_PATH, "base_channel_1");
    cvSaveImage(out_file_name, ch1, NULL);
    sprintf(out_file_name, "%s/%s.png", OUTPUT_PATH, "base_channel_2");
    cvSaveImage(out_file_name, ch2, NULL);
    sprintf(out_file_name, "%s/%s.png", OUTPUT_PATH, "base_channel_3");
    cvSaveImage(out_file_name, ch3, NULL);
    cvReleaseMat(&ch1);
    cvReleaseMat(&ch2);
    cvReleaseMat(&ch3);
}

void output_filtered_images(char *prefix, unsigned int n, CvMat** imgs)
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

void output_filter_bank(FilterBank *fb)
{
    int i = 0;
    for (int bw = 0; bw < N_BANDWIDTHS; bw++)
    {
        for (int frq = 0; frq < N_FREQS; frq++)
        {
            for (int orn = 0; orn < N_ORIENTATIONS; orn++)
            {
                char out_file_name[256];
                sprintf(out_file_name, "%s/%s_%02d_%02.2f_%02.2f.png",
                        OUTPUT_PATH,
                        "FILTER",
                        bandwidths[bw],
                        spatial_frequencies[frq],
                        orientations[orn]);
                puts(out_file_name);
                CvMat *out = cvClone(fb->filters[i]->real);
                cvNormalize(out, out, 255, 0, CV_MINMAX, NULL);
                cvSaveImage(out_file_name, out, NULL);
                cvReleaseMat(&out);
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

void img_from_labels(CvMat* labels, CvMat **classes, CvMat* color_dst, CvScalar *color_tab)
{
    for (int row = 0; row < color_dst->rows; row++)
    {
        for (int col = 0; col < color_dst->cols; col++)
        {
            int i = CV_MAT_ELEM(*labels, int, row * color_dst->cols + col, 0);
            cvSet2D(color_dst, row, col, color_tab[i]);
            cvSet2D(classes[i], row, col, cvRealScalar(255));
        }
    }
}

void output_classes(CvMat **classes, CvMat *orig)
{
    char file[256];
    for (int i = 0; i < K_CLUSTERS; i++)
    {
        CvMat *masked = cvClone(orig);
        cvZero(masked);
        cvCopy(orig, masked, classes[i]);
        sprintf(file, "%s/%s_%d.png", OUTPUT_PATH, "class", i);
        cvSaveImage(file, masked, NULL);
        cvReleaseMat(&masked);
    }
}

int main(int argc, char** argv)
{
    // Load and display original image
    puts("Loading image...");
    CvMat* img = cvLoadImageM(PATH, CV_LOAD_IMAGE_COLOR);
    CvMat* orig = cvCloneMat(img);
    cvCvtColor(img, img, CV_BGR2Lab);
    if (SMOOTH_ORIGINAL)
    {
        cvSmooth(img, img, CV_GAUSSIAN, SMOOTH_ORIGINAL, 0, 0, 0);
    }

    //show(ORIGINAL_IMAGE_WINDOW_NAME, orig);
    //show(PRETREATED_IMAGE_WINDOW_NAME, img);

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
    CvMat **filtered_channel_1 = results;
    apply_filter_bank(&filter_bank, ch1, filtered_channel_1);
    CvMat **filtered_channel_2 = results + filter_bank.size;
    apply_filter_bank(&filter_bank, ch2, filtered_channel_2);
    CvMat **filtered_channel_3 = results + 2 * filter_bank.size;
    apply_filter_bank(&filter_bank, ch3, filtered_channel_3);

    // Now sort the samples
    puts("Sorting...");
    int n_channels = (IGNORAR_L ? 2 : 3);
    results = (IGNORAR_L ? filtered_channel_2 : results);
    CvMat *samples;
    sort_samples(n_channels * filter_bank.size, results, &samples);
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
              10, NULL, 0, NULL, NULL);
    puts("done");
    fflush(stdout);

    CvMat *color_labels = cvCreateMat(img->rows, img->cols, CV_8UC3);
    CvMat **classes = malloc(K_CLUSTERS * sizeof (CvMat*));
    for (int i = 0; i < K_CLUSTERS; i++)
    {
        classes[i] = cvCreateMat(img->rows, img->cols, CV_8UC1);
        cvZero(classes[i]);
    }
    img_from_labels(labels, classes, color_labels, color_tab);
    //show("Labels", labeled_img);

    CvMat *mix = cvClone(img);
    cvAddWeighted(orig, 0.7, color_labels, 0.3, 0, mix);

    //
    puts("Outputting...");
    char out_file_name[256];
    sprintf(out_file_name, "%s/%s.png", OUTPUT_PATH, "original");
    cvSaveImage(out_file_name, orig, NULL);
    output_base_channels(img);
    if (!IGNORAR_L)
    {
        output_filtered_images("CH1", filter_bank.size, filtered_channel_1);
    }
    output_filtered_images("CH2", filter_bank.size, filtered_channel_2);
    output_filtered_images("CH3", filter_bank.size, filtered_channel_3);
    output_filter_bank(&filter_bank);
    // output labels
    output_classes(classes, orig);
    // output mix
    sprintf(out_file_name, "%s/%s.png", OUTPUT_PATH, "mix");
    cvSaveImage(out_file_name, mix, NULL);

    //show("Mix", mix);
    //    cvWaitKey(0);
    //    cvWaitKey(0);
    //    cvWaitKey(0);
    // Should do some cleanup here... :_(

    return (EXIT_SUCCESS);
}

