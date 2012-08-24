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
    bank->filters = malloc(bank->size * sizeof (CvMat*));

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

void apply_filter_bank(
                       FilterBank *bank,
                       CvMat *source,
                       CvMat **outputs
                       )
{
    printf("Source: %dx%d / %d\n", source->rows, source->cols, source->type);

    // Iterate over the filters...
    for (int fidx = 0; fidx < bank->size; fidx++)
    {
        // ...taking each one
        CvMat *f = bank->filters[fidx];
        // applying it to the source matrix
        CvMat *out = cvCreateMat(source->rows, source->cols, source->type);
        cvFilter2D(source, out, f, cvPoint(-1, -1));
        printf("Filter %d: %dx%d / %d\n", fidx, f->rows, f->cols, f->type);
        // and saving the result in the output array
        outputs[fidx] = out;
    }
}