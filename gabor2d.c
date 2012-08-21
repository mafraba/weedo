#include "gabor2d.h"
#include <math.h>
#include <stdio.h>

float gaussian(float std_deviation, int x, int y)
{
    return exp(-1 * (x * x + y * y) / (std_deviation * std_deviation));
}

float sinusoid(float wave_length, float orientation, int x, int y)
{
    return cos(wave_length * 2 * PI * (x * cos(orientation) + y * sin(orientation)));
}

/*
 * 
 */
CvMat* create_gabor_filter_2d(float spatial_freq, unsigned int bandwidth, float orientation)
{
    // The filter will be a square matrix of NxN, where N = 2*bandwidth+1
    int dim = 2 * bandwidth + 1;
    CvMat* mat = cvCreateMat(dim, dim, CV_32FC1);

    // 
    for (int row = 0; row < mat->rows; row++)
    {
        float* ptr = (float*) (mat->data.ptr + row * mat->step);
        for (int col = 0; col < mat->cols; col++)
        {
            float gaussian_component = gaussian(0.65f * bandwidth, row - bandwidth, col - bandwidth);
            float sinusoid_component = sinusoid(spatial_freq / bandwidth, orientation, row - bandwidth, col - bandwidth);
            *ptr = gaussian_component * sinusoid_component;
            ptr++;
        }
    }

    return mat;
}

/*
 */
void generate_gabor_filter_bank(
                                FilterBank *bank,
                                unsigned int n_freqs,
                                float *spatial_frequencies,
                                unsigned int n_orientations,
                                float *orientations,
                                unsigned int n_bands,
                                unsigned int *bandwidths)
{
    // Assuming 'bank' is pre-allocated
    bank->size = n_freqs * n_orientations * n_bands;

    // Allocate array of pointers to filters
    bank->filters = malloc(bank->size * sizeof (CvArr*));

    // Create filters, one by one...
    // ... iterating on frequencies
    int f = 0;
    for (int i = 0; i < n_freqs; i++)
    {
        float frq = spatial_frequencies[i];
        // ...now for each frequency, iterating on orientations
        for (int j = 0; j < n_orientations; j++)
        {
            float orn = orientations[j];
            // ...and finally on bandwidths
            for (int k = 0; k < n_bands; k++)
            {
                unsigned int bw = bandwidths[k];
                bank->filters[f] = create_gabor_filter_2d(frq, bw, orn);
            }
        }
    }
}