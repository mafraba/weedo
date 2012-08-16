#include "gabor2d.h"

CvMat* create_gabor_filter_2d(double spatial_freq,
                              int bandwidth,
                              double orientation)
{
    // The filter will be a square matrix of NxN, where N = 2*bandwith+1
    int dim = 2 * bandwidth + 1;
    CvMat* mat = cvCreateMat(dim, dim, CV_32FC1);

    // 
    for (int row = 0; row < mat->rows; row++)
    {
        float* ptr = (float*) (mat->data.ptr + row * mat->step);
        for (int col = 0; col < mat->cols; col++)
        {
            float gaussian_component = gaussian(spatial_freq,bandwidth,row,col);
            float sinusoid_component = sinusoid(spatial_freq,bandwidth,orientation,row,col);
            *ptr = gaussian_component * sinusoid_component;
            ptr++;
        }
    }

}
