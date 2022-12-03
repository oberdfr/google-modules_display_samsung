/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Headef file for Display Quality Enhancer.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __EXYNOS_DRM_DQE_H__
#define __EXYNOS_DRM_DQE_H__

#include <drm/samsung_drm.h>
#include <dqe_cal.h>
#include <cal_config.h>

struct decon_device;
struct exynos_dqe;
struct exynos_dqe_state;

/* internal histogram callback function */
typedef void (*histogram_chan_callback)(u32 dqe_id, enum exynos_histogram_id hist_id,
					struct histogram_bins *hist_bins);

struct exynos_dqe_funcs {
	void (*update)(struct exynos_dqe *dqe, struct exynos_dqe_state *state,
			u32 width, u32 height);
};

struct exynos_dqe_state {
	const struct drm_color_lut *degamma_lut;
	const struct exynos_matrix *linear_matrix;
	const struct exynos_matrix *gamma_matrix;
	const struct cgc_lut *cgc_lut;
	struct drm_color_lut *regamma_lut;
	struct dither_config *disp_dither_config;
	struct dither_config *cgc_dither_config;
	bool enabled;
	bool rcd_enabled;
	struct drm_gem_object *cgc_gem;

	struct histogram_roi *roi;
	struct histogram_weights *weights;
	struct histogram_bins *bins;
	struct exynos_drm_pending_histogram_event *event;
	u32 histogram_threshold;
	spinlock_t histogram_slock;
	enum exynos_prog_pos histogram_pos;
	enum exynos_histogram_id histogram_id;

	/* track per channel h/w histogram */
	struct {
		enum histogram_state hist_state;
		struct histogram_bins hist_bins;
		histogram_chan_callback hist_cb;
	} hist_chan[HISTOGRAM_MAX];
};

struct dither_debug_override {
	bool force_en;
	bool verbose;
	struct dither_config val;
};

#define MAX_NAME_SIZE		32
struct debugfs_lut {
	void *lut_ptr;
	struct drm_color_lut *dlut_ptr;
	char name[MAX_NAME_SIZE];
	enum elem_size elem_size;
	size_t count;
	size_t pcount;
	bool *dirty;
};

struct exynos_debug_info {
	bool force_en;
	bool verbose;
	bool dirty;
};

struct degamma_debug_override {
	struct exynos_debug_info info;
	struct drm_color_lut force_lut[DEGAMMA_LUT_SIZE];
};

struct regamma_debug_override {
	struct exynos_debug_info info;
	struct drm_color_lut force_lut[REGAMMA_LUT_SIZE];
};

struct cgc_debug_override {
	bool first_write;
	u32 verbose_cnt;
	struct exynos_debug_info info;
	struct cgc_lut force_lut;
};

struct matrix_debug_override {
	struct exynos_debug_info info;
	struct exynos_matrix force_matrix;
};

enum dump_type {
	DUMP_TYPE_CGC_DIHTER	= 0,
	DUMP_TYPE_DISP_DITHER,
	DUMP_TYPE_DEGAMMA_LUT,
	DUMP_TYPE_REGAMMA_LUT,
	DUMP_TYPE_CGC_LUT,
	DUMP_TYPE_LINEAR_MATRIX,
	DUMP_TYPE_GAMMA_MATRIX,
	DUMP_TYPE_HISTOGRAM,
	DUMP_TYPE_ATC,
	DUMP_TYPE_DQE_MAX	= DUMP_TYPE_ATC,
	DUMP_TYPE_HDR_EOTF,
	DUMP_TYPE_HDR_OETF,
	DUMP_TYPE_HDR_GAMMUT,
	DUMP_TYPE_HDR_TONEMAP,
};

struct debugfs_dump {
	enum dump_type type;
	u32 id;
	enum dqe_dither_type dither_type;
	void *priv;
};

/* internal histogram channel control functions */
enum histogram_chan_config_flags {
	HIST_CHAN_THRESHOLD = 1U << 0,
	HIST_CHAN_POS = 1U << 1,
	HIST_CHAN_ROI = 1U << 2,
	HIST_CHAN_WEIGHTS = 1U << 3,
};

struct histogram_chan_config {
	u32 threshold;
	enum exynos_prog_pos pos;
	struct histogram_roi roi;
	struct histogram_weights weights;
};

struct exynos_dqe {
	void __iomem *regs;
	void __iomem *cgc_regs;
	bool initialized;
	const struct exynos_dqe_funcs *funcs;
	struct exynos_dqe_state state;
	struct decon_device *decon;
	struct class *dqe_class;
	struct device *dev;

	struct dither_debug_override cgc_dither_override;
	struct dither_debug_override disp_dither_override;

	struct degamma_debug_override degamma;
	struct regamma_debug_override regamma;
	struct cgc_debug_override cgc;
	struct matrix_debug_override gamma;
	struct matrix_debug_override linear;

	bool verbose_hist;

	bool force_disabled;

	bool verbose_atc;
	bool dstep_changed;
	struct exynos_atc force_atc_config;
	u32 lpd_atc_regs[LPD_ATC_REG_CNT];
	struct histogram_chan_config lhbm_hist_config;
	int lhbm_gray_level;
};

int histogram_request_ioctl(struct drm_device *drm_dev, void *data,
				struct drm_file *file);
int histogram_cancel_ioctl(struct drm_device *drm_dev, void *data,
				struct drm_file *file);
void handle_histogram_event(struct exynos_dqe *dqe);
void exynos_dqe_update(struct exynos_dqe *dqe, struct exynos_dqe_state *state,
			u32 width, u32 height);
void exynos_dqe_reset(struct exynos_dqe *dqe);
struct exynos_dqe *exynos_dqe_register(struct decon_device *decon);
void exynos_dqe_save_lpd_data(struct exynos_dqe *dqe);
void exynos_dqe_restore_lpd_data(struct exynos_dqe *dqe);

int histogram_chan_configure(struct exynos_dqe *dqe, const enum exynos_histogram_id hist_id,
			     struct histogram_chan_config *config, u32 flags);
int histogram_chan_start(struct exynos_dqe *dqe, const enum exynos_histogram_id hist_id,
			 const enum histogram_state hist_state, histogram_chan_callback hist_cb);
int histogram_chan_stop(struct exynos_dqe *dqe, const enum exynos_histogram_id hist_id);
#endif /* __EXYNOS_DRM_DQE_H__ */
