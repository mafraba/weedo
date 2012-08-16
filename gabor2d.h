/* 
 * 
 */

#ifndef GABOR2D_H
#define	GABOR2D_H

#include <opencv2/core/core_c.h>

#ifdef	__cplusplus
extern "C" {
#endif

    /*
     * 
     */
    CvMat* create_gabor_filter_2d(
            double spatial_freq,
            int bandwith,
            double orientation);


#ifdef	__cplusplus
}
#endif

#endif	/* GABOR2D_H */

