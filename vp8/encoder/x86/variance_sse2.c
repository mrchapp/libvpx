/*
 *  Copyright (c) 2010 The VP8 project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "variance.h"
#include "pragmas.h"
#include "vpx_ports/mem.h"

extern void filter_block1d_h6_mmx(unsigned char *src_ptr, unsigned short *output_ptr, unsigned int src_pixels_per_line, unsigned int pixel_step, unsigned int output_height, unsigned int output_width, short *vp7_filter);
extern void filter_block1d_v6_mmx(short *src_ptr, unsigned char *output_ptr, unsigned int pixels_per_line, unsigned int pixel_step, unsigned int output_height, unsigned int output_width, short *vp7_filter);
extern void filter_block1d8_h6_sse2(unsigned char *src_ptr, unsigned short *output_ptr, unsigned int src_pixels_per_line, unsigned int pixel_step, unsigned int output_height, unsigned int output_width, short *vp7_filter);
extern void filter_block1d8_v6_sse2(short *src_ptr, unsigned char *output_ptr, unsigned int pixels_per_line, unsigned int pixel_step, unsigned int output_height, unsigned int output_width, short *vp7_filter);

extern void vp8_filter_block2d_bil4x4_var_mmx
(
    unsigned char *ref_ptr,
    int ref_pixels_per_line,
    unsigned char *src_ptr,
    int src_pixels_per_line,
    const short *HFilter,
    const short *VFilter,
    int *sum,
    unsigned int *sumsquared
);

extern unsigned int vp8_get4x4var_mmx
(
    unsigned char *src_ptr,
    int  source_stride,
    unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *SSE,
    int *Sum
);

unsigned int vp8_get_mb_ss_sse2
(
    short *src_ptr
);
unsigned int vp8_get16x16var_sse2
(
    unsigned char     *src_ptr,
    int             source_stride,
    unsigned char     *ref_ptr,
    int             recon_stride,
    unsigned int      *SSE,
    int               *Sum
);
unsigned int vp8_get16x16pred_error_sse2
(
    unsigned char *src_ptr,
    int src_stride,
    unsigned char *ref_ptr,
    int ref_stride
);
unsigned int vp8_get8x8var_sse2
(
    unsigned char     *src_ptr,
    int             source_stride,
    unsigned char     *ref_ptr,
    int             recon_stride,
    unsigned int      *SSE,
    int               *Sum
);
void vp8_filter_block2d_bil_var_sse2
(
    unsigned char *ref_ptr,
    int ref_pixels_per_line,
    unsigned char *src_ptr,
    int src_pixels_per_line,
    unsigned int Height,
    const short *HFilter,
    const short *VFilter,
    int *sum,
    unsigned int *sumsquared
);
void vp8_half_horiz_vert_variance16x_h_sse2
(
    unsigned char *ref_ptr,
    int ref_pixels_per_line,
    unsigned char *src_ptr,
    int src_pixels_per_line,
    unsigned int Height,
    int *sum,
    unsigned int *sumsquared
);
void vp8_half_horiz_variance16x_h_sse2
(
    unsigned char *ref_ptr,
    int ref_pixels_per_line,
    unsigned char *src_ptr,
    int src_pixels_per_line,
    unsigned int Height,
    int *sum,
    unsigned int *sumsquared
);
void vp8_half_vert_variance16x_h_sse2
(
    unsigned char *ref_ptr,
    int ref_pixels_per_line,
    unsigned char *src_ptr,
    int src_pixels_per_line,
    unsigned int Height,
    int *sum,
    unsigned int *sumsquared
);

DECLARE_ALIGNED(16, extern short, vp8_vp7_bilinear_filters_mmx[8][8]);

unsigned int vp8_variance4x4_wmt(
    unsigned char *src_ptr,
    int  source_stride,
    unsigned char *ref_ptr,
    int  recon_stride)
{
    unsigned int var;
    int avg;

    vp8_get4x4var_mmx(src_ptr, source_stride, ref_ptr, recon_stride, &var, &avg) ;
    return (var - ((avg * avg) >> 4));

}



unsigned int vp8_variance8x8_wmt
(
    unsigned char *src_ptr,
    int  source_stride,
    unsigned char *ref_ptr,
    int  recon_stride)
{
    unsigned int var;
    int avg;

    vp8_get8x8var_sse2(src_ptr, source_stride, ref_ptr, recon_stride, &var, &avg) ;

    return (var - ((avg * avg) >> 6));

}


unsigned int vp8_variance16x16_wmt
(
    unsigned char *src_ptr,
    int  source_stride,
    unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    unsigned int sse0;
    int sum0;


    vp8_get16x16var_sse2(src_ptr, source_stride, ref_ptr, recon_stride, &sse0, &sum0) ;
    *sse = sse0;
    return (sse0 - ((sum0 * sum0) >> 8));
}
unsigned int vp8_mse16x16_wmt(
    unsigned char *src_ptr,
    int  source_stride,
    unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{

    unsigned int sse0;
    int sum0;
    vp8_get16x16var_sse2(src_ptr, source_stride, ref_ptr, recon_stride, &sse0, &sum0) ;
    *sse = sse0;
    return sse0;

}


unsigned int vp8_variance16x8_wmt
(
    unsigned char *src_ptr,
    int  source_stride,
    unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    unsigned int sse0, sse1, var;
    int sum0, sum1, avg;

    vp8_get8x8var_sse2(src_ptr, source_stride, ref_ptr, recon_stride, &sse0, &sum0) ;
    vp8_get8x8var_sse2(src_ptr + 8, source_stride, ref_ptr + 8, recon_stride, &sse1, &sum1);

    var = sse0 + sse1;
    avg = sum0 + sum1;
    *sse = var;
    return (var - ((avg * avg) >> 7));

}

unsigned int vp8_variance8x16_wmt
(
    unsigned char *src_ptr,
    int  source_stride,
    unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    unsigned int sse0, sse1, var;
    int sum0, sum1, avg;

    vp8_get8x8var_sse2(src_ptr, source_stride, ref_ptr, recon_stride, &sse0, &sum0) ;
    vp8_get8x8var_sse2(src_ptr + 8 * source_stride, source_stride, ref_ptr + 8 * recon_stride, recon_stride, &sse1, &sum1) ;

    var = sse0 + sse1;
    avg = sum0 + sum1;
    *sse = var;
    return (var - ((avg * avg) >> 7));

}

///////////////////////////////////////////////////////////////////////////
// the mmx function that does the bilinear filtering and var calculation //
// int one pass                                                          //
///////////////////////////////////////////////////////////////////////////
DECLARE_ALIGNED(16, const short, vp8_bilinear_filters_xmm[8][16]) =
{
    { 128, 128, 128, 128, 128, 128, 128, 128,  0,  0,  0,  0,  0,  0,  0,  0 },
    { 112, 112, 112, 112, 112, 112, 112, 112, 16, 16, 16, 16, 16, 16, 16, 16 },
    {  96, 96, 96, 96, 96, 96, 96, 96, 32, 32, 32, 32, 32, 32, 32, 32 },
    {  80, 80, 80, 80, 80, 80, 80, 80, 48, 48, 48, 48, 48, 48, 48, 48 },
    {  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
    {  48, 48, 48, 48, 48, 48, 48, 48, 80, 80, 80, 80, 80, 80, 80, 80 },
    {  32, 32, 32, 32, 32, 32, 32, 32, 96, 96, 96, 96, 96, 96, 96, 96 },
    {  16, 16, 16, 16, 16, 16, 16, 16, 112, 112, 112, 112, 112, 112, 112, 112 }
};
unsigned int vp8_sub_pixel_variance4x4_wmt
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
    int xsum;
    unsigned int xxsum;
    vp8_filter_block2d_bil4x4_var_mmx(
        src_ptr, src_pixels_per_line,
        dst_ptr, dst_pixels_per_line,
        vp8_vp7_bilinear_filters_mmx[xoffset], vp8_vp7_bilinear_filters_mmx[yoffset],
        &xsum, &xxsum
    );
    *sse = xxsum;
    return (xxsum - ((xsum * xsum) >> 4));
}


unsigned int vp8_sub_pixel_variance8x8_wmt
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{

    int xsum;
    unsigned int xxsum;
    vp8_filter_block2d_bil_var_sse2(
        src_ptr, src_pixels_per_line,
        dst_ptr, dst_pixels_per_line, 8,
        vp8_bilinear_filters_xmm[xoffset], vp8_bilinear_filters_xmm[yoffset],
        &xsum, &xxsum
    );

    *sse = xxsum;
    return (xxsum - ((xsum * xsum) >> 6));
}

unsigned int vp8_sub_pixel_variance16x16_wmt
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
    int xsum0, xsum1;
    unsigned int xxsum0, xxsum1;


    // note we could avoid these if statements if the calling function
    // just called the appropriate functions inside.
    if (xoffset == 4 && yoffset == 0)
    {
        vp8_half_horiz_variance16x_h_sse2(
            src_ptr, src_pixels_per_line,
            dst_ptr, dst_pixels_per_line, 16,
            &xsum0, &xxsum0);

        vp8_half_horiz_variance16x_h_sse2(
            src_ptr + 8, src_pixels_per_line,
            dst_ptr + 8, dst_pixels_per_line, 16,
            &xsum1, &xxsum1);
    }
    else if (xoffset == 0 && yoffset == 4)
    {
        vp8_half_vert_variance16x_h_sse2(
            src_ptr, src_pixels_per_line,
            dst_ptr, dst_pixels_per_line, 16,
            &xsum0, &xxsum0);

        vp8_half_vert_variance16x_h_sse2(
            src_ptr + 8, src_pixels_per_line,
            dst_ptr + 8, dst_pixels_per_line, 16,
            &xsum1, &xxsum1);
    }
    else if (xoffset == 4 && yoffset == 4)
    {
        vp8_half_horiz_vert_variance16x_h_sse2(
            src_ptr, src_pixels_per_line,
            dst_ptr, dst_pixels_per_line, 16,
            &xsum0, &xxsum0);

        vp8_half_horiz_vert_variance16x_h_sse2(
            src_ptr + 8, src_pixels_per_line,
            dst_ptr + 8, dst_pixels_per_line, 16,
            &xsum1, &xxsum1);
    }
    else
    {
        vp8_filter_block2d_bil_var_sse2(
            src_ptr, src_pixels_per_line,
            dst_ptr, dst_pixels_per_line, 16,
            vp8_bilinear_filters_xmm[xoffset], vp8_bilinear_filters_xmm[yoffset],
            &xsum0, &xxsum0
        );


        vp8_filter_block2d_bil_var_sse2(
            src_ptr + 8, src_pixels_per_line,
            dst_ptr + 8, dst_pixels_per_line, 16,
            vp8_bilinear_filters_xmm[xoffset], vp8_bilinear_filters_xmm[yoffset],
            &xsum1, &xxsum1
        );
    }

    xsum0 += xsum1;
    xxsum0 += xxsum1;
    *sse = xxsum0;
    return (xxsum0 - ((xsum0 * xsum0) >> 8));
}

unsigned int vp8_sub_pixel_mse16x16_wmt(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
    vp8_sub_pixel_variance16x16_wmt(src_ptr, src_pixels_per_line, xoffset, yoffset, dst_ptr, dst_pixels_per_line, sse);
    return *sse;
}

unsigned int vp8_sub_pixel_variance16x8_wmt
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse

)
{
    int xsum0, xsum1;
    unsigned int xxsum0, xxsum1;


    vp8_filter_block2d_bil_var_sse2(
        src_ptr, src_pixels_per_line,
        dst_ptr, dst_pixels_per_line, 8,
        vp8_bilinear_filters_xmm[xoffset], vp8_bilinear_filters_xmm[yoffset],
        &xsum0, &xxsum0
    );


    vp8_filter_block2d_bil_var_sse2(
        src_ptr + 8, src_pixels_per_line,
        dst_ptr + 8, dst_pixels_per_line, 8,
        vp8_bilinear_filters_xmm[xoffset], vp8_bilinear_filters_xmm[yoffset],
        &xsum1, &xxsum1
    );

    xsum0 += xsum1;
    xxsum0 += xxsum1;

    *sse = xxsum0;
    return (xxsum0 - ((xsum0 * xsum0) >> 7));
}

unsigned int vp8_sub_pixel_variance8x16_wmt
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
    int xsum;
    unsigned int xxsum;
    vp8_filter_block2d_bil_var_sse2(
        src_ptr, src_pixels_per_line,
        dst_ptr, dst_pixels_per_line, 16,
        vp8_bilinear_filters_xmm[xoffset], vp8_bilinear_filters_xmm[yoffset],
        &xsum, &xxsum
    );

    *sse = xxsum;
    return (xxsum - ((xsum * xsum) >> 7));
}

unsigned int vp8_i_variance16x16_wmt(
    unsigned char *src_ptr,
    int  source_stride,
    unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    unsigned int sse0, sse1, sse2, sse3, var;
    int sum0, sum1, sum2, sum3, avg;


    vp8_get8x8var_sse2(src_ptr, source_stride, ref_ptr, recon_stride, &sse0, &sum0) ;
    vp8_get8x8var_sse2(src_ptr + 8, source_stride, ref_ptr + 8, recon_stride, &sse1, &sum1);
    vp8_get8x8var_sse2(src_ptr + (source_stride >> 1), source_stride, ref_ptr + (recon_stride >> 1), recon_stride, &sse2, &sum2) ;
    vp8_get8x8var_sse2(src_ptr + (source_stride >> 1) + 8, source_stride, ref_ptr + (recon_stride >> 1) + 8, recon_stride, &sse3, &sum3);

    var = sse0 + sse1 + sse2 + sse3;
    avg = sum0 + sum1 + sum2 + sum3;

    *sse = var;
    return (var - ((avg * avg) >> 8));

}

unsigned int vp8_i_variance8x16_wmt(
    unsigned char *src_ptr,
    int  source_stride,
    unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    unsigned int sse0, sse1, var;
    int sum0, sum1, avg;
    vp8_get8x8var_sse2(src_ptr, source_stride, ref_ptr, recon_stride, &sse0, &sum0) ;
    vp8_get8x8var_sse2(src_ptr + (source_stride >> 1), source_stride, ref_ptr + (recon_stride >> 1), recon_stride, &sse1, &sum1) ;

    var = sse0 + sse1;
    avg = sum0 + sum1;

    *sse = var;
    return (var - ((avg * avg) >> 7));

}


unsigned int vp8_i_sub_pixel_variance16x16_wmt
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
    return vp8_sub_pixel_variance16x16_wmt(src_ptr, (src_pixels_per_line >> 1), xoffset, yoffset, dst_ptr, (dst_pixels_per_line >> 1), sse);
}


unsigned int vp8_i_sub_pixel_variance8x16_wmt
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{

    return vp8_sub_pixel_variance8x16_wmt(src_ptr, (src_pixels_per_line >> 1), xoffset, yoffset, dst_ptr, (dst_pixels_per_line >> 1), sse);
}
