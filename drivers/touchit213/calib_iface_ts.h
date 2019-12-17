#ifndef __DRIVERS_TOUCHSCREEN_CALIBRATION_TS_INTERFACE_H__
#define __DRIVERS_TOUCHSCREEN_CALIBRATION_TS_INTERFACE_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/err.h>

/*
 * Per-touchscreen data.
 */
struct axis {
	unsigned int xMax;
	unsigned int yMax;
};

struct calibration {
	int calibindex;
	bool isCalibrateing;
	bool isAlreadyCalibrate;
	
	int screen_x[5];
	int screen_y[5];
	int uncali_x[5];
	int uncali_y[5];

	//extern volatile struct adc_point gADPoint;
	struct axis screen;
    struct axis ADPoint;
	
	// extern module private object.
	const void *pdata;
	int (*reinput)(void *pdata, const int xMax, const int yMax);
};

int tp_calib_iface_init(struct calibration *, struct axis *saxis);
void tp_calib_iface_exit(void);

#endif //__DRIVERS_TOUCHSCREEN_CALIBRATION_TS_INTERFACE_H__
