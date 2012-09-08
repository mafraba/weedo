#include "gabor2d.h"
#include <math.h>
#include <stdio.h>

float gaussian(float std_deviation, int x, int y)
{
    return exp(-1 * (x * x + y * y) / (std_deviation * std_deviation));
}

float sinusoid_real(float wave_length, float orientation, int x, int y)
{
    return cos(wave_length * 2 * PI * (x * cos(orientation) + y * sin(orientation)));
}

float sinusoid_imag(float wave_length, float orientation, int x, int y)
{
    return sin(wave_length * 2 * PI * (x * cos(orientation) + y * sin(orientation)));
}

/*
 * 
 */
GaborFilter* create_gabor_filter_2d(float spatial_freq, unsigned int bandwidth, float orientation)
{
    // The filter will be a square matrix of NxN, where N = 2*bandwidth+1
    int dim = 2 * bandwidth + 1;
    GaborFilter* filter = malloc(sizeof (GaborFilter));
    CvMat* mat1 = cvCreateMat(dim, dim, CV_32FC1);
    CvMat* mat2 = cvCreateMat(dim, dim, CV_32FC1);
    filter->real = mat1;
    filter->imag = mat2;

    // 
    for (int row = 0; row < mat1->rows; row++)
    {
        float* ptr1 = (float*) (mat1->data.ptr + row * mat1->step);
        float* ptr2 = (float*) (mat2->data.ptr + row * mat2->step);
        for (int col = 0; col < mat1->cols; col++)
        {
            float gaussian_component = gaussian(0.65f * bandwidth, row - bandwidth, col - bandwidth);
            float sinusoid_component_r = sinusoid_real(spatial_freq / bandwidth, orientation, row - bandwidth, col - bandwidth);
            float sinusoid_component_i = sinusoid_imag(spatial_freq / bandwidth, orientation, row - bandwidth, col - bandwidth);
            *ptr1 = gaussian_component * sinusoid_component_r;
            *ptr2 = gaussian_component * sinusoid_component_i;
            ptr1++;
            ptr2++;
        }
    }

    return filter;
}

/*
 */
void generate_gabor_filter_bank(
                                FilterBank *bank,
                                unsigned int n_bands,
                                unsigned int *bandwidths,
                                unsigned int n_freqs,
                                float *spatial_frequencies,
                                unsigned int n_orientations,
                                float *orientations
                                )
{
    // Assuming 'bank' is pre-allocated
    bank->size = n_freqs * n_orientations * n_bands;

    // Allocate array of pointers to filters
    bank->filters = malloc(bank->size * sizeof (GaborFilter*));

    // Create filters, one by one...
    int f = 0;
    // ...covering all bandwidths
    for (int k = 0; k < n_bands; k++)
    {
        unsigned int bw = bandwidths[k];
        // ... then frequencies
        for (int i = 0; i < n_freqs; i++)
        {
            float frq = spatial_frequencies[i];
            // ...then orientations
            for (int j = 0; j < n_orientations; j++)
            {
                float orn = orientations[j];
                bank->filters[f] = create_gabor_filter_2d(frq, bw, orn);
                f++;
            }
        }
    }
}

void apply_filter(GaborFilter *filter, CvMat *source, CvMat *output)
{
    CvMat *out_r = cvCreateMat(source->rows, source->cols, CV_32FC1);
    CvMat *out_i = cvCreateMat(source->rows, source->cols, CV_32FC1);

    cvFilter2D(source, out_r, filter->real, cvPoint(-1, -1));
    cvFilter2D(source, out_i, filter->imag, cvPoint(-1, -1));

    // Now sum the squares to get the energy
    cvCartToPolar(out_r, out_i, output, NULL, 0);
    
//    cvCopy(out_r, output, NULL);

    cvRelease(&out_r);
    cvRelease(&out_i);
}

void apply_filter_bank(
                       FilterBank *bank,
                       CvMat *source,
                       CvMat **outputs
                       )
{
    // Iterate over the filters...
    for (int fidx = 0; fidx < bank->size; fidx++)
    {
        // ...taking each one
        // applying it to the source matrix
        CvMat *out = cvCreateMat(source->rows, source->cols, CV_32FC1);
        apply_filter(bank->filters[fidx], source, out);
        // cvSmooth(out, out, CV_GAUSSIAN, 5, 0, 0, 0);
        // and saving the result in the output array
        outputs[fidx] = out;
    }
}