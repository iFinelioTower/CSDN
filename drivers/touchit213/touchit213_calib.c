/*
 * Sahara TouchIT-213 serial touchscreen driver
 *
 * Copyright (c) 2007-2008 Claudio Nieder <private@claudio.ch>
 *
 * Based on Touchright driver (drivers/input/touchscreen/touchright.c)
 * Copyright (c) 2006 Rick Koch <n1gp@hotmail.com>
 * Copyright (c) 2004 Vojtech Pavlik
 * and Dan Streetman <ddstreet@ieee.org>
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include <linux/init.h>
#include <linux/fs.h>

#define ENABLE_CALIBRATION
#ifdef ENABLE_CALIBRATION
#include "calib_iface_ts.h"
#include "calibration_ts.h"
#endif

#define DRVICE_NAME "touchit213"
#define DRIVER_DESC "Sahara TouchIT-213 serial touchscreen driver"

//#define DEBUG_IRQ
//#define DEBUG_DATA
//#define DEBUG_SWAP

#define DBG_PRINTK(fmt, args...) \
	do { \
		printk(KERN_INFO "<"DRVICE_NAME">: "fmt, ##args); \
	} while (0)

/*
 * Definitions & global arrays.
 */

/*
 * Data is received through COM1 at 9600bit/s,8bit,no parity in packets
 * of 5 byte each.
 *
 *   +--------+   +--------+   +--------+   +--------+   +--------+
 *   |1000000p|   |0xxxxxxx|   |0xxxxxxx|   |0yyyyyyy|   |0yyyyyyy|
 *   +--------+   +--------+   +--------+   +--------+   +--------+
 *                    MSB          LSB          MSB          LSB
 *
 * The value of p is 1 as long as the screen is touched and 0 when
 * reporting the location where touching stopped, e.g. where the pen was
 * lifted from the screen.
 *
 * When holding the screen in landscape mode as the BIOS text output is
 * presented, x is the horizontal axis with values growing from left to
 * right and y is the vertical axis with values growing from top to
 * bottom.
 *
 * When holding the screen in portrait mode with the Sahara logo in its
 * correct position, x ist the vertical axis with values growing from
 * top to bottom and y is the horizontal axis with values growing from
 * right to left.
 */

#define T213_FORMAT_TOUCH_BIT	0x01
#define T213_FORMAT_STATUS_BYTE	0x55
#define T213_FORMAT_STATUS_MASK	0x54

/*
 * On my Sahara Touch-IT 213 I have observed x values from 0 to 0x7f0
 * and y values from 0x1d to 0x7e9, so the actual measurement is
 * probably done with an 11 bit precision.
 */
#define T213_MIN_XC 0
#define T213_MAX_XC 0x07ff
#define T213_MIN_YC 0
#define T213_MAX_YC 0x07ff

#define MAX_16BIT ((1 << 16) - 1)

enum inversion_mode {
    INV_XY_NORMAL = 0,
	INV_X_MIRROR  = 1,
	INV_Y_MIRROR  = 2,
	INV_XY_MIRROR = 3,
};

struct touchit213 {
	char phys[32];
	unsigned char csum;

	int idx;
	unsigned char data[10];

	struct axis validArea;

	enum inversion_mode inver;

#ifdef ENABLE_CALIBRATION
    struct calibration calib;
#endif
	
	struct serio *serio;
	struct input_dev *input;
	spinlock_t spin;
};

/* package format:
 * bit0 bit1 bit2 bit3 bit4 bit5 bit6 bit7
 * BYTE MASK  XM   XL   YM   YL       full
 */

#define STATUS_BYTE          0
#define STATUS_MASK          1
#define COMPLETE_PACKAGE     8
#define RESTORATION_PACKAGE  0

inline void offset_collect_xy(struct touchit213 *touch, int *x, int *y)
{
#ifdef DEBUG_SWAP
    DBG_PRINTK("before inver = %d, x = %d, y = %d!. \n", touch->inver, *x, *y);
#endif

    switch (touch->inver) {
	case INV_XY_NORMAL:	
		break;
	case INV_X_MIRROR:
		*x = touch->validArea.xMax - *x;
		break;
	case INV_Y_MIRROR:
		*y = touch->validArea.yMax - *y;
		break;
	case INV_XY_MIRROR:
		*x = touch->validArea.xMax - *x;
		*y = touch->validArea.yMax - *y;
		break;
	default:
		break;
	}
	
#ifdef DEBUG_SWAP
    DBG_PRINTK("after inver = %d, x = %d, y = %d!. \n", touch->inver, *x, *y);
#endif
}

static irqreturn_t touchit213_interrupt(struct serio *serio,
	unsigned char data, unsigned int flags) 
{
	struct touchit213 *touch = serio_get_drvdata(serio);
	int report_x;
	int report_y;
	int cal_x, cal_y;
	int report_touch = 0;
	
#ifdef DEBUG_IRQ
    DBG_PRINTK("tirgger irq!. \n");
#endif
	
	touch->data[touch->idx] = data;

	switch (touch->idx++) {
	case STATUS_BYTE:
		if (touch->data[0] != T213_FORMAT_STATUS_BYTE) {
			pr_debug("unsynchronized data: 0x%02x\n", data);
			touch->idx = RESTORATION_PACKAGE;
		}
		break;
		
	case STATUS_MASK:
		if (touch->data[1] != T213_FORMAT_STATUS_MASK) {
			pr_debug("unsynchronized data: 0x%02x\n", data);
			touch->idx = RESTORATION_PACKAGE;
		}
		break;
		
	case COMPLETE_PACKAGE:
		report_x = ((touch->data[4] << 8) | touch->data[3]) << 3;
		report_y = ((touch->data[6] << 8) | touch->data[5]) << 3;
	    report_touch = (touch->data[2] & 0x03) ? 1 : 0;

		
		//offset_collect_xy(touch, &report_x, &report_y);
#ifdef ENABLE_CALIBRATION
		if (touch->calib.isCalibrateing) {
			touch->calib.ADPoint.xMax = report_x;
			touch->calib.ADPoint.yMax = report_y;
		}

		if (touch->calib.isAlreadyCalibrate) {
			TouchPanelCalibrateAPoint(report_x, report_y, &cal_x, &cal_y);
			report_x = cal_x / 4;
		    report_y = cal_y / 4;
		}
#endif

		input_report_key(touch->input, BTN_TOUCH, report_touch);
		input_report_abs(touch->input, ABS_X, report_x);
		input_report_abs(touch->input, ABS_Y, report_y);
		input_sync(touch->input);
		
#ifdef DEBUG_DATA
		DBG_PRINTK("report_touch:%d, report_x=%d , report_y=%d\n\n", report_touch, report_x, report_y);
#endif

		touch->idx = RESTORATION_PACKAGE;
		break;
	default:
		break;
	}

	return IRQ_HANDLED;
}

static int initialization_input(void *pdata, const int xMax, const int yMax) 
{
    int rtn;
	
	struct input_dev *input;
	struct touchit213 *pdev = (struct touchit213 *)pdata;
	
	// clear input_dev, for file node refresh
	if (pdev->input != NULL) {
		input_unregister_device(pdev->input);
		input_free_device(pdev->input);
	}
	
	input = input_allocate_device();
	if (input == NULL) {
		goto error_input_alloc;
	}

	snprintf(pdev->phys, sizeof(pdev->phys), "%s/input0", pdev->serio->phys);
	input->name = "Sahara Touch-iT213 Serial TouchScreen";
	input->phys = pdev->phys;
	input->id.bustype = BUS_RS232;
	input->id.vendor  = SERIO_TOUCHIT213;
	input->id.product = 0x0001;
	input->id.version = 0x0100;
	input->dev.parent = &pdev->serio->dev;

	set_bit(ABS_X, input->absbit);
	set_bit(ABS_Y, input->absbit);
	set_bit(EV_ABS, input->evbit);
	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_TOUCH, input->keybit);

	DBG_PRINTK("validArea->x = %d, validArea->y = %d\n", xMax, yMax);
	input_set_abs_params(input, ABS_X, 0, xMax, 0, 0);
	input_set_abs_params(input, ABS_Y, 0, yMax, 0, 0);

	rtn = input_register_device(input);
	if (rtn) {
		DBG_PRINTK("input_register_device error!.");
		goto error_input_register;	
	}
	
	pdev->input = input;
	
    return 0;

error_input_register:
	input_free_device(input);
error_input_alloc:
	return -ENOMEM;
}

struct touchit213 *pdata = NULL;

static ssize_t xmax_show(struct class *class,
    struct class_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", 1);		
}

static ssize_t xmax_store(struct class *cls,
    struct class_attribute *attr, const char *buf, size_t count)
{
    int value = simple_strtoul(buf, NULL, 0);
	DBG_PRINTK("value:%d \n", value);
    return count;
}
	
static struct class_attribute touchit213_attrs[] = {
	__ATTR(xmax, S_IRUGO | S_IWUGO, xmax_show, xmax_store),
    __ATTR_NULL
};
 
static struct class touchit213_class = {
    .name = "touchit213",
    .class_attrs = touchit213_attrs,
};
	
static int touchit213_connect(struct serio *serio, struct serio_driver *serio_drv) 
{
    int rtn;
	struct touchit213 *touch_dev;
	
	DBG_PRINTK("touchit213 connect!.");
	
	touch_dev = kzalloc(sizeof(struct touchit213), GFP_KERNEL);
	if (touch_dev == NULL) {
		goto error_kzalloc;
	}
	
   	touch_dev->idx  = 0;
	touch_dev->serio = serio;
	touch_dev->input = NULL;

	touch_dev->inver = INV_XY_NORMAL;
	touch_dev->validArea.xMax = MAX_16BIT;
	touch_dev->validArea.yMax = MAX_16BIT;

	class_register(&touchit213_class);
	
#ifdef ENABLE_CALIBRATION
	memset(&touch_dev->calib, 0, sizeof(struct calibration));
    touch_dev->calib.pdata = (void *)touch_dev;
	touch_dev->calib.reinput = initialization_input;
    tp_calib_iface_init(&touch_dev->calib, &touch_dev->validArea);	
#endif

	if (initialization_input(touch_dev, touch_dev->validArea.xMax, touch_dev->validArea.yMax)) {
		DBG_PRINTK("initialization_input error!.");
		goto error_initialization;
	}

	spin_lock_init(&(touch_dev->spin));
	
	serio_set_drvdata(serio, touch_dev);
	rtn = serio_open(serio, serio_drv);
	if (rtn) {
		DBG_PRINTK("serio_open error!.");
		goto error_serio_open;
	}
	
	return 0;

error_serio_open:
	serio_close(serio);
error_initialization:
    kfree(touch_dev);
error_kzalloc:
	return -ENOMEM;
}

static void touchit213_disconnect(struct serio *serio) 
{
    struct touchit213 *touchit213 = serio_get_drvdata(serio);
	DBG_PRINTK("touchit213 disconnect!.");
	
	serio_close(serio);

#ifdef ENABLE_CALIBRATION
    tp_calib_iface_exit();
#endif

	input_unregister_device(touchit213->input);
	input_free_device(touchit213->input);
    kfree(touchit213);
}

/*
 * The serio driver structure.
 */

static struct serio_device_id touchit213_serio_ids[] = {
	{
		.type	= SERIO_RS232,
		.proto	= SERIO_TOUCHIT213,
		.id	    = SERIO_ANY,
		.extra	= SERIO_ANY,
	} , { 0 }
};
	
static struct serio_driver touchit213_drv = {
	.driver		= {
		.name	= "touchit213",
	},
	.description	= DRIVER_DESC,
	.id_table	= touchit213_serio_ids,
	.interrupt	= touchit213_interrupt,
	.connect	= touchit213_connect,
	.disconnect	= touchit213_disconnect,
};
		
module_serio_driver(touchit213_drv);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("iFinelio Tower <luorongwei@sunchip-tech.com>");
