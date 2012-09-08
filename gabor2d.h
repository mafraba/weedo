/* 
 * 
 */

#ifndef GABOR2D_H
#define	GABOR2D_H

#include <opencv/cv.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define PI		3.14159265358979323846	/* pi */

    typedef struct {
        CvMat *real;
        CvMat *imag;
    } GaborFilter;

    typedef struct {
        unsigned int size;
        GaborFilter **filters;
    } FilterBank;
    
    /*
     * Creation of a particular filter with the parameters passed in 
     * as arguments
     */
    GaborFilter* create_gabor_filter_2d(
            float spatial_freq,
            unsigned int bandwidth,
            float orientation);

    /*
     * Creation of a bank of filters by combining the arguments passed
     * in all possible ways
     */
    void generate_gabor_filter_bank(
            FilterBank *bank,
            unsigned int n_bandwidths,
            unsigned int *bandwidths,
            unsigned int n_freqs,
            float *spatial_frequencies,
            unsigned int n_orientations,
            float *orientations
            );


    /*
     * Apply a filter bank to a single-channel matrix.
     * 
     * The result set is an array of matrixes, one per filter in the bank, same
     * dimensions as the original.
     */
    void apply_filter_bank(
            FilterBank *bank,
            CvMat *source,
            CvMat **outputs
            );

#ifdef	__cplusplus
}
#endif

#endif	/* GABOR2D_H */

