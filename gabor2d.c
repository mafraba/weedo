#include "gabor2d.h"
#include <math.h>
#include <stdio.h>

float gaussian(float std_deviation, int x, int y)
{
    return exp(-1*(x * x + y * y) / (std_deviation * std_deviation));
}

float sinusoid(float wave_length, float orientation, int x, int y)
{
    return cos(wave_length * 2 * PI * (x * cos(orientation) + y * sin(orientation)));
}

/*
 * 
 */
CvMat* create_gabor_filter_2d(float spatial_freq, int bandwidth, float orientation)
{
    // The filter will be a square matrix of NxN, where N = 2*bandwidth+1
    int dim = 2 * bandwidth + 1;
    CvMat* mat = cvCreateMat(dim, dim, CV_32FC1);

    float max_g = 0, max_s = 0;
    float min_g = 0, min_s = 0;

    // 
    for (int row = 0; row < mat->rows; row++)
    {
        float* ptr = (float*) (mat->data.ptr + row * mat->step);
        for (int col = 0; col < mat->cols; col++)
        {
            float gaussian_component = gaussian(0.65f * bandwidth, row - bandwidth, col - bandwidth);
            float sinusoid_component = sinusoid(spatial_freq / bandwidth, orientation, row - bandwidth, col - bandwidth);
            max_g = (max_g > gaussian_component) ? max_g : gaussian_component;
            max_s = (max_s > sinusoid_component) ? max_s : sinusoid_component;
            min_g = (min_g < gaussian_component) ? min_g : gaussian_component;
            min_s = (min_s < sinusoid_component) ? min_s : sinusoid_component;
            *ptr = (gaussian_component * sinusoid_component + 1) / 2;
            ptr++;
        }
    }

    printf("Maxs: %f %f\n", max_g, max_s);
    printf("Mins: %f %f\n", min_g, min_s);

    return mat;
}
