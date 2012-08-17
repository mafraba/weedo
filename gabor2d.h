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

    /*
     * 
     */
    CvMat* create_gabor_filter_2d(
            float spatial_freq,
            int bandwith,
            float orientation);


#ifdef	__cplusplus
}
#endif

#endif	/* GABOR2D_H */

