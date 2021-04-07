/* SPDX-License-Identifier: GPL-2.0-only
 *
 * linux/drivers/gpu/drm/samsung/exynos_drm_decon.h
 *
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Headef file for Samsung MIPI DSI Master driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __EXYNOS_DRM_DECON_H__
#define __EXYNOS_DRM_DECON_H__

#include <linux/of_gpio.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/pm_runtime.h>
#include <linux/spinlock.h>
#if IS_ENABLED(CONFIG_EXYNOS_PM_QOS) || IS_ENABLED(CONFIG_EXYNOS_PM_QOS_MODULE)
#include <soc/google/exynos_pm_qos.h>
#endif
#include <linux/notifier.h>
#if IS_ENABLED(CONFIG_EXYNOS_ITMON)
#include <soc/google/exynos-itmon.h>
#endif
#include <soc/google/bts.h>
#include <drm/drm_device.h>
#include <video/videomode.h>

#include <decon_cal.h>

#include "exynos_drm_dpp.h"
#include "exynos_drm_dqe.h"
#include "exynos_drm_drv.h"
#include "exynos_drm_dsim.h"

#include "exynos_drm_fb.h"
#include "exynos_drm_hibernation.h"
#include "exynos_drm_writeback.h"
#include "exynos_drm_partial.h"

enum decon_state {
	DECON_STATE_INIT = 0,
	DECON_STATE_ON,
	DECON_STATE_HIBERNATION,
	DECON_STATE_OFF,
};

enum dpu_win_state {
	DPU_WIN_STATE_DISABLED = 0,
	DPU_WIN_STATE_COLOR,
	DPU_WIN_STATE_BUFFER,
};

struct decon_resources {
	struct clk *aclk;
	struct clk *aclk_disp;
};

struct dpu_bts_ops {
	void (*init)(struct decon_device *decon);
	void (*release_bw)(struct decon_device *decon);
	void (*calc_bw)(struct decon_device *decon);
	void (*update_bw)(struct decon_device *decon, bool shadow_updated);
	void (*deinit)(struct decon_device *decon);
};

struct dpu_bts_bw {
	u32 val;
	u32 ch_num;
};

struct dpu_bts_win_config {
	enum dpu_win_state state;
	u32 src_x;
	u32 src_y;
	u32 src_w;
	u32 src_h;
	int dst_x;
	int dst_y;
	u32 dst_w;
	u32 dst_h;
	bool is_rot;
	bool is_comp;
	int dpp_ch;
	u32 format;
	u64 comp_src;
};

struct bts_layer_position {
	u32 x1;
	u32 x2; /* x2 = x1 + width */
	u32 y1;
	u32 y2; /* y2 = y1 + height */
};

struct bts_dpp_info {
	u32 bpp;
	u32 src_h;
	u32 src_w;
	struct bts_layer_position dst;
	u32 bw;
	u32 rt_bw;
	bool rotation;
	bool is_afbc;
	bool is_yuv;
};

struct bts_decon_info {
	struct bts_dpp_info rdma[MAX_WIN_PER_DECON];
	struct bts_dpp_info odma;
	u32 vclk; /* Khz */
	u32 lcd_w;
	u32 lcd_h;
};

struct dpu_bts {
	bool enabled;
	u32 resol_clk;
	u32 peak;
	u32 prev_peak;
	u32 rt_avg_bw;
	u32 prev_rt_avg_bw;
	u32 read_bw;
	u32 write_bw;
	u32 total_bw;
	u32 prev_total_bw;
	u32 max_disp_freq;
	u32 prev_max_disp_freq;
	u32 dvfs_max_disp_freq;
	u64 ppc;
	u32 ppc_rotator;
	u32 ppc_scaler;
	u32 delay_comp;
	u32 delay_scaler;
	u32 bus_width;
	u32 bus_util_pct;
	u32 rot_util_pct;
	u32 afbc_rgb_util_pct;
	u32 afbc_yuv_util_pct;
	u32 dfs_lv_cnt;
	u32 dfs_lv_khz[BTS_DFS_MAX];
	u32 vbp;
	u32 vfp;
	u32 vsa;
	u32 fps;
	/* includes writeback dpp */
	struct dpu_bts_bw rt_bw[MAX_DPP_CNT];

	/* each decon must know other decon's BW to get overall BW */
	u32 ch_bw[3][MAX_DECON_CNT];
	int bw_idx;
	struct dpu_bts_ops *ops;
#if IS_ENABLED(CONFIG_EXYNOS_PM_QOS) || IS_ENABLED(CONFIG_EXYNOS_PM_QOS_MODULE)
	struct exynos_pm_qos_request mif_qos;
	struct exynos_pm_qos_request int_qos;
	struct exynos_pm_qos_request disp_qos;
#endif

	struct dpu_bts_win_config win_config[MAX_WIN_PER_DECON];
	struct dpu_bts_win_config wb_config;
	atomic_t delayed_update;
};

/**
 * Display Subsystem event management status.
 *
 * These status labels are used internally by the DECON to indicate the
 * current status of a device with operations.
 */
enum dpu_event_type {
	DPU_EVT_NONE = 0,

	DPU_EVT_DECON_ENABLED,
	DPU_EVT_DECON_DISABLED,
	DPU_EVT_DECON_FRAMEDONE,
	DPU_EVT_DECON_FRAMESTART,
	DPU_EVT_DECON_RSC_OCCUPANCY,
	DPU_EVT_DECON_TRIG_MASK,

	DPU_EVT_DSIM_ENABLED,
	DPU_EVT_DSIM_DISABLED,
	DPU_EVT_DSIM_COMMAND,
	DPU_EVT_DSIM_ULPS_ENTER,
	DPU_EVT_DSIM_ULPS_EXIT,
	DPU_EVT_DSIM_UNDERRUN,
	DPU_EVT_DSIM_FRAMEDONE,

	DPU_EVT_DPP_FRAMEDONE,
	DPU_EVT_DMA_RECOVERY,

	DPU_EVT_ATOMIC_COMMIT,
	DPU_EVT_TE_INTERRUPT,

	DPU_EVT_ENTER_HIBERNATION_IN,
	DPU_EVT_ENTER_HIBERNATION_OUT,
	DPU_EVT_EXIT_HIBERNATION_IN,
	DPU_EVT_EXIT_HIBERNATION_OUT,

	DPU_EVT_ATOMIC_BEGIN,
	DPU_EVT_ATOMIC_FLUSH,

	DPU_EVT_WB_ENABLE,
	DPU_EVT_WB_DISABLE,
	DPU_EVT_WB_ATOMIC_COMMIT,
	DPU_EVT_WB_FRAMEDONE,
	DPU_EVT_WB_ENTER_HIBERNATION,
	DPU_EVT_WB_EXIT_HIBERNATION,

	DPU_EVT_PLANE_UPDATE,
	DPU_EVT_PLANE_DISABLE,

	DPU_EVT_REQ_CRTC_INFO_OLD,
	DPU_EVT_REQ_CRTC_INFO_NEW,

	DPU_EVT_FRAMESTART_TIMEOUT,

	DPU_EVT_BTS_RELEASE_BW,
	DPU_EVT_BTS_CALC_BW,
	DPU_EVT_BTS_UPDATE_BW,

	DPU_EVT_PARTIAL_INIT,
	DPU_EVT_PARTIAL_PREPARE,
	DPU_EVT_PARTIAL_UPDATE,
	DPU_EVT_PARTIAL_RESTORE,

	DPU_EVT_DSIM_CRC,
	DPU_EVT_DSIM_ECC,

	DPU_EVT_VBLANK_ENABLE,
	DPU_EVT_VBLANK_DISABLE,

	DPU_EVT_DIMMING_START,
	DPU_EVT_DIMMING_END,

	DPU_EVT_MAX, /* End of EVENT */
};

enum dpu_event_condition {
	DPU_EVT_CONDITION_ALL = 0,
	DPU_EVT_CONDITION_UNDERRUN,
	DPU_EVT_CONDITION_FAIL_UPDATE_BW,
};

#define DPU_CALLSTACK_MAX 10
struct dpu_log_dsim_cmd {
	u8 id;
	u8 d0;
	u16 len;
	void *caller[DPU_CALLSTACK_MAX];
};

struct dpu_log_dpp {
	u32 id;
	u64 comp_src;
	u32 recovery_cnt;
};

struct dpu_log_win {
	u32 win_idx;
	u32 plane_idx;
};

struct dpu_log_rsc_occupancy {
	u32 rsc_ch;
	u32 rsc_win;
};

struct decon_win_config {
	struct dpu_bts_win_config win;
	dma_addr_t dma_addr;
};

struct dpu_log_atomic {
	struct decon_win_config win_config[MAX_WIN_PER_DECON];
};

/* Event log structure for DPU power domain status */
struct dpu_log_pd {
	bool rpm_active;
};

struct dpu_log_crtc_info {
	bool enable;
	bool active;
	bool planes_changed;
	bool mode_changed;
	bool active_changed;
	bool self_refresh;
};

struct dpu_log_freqs {
	unsigned long mif_freq;
	unsigned long int_freq;
	unsigned long disp_freq;
};

struct dpu_log_bts_update {
	struct dpu_log_freqs freqs;
	u32 peak;
	u32 prev_peak;
	u32 rt_avg_bw;
	u32 prev_rt_avg_bw;
	u32 total_bw;
	u32 prev_total_bw;
};

struct dpu_log_bts_cal {
	struct dpu_log_freqs freqs;
	u32 disp_freq;
	u32 peak;
	u32 rt_avg_bw;
	u32 read_bw;
	u32 write_bw;
	u32 fps;
};

struct dpu_log_bts_event {
	struct dpu_log_freqs freqs;
	u32 value;
};

struct dpu_log_partial {
	u32 min_w;
	u32 min_h;
	struct drm_rect prev;
	struct drm_rect req;
	struct drm_rect adj;
	bool reconfigure;
};

struct dpu_log {
	ktime_t time;
	enum dpu_event_type type;

	union {
		struct dpu_log_dpp dpp;
		struct dpu_log_atomic atomic;
		struct dpu_log_dsim_cmd cmd;
		struct dpu_log_rsc_occupancy rsc;
		struct dpu_log_pd pd;
		struct dpu_log_win win;
		struct dpu_log_crtc_info crtc_info;
		struct dpu_log_freqs freqs;
		struct dpu_log_bts_update bts_update;
		struct dpu_log_bts_cal bts_cal;
		struct dpu_log_bts_event bts_event;
		struct dpu_log_partial partial;
		unsigned int value;
	} data;
};

/* Definitions below are used in the DECON */
#define DPU_EVENT_LOG_RETRY	3
#define DPU_EVENT_KEEP_CNT	3

struct decon_debug {
	/* ring buffer of event log */
	struct dpu_log *event_log;
	/* count of log buffers in each event log */
	u32 event_log_cnt;
	/* count of underrun interrupt */
	u32 underrun_cnt;
	/* count of crc interrupt */
	u32 crc_cnt;
	/* count of ecc interrupt */
	u32 ecc_cnt;
	/* array index of log buffer in event log */
	atomic_t event_log_idx;
	/* lock for saving log to event log buffer */
	spinlock_t event_lock;

	u32 auto_refresh_frames;
};

struct decon_device {
	u32				id;
	enum decon_state		state;
	struct decon_regs		regs;
	struct device			*dev;
	struct drm_device		*drm_dev;
	struct exynos_drm_crtc		*crtc;
	/* dpp information saved in dpp channel number order */
	struct dpp_device		*dpp[MAX_WIN_PER_DECON];
	struct dpp_device		*rcd;
	u32				dpp_cnt;
	u32				win_cnt;
	enum exynos_drm_output_type	con_type;
	struct decon_config		config;
	struct decon_resources		res;
	struct dpu_bts			bts;
	struct decon_debug		d;
	struct exynos_hibernation	*hibernation;
	struct drm_pending_vblank_event *event;
	struct exynos_dqe		*dqe;
	struct task_struct		*thread;
	struct kthread_worker		worker;
	struct kthread_work		early_wakeup_work;

	u32				irq_fs;	/* frame start irq number*/
	u32				irq_fd;	/* frame done irq number*/
	u32				irq_ext;/* extra irq number*/
	int				irq_te;
	int				irq_ds;	/* dimming start irq number */
	int				irq_de;	/* dimming end irq number */

	spinlock_t			slock;

#if IS_ENABLED(CONFIG_EXYNOS_ITMON)
	struct notifier_block itmon_nb;
	bool itmon_notified;
#endif

	wait_queue_head_t framedone_wait;

	bool keep_unmask;
	struct exynos_partial *partial;
};

extern struct dpu_bts_ops dpu_bts_control;
extern struct decon_device *decon_drvdata[MAX_DECON_CNT];

static inline struct decon_device *get_decon_drvdata(u32 id)
{
	if (id >= 0 && id < MAX_DECON_CNT)
		return decon_drvdata[id];

	return NULL;
}

void decon_dump(struct decon_device *decon);
void decon_dump_all(struct decon_device *decon);
void decon_dump_event_condition(const struct decon_device *decon,
		enum dpu_event_condition condition);
int dpu_init_debug(struct decon_device *decon);
void DPU_EVENT_LOG(enum dpu_event_type type, int index, void *priv);
void DPU_EVENT_LOG_ATOMIC_COMMIT(int index);
void DPU_EVENT_LOG_CMD(struct dsim_device *dsim, u8 type, u8 d0, u16 len);

#if IS_ENABLED(CONFIG_EXYNOS_ITMON)
int dpu_itmon_notifier(struct notifier_block *nb, unsigned long action,
		void *data);
#endif

static inline struct drm_encoder *
crtc_find_first_encoder_by_type(const struct drm_crtc_state *crtc_state, u32 encoder_type)
{
	const struct drm_crtc *crtc = crtc_state->crtc;
	const struct drm_device *dev = crtc->dev;
	struct drm_encoder *encoder;

	drm_for_each_encoder_mask(encoder, dev, crtc_state->encoder_mask)
		if (encoder->crtc == crtc && encoder->encoder_type == encoder_type)
			return encoder;

	return NULL;
}

static inline struct drm_encoder*
decon_get_encoder(const struct decon_device *decon, u32 encoder_type)
{
	const struct drm_crtc *crtc = &decon->crtc->base;

	if (!crtc->state)
		return NULL;

	return crtc_find_first_encoder_by_type(crtc->state, encoder_type);
}

static inline struct dsim_device*
decon_get_dsim(struct decon_device *decon)
{
	struct drm_encoder *encoder;

	encoder = decon_get_encoder(decon, DRM_MODE_ENCODER_DSI);
	if (!encoder)
		return NULL;

	return container_of(encoder, struct dsim_device, encoder);
}

static inline struct writeback_device*
decon_get_wb(struct decon_device *decon)
{
	struct drm_encoder *encoder;
	struct drm_writeback_connector *wb_connector;

	encoder = decon_get_encoder(decon, DRM_MODE_ENCODER_VIRTUAL);
	if (!encoder)
		return NULL;

	wb_connector = container_of(encoder, struct drm_writeback_connector,
			encoder);

	if (!wb_connector)
		return NULL;

	return container_of(wb_connector, struct writeback_device, writeback);
}

#define crtc_to_decon(crtc)                                                    \
	(container_of((crtc), struct exynos_drm_crtc, base)->ctx)
static inline bool is_power_on(struct drm_device *drm_dev)
{
	struct drm_crtc *crtc;
	struct decon_device *decon;
	bool ret = false;

	drm_for_each_crtc(crtc, drm_dev) {
		decon = crtc_to_decon(crtc);
		ret |= pm_runtime_active(decon->dev);
	}

	return ret;
}

#endif /* __EXYNOS_DRM_DECON_H__ */
