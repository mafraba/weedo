/* 
 * 
 */

#ifndef GABOR2D_H
#define	GABOR2D_H

#include <opencv2/core/core_c.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define PI		3.14159265358979323846	/* pi */

    typedef struct {
        unsigned int size;
        CvMat **filters;
    } FilterBank;

    /*
     * Creation of a particular filter with the parameters passed in 
     * as arguments
     */
    CvMat* create_gabor_filter_2d(
            float spatial_freq,
            unsigned int bandwidth,
            float orientation);

    /*
     * Creation of a bank of filters by combining the arguments passed
     * in all possible ways
     */
    void generate_gabor_filter_bank(
            FilterBank *bank,
            unsigned int n_freqs,
            float *spatial_frequencies,
            unsigned int n_orientations,
            float *orientations,
            unsigned int n_bandwidths,
            unsigned int *bandwidths);


#ifdef	__cplusplus
}
#endif

#endif	/* GABOR2D_H */

