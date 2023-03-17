/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef DRM_FOURCC_GS101_H
#define DRM_FOURCC_GS101_H

#include <drm/drm_fourcc.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * 2 plane packed YCbCr
 * 2x2 subsampled Cr:Cb plane 10 bits per channel
 * index 0 = Y plane, [9:0] Y [10] little endian
 * index 1 = Cr:Cb plane, [19:0] Cr:Cb [10:10] little endian
 */
#define DRM_FORMAT_Y010		fourcc_code('Y', '0', '1', '0')

/*
 * Set to access the secure buffer
 *
 * The secure buffer is used to store DRM(Digital Right Management) contents.
 * DMA needs special authority to access the secure buffer. This modifier can
 * be set to allow the DMA to access the secure buffer. This can be used in
 * combination with another modifier.
 */
#define DRM_FORMAT_MOD_PROTECTION	fourcc_mod_code(NONE, (1ULL << 51))

/*
 * 4 plane YCbCr 4:2:0 10 bits per channel
 * index 0: Y8 plane, [7:0] Y little endian
 * index 1: Cr8:Cb8 plane, [15:0] CrCb little endian
 * index 2: Y2 plane, [1:0] Y little endian
 * index 3: Cr2:Cb2 plane, [3:0] CrCb little endian
 */
#define DRM_FORMAT_MOD_SAMSUNG_YUV_8_2_SPLIT	fourcc_mod_code(SAMSUNG, 3)

/*
 * The colormap uses the color data generated by hardware instead of reading
 * the data from the memory.
 *
 * It supports only solid color in BGRA8888 format. When it is used as
 * a modifier, BGRA8888 format should be used and color value is passed through
 * first handles[0].
 */
#define DRM_FORMAT_MOD_SAMSUNG_COLORMAP		fourcc_mod_code(SAMSUNG, 4)

/*
 * Samsung Band Width Compression (SBWC) modifier
 *
 * SBWC is a specific lossless or lossy image compression protocol and format.
 * It supports video image (YUV) compression to reduce the amount of data
 * transferred between IP blocks. This modifier is used when to decode data or
 * when to encode data through writeback.
 */
#define SBWC_IDENTIFIER				(1 << 4)
#define SBWC_FORMAT_MOD_BLOCK_SIZE_MASK		(0xfULL << 5)
#define SBWC_BLOCK_SIZE_SET(blk_size)		\
		(((blk_size) << 5) & SBWC_FORMAT_MOD_BLOCK_SIZE_MASK)
#define SBWC_BLOCK_SIZE_GET(modifier)		\
		(((modifier) & SBWC_FORMAT_MOD_BLOCK_SIZE_MASK) >> 5)
#define SBWC_FORMAT_MOD_BLOCK_SIZE_32x2		(2ULL)
#define SBWC_FORMAT_MOD_BLOCK_SIZE_32x3		(3ULL)
#define SBWC_FORMAT_MOD_BLOCK_SIZE_32x4		(4ULL)
#define SBWC_FORMAT_MOD_BLOCK_SIZE_32x5		(5ULL)
#define SBWC_FORMAT_MOD_BLOCK_SIZE_32x6		(6ULL)

#define SBWC_FORMAT_MOD_LOSSY			(1 << 12)

#define DRM_FORMAT_MOD_SAMSUNG_SBWC(blk_size)	\
		fourcc_mod_code(SAMSUNG,	\
		(SBWC_BLOCK_SIZE_SET(blk_size) | SBWC_IDENTIFIER))

/* from 52 to 55 bit are reserved for AFBC encoder source informaton */
#define AFBC_FORMAT_MOD_SOURCE_MASK	(0xfULL << 52)
#define AFBC_FORMAT_MOD_SOURCE_GPU	(1ULL << 52)
#define AFBC_FORMAT_MOD_SOURCE_G2D	(2ULL << 52)
#define AFBC_BLOCK_SIZE_GET(modifier)	(modifier & AFBC_FORMAT_MOD_BLOCK_SIZE_MASK)

#if defined(__cplusplus)
}
#endif

#endif /* DRM_FOURCC_GS101_H */
