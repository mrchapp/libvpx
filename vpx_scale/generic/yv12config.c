/*
 *  Copyright (c) 2010 The VP8 project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license 
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may 
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vpx_scale/yv12config.h"
#include "vpx_mem/vpx_mem.h"

#if 1
#  include <stdio.h>
#  define TRACE(fmt,...) printf("%s:%d\t%s()\t"fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#  define TRACE(fmt,...)
#endif

/****************************************************************************
*  Exports
****************************************************************************/

/****************************************************************************
 *
 ****************************************************************************/
int
vp8_yv12_de_alloc_frame_buffer(YV12_BUFFER_CONFIG **ybfp)
{
    YV12_BUFFER_CONFIG *ybf;

    TRACE("");

    if (!ybfp)
        return -1;

    ybf = *ybfp;
    *ybfp = NULL;

    TRACE("ybf=%p", ybf);

    if (ybf)
    {
        if (ybf->buffer_alloc)
        {
            duck_free(ybf->buffer_alloc);
            TRACE("freed buffer_alloc");
       }

        ybf->buffer_alloc = 0;

        duck_free(ybf);

        TRACE("freed ybf");
    }
    else
    {
        return -1;
    }

    return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
int
vp8_yv12_alloc_frame_buffer(YV12_BUFFER_CONFIG **ybfp, int width, int height, int border)
{
//NOTE:

    int yplane_size = (height + 2 * border) * (width + 2 * border);
    int uvplane_size = ((1 + height) / 2 + border) * ((1 + width) / 2 + border);

    TRACE("width=%d, height=%d, border=%d", width, height, border);

    if (ybfp)
    {
        YV12_BUFFER_CONFIG *ybf = duck_calloc(1, sizeof(YV12_BUFFER_CONFIG), DMEM_GENERAL);

        if (*ybfp)
            vp8_yv12_de_alloc_frame_buffer(ybfp);

        *ybfp = ybf;

        TRACE("ybf=%p", ybf);

        ybf->y_width  = width;
        ybf->y_height = height;
        ybf->y_stride = width + 2 * border;

        ybf->uv_width = (1 + width) / 2;
        ybf->uv_height = (1 + height) / 2;
        ybf->uv_stride = ybf->uv_width + border;

        ybf->border = border;
        ybf->frame_size = yplane_size + 2 * uvplane_size;

        // Added 2 extra lines to framebuffer so that copy12x12 doesn't fail
        // when we have a large motion vector in V on the last v block.
        // Note : We never use these pixels anyway so this doesn't hurt.
        ybf->buffer_alloc = (unsigned char *) duck_memalign(32,  ybf->frame_size + (ybf->y_stride * 2) + 32, 0);

        if (ybf->buffer_alloc == NULL)
            return -1;

        ybf->y_buffer = ybf->buffer_alloc + (border * ybf->y_stride) + border;

        if (yplane_size & 0xf)
            yplane_size += 16 - (yplane_size & 0xf);

        ybf->u_buffer = ybf->buffer_alloc + yplane_size + (border / 2  * ybf->uv_stride) + border / 2;
        ybf->v_buffer = ybf->buffer_alloc + yplane_size + uvplane_size + (border / 2  * ybf->uv_stride) + border / 2;

        ybf->refcnt = 0;
    }
    else
    {
        return -2;
    }

    return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
int
vp8_yv12_black_frame_buffer(YV12_BUFFER_CONFIG *ybf)
{
    if (ybf)
    {
        if (ybf->buffer_alloc)
        {
            duck_memset(ybf->y_buffer, 0x0, ybf->y_stride * ybf->y_height);
            duck_memset(ybf->u_buffer, 0x80, ybf->uv_stride * ybf->uv_height);
            duck_memset(ybf->v_buffer, 0x80, ybf->uv_stride * ybf->uv_height);
        }

        return 0;
    }

    return -1;
}

YV12_BUFFER_CONFIG *
vp8_yv12_ref(YV12_BUFFER_CONFIG *ybf)
{
    TRACE("ybf=%p, refcnt=%d", ybf, ybf ? ybf->refcnt : 0);

    if (!ybf)
        return NULL;

    // XXX probably need atomic functions:
    ybf->refcnt++;
    return ybf;
}

void vp8_yv12_unref(YV12_BUFFER_CONFIG *ybf)
{
    if (!ybf)
        return;

    TRACE("ybf=%p, refcnt=%d", ybf, ybf ? ybf->refcnt : 0);

    // XXX probably need atomic functions:
    if (!ybf->refcnt)
    {
        vp8_yv12_de_alloc_frame_buffer(&ybf);
    }
    else
    {
        ybf->refcnt--;
    }
}

YV12_BUFFER_CONFIG *
vp8_yv12_make_writable(YV12_BUFFER_CONFIG *ybf)
{
    TRACE("ybf=%p, refcnt=%d", ybf, ybf ? ybf->refcnt : 0);

    // XXX probably need atomic functions:
    if (ybf && ybf->refcnt)
    {
        YV12_BUFFER_CONFIG *new_ybf = NULL;
        if (vp8_yv12_alloc_frame_buffer(&new_ybf, ybf->y_width, ybf->y_height, ybf->border) == 0)
        {
#if 0
            // XXX do I need to copy contents?  I hope not.
            extern void (*vp8_yv12_copy_frame_ptr)(YV12_BUFFER_CONFIG *src_ybc, YV12_BUFFER_CONFIG *dst_ybc);
            TRACE("vp8_yv12_copy_frame_ptr=%p, ybf=%p, new_ybf=%p", vp8_yv12_copy_frame_ptr, ybf, new_ybf);
            vp8_yv12_copy_frame_ptr(ybf, new_ybf);
#endif
            vp8_yv12_unref(ybf);
        }
        ybf = new_ybf;
    }

    return ybf;
}


