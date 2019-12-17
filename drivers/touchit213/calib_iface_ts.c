/*
 * Export interface in /sys/class/touchpanel for calibration.
 *
 * Yongle Lai @ Rockchip - 2010-07-26
 */
#include <linux/fs.h>

#include "calib_iface_ts.h"
#include "calibration_ts.h"

#define MAX_16BIT ((1 << 16) - 1)

#define DBG_PRINTK(fmt, args...) \
	do { \
		printk(KERN_INFO "<calib_ifaces>: "fmt, ##args); \
	} while (0)

/*
 * The sys nodes for touch panel calibration depends on controller's name,
 * such as: /sys/bus/spi/drivers/xpt2046_ts/touchadc
 * If we use another TP controller (not xpt2046_ts), the above path will 
 * be unavailable which will cause calibration to be fail.
 *
 * Another choice is: 
 *   sys/devices/platform/rockchip_spi_master/spi0.0/driver/touchadc
 * this path request the TP controller will be the first device of SPI.
 *
 * To make TP calibration module in Android be universal, we create
 * a class named touchpanel as the path for calibration interfaces.
 */
 
/*
 * TPC driver depended.
 */

#if 0
#if defined(CONFIG_MACH_RK2818INFO_IT50) && defined(CONFIG_TOUCHSCREEN_XPT2046_CBN_SPI)
	int screen_x[5] = { 50, 750,  50, 750, 400};
  	int screen_y[5] = { 40,  40, 440, 440, 240};
	int uncali_x_default[5] = { 3735,  301, 3754,  290, 1993 };
	int uncali_y_default[5] = {  3442,  3497, 413, 459, 1880 };
#elif defined(CONFIG_MACH_RK2818INFO) && defined(CONFIG_TOUCHSCREEN_XPT2046_CBN_SPI) 
	int screen_x[5] = { 50, 750,  50, 750, 400};
  	int screen_y[5] = { 40,  40, 440, 440, 240};
	int uncali_x_default[5] = { 438,  565, 3507,  3631, 2105 };
	int uncali_y_default[5] = {  3756,  489, 3792, 534, 2159 };
#elif (defined(CONFIG_MACH_RAHO) || defined(CONFIG_MACH_RAHOSDK) || defined(CONFIG_MACH_RK2818INFO))&& defined(CONFIG_TOUCHSCREEN_XPT2046_320X480_CBN_SPI)
	int screen_x[5] = { 50, 270,  50, 270, 160}; 
	int screen_y[5] = { 40,  40, 440, 440, 240}; 
	int uncali_x_default[5] = { 812,  3341, 851,  3371, 2183 };
	int uncali_y_default[5] = {  442,  435, 3193, 3195, 2004 };
#elif defined(CONFIG_MACH_Z5) && defined(CONFIG_TOUCHSCREEN_XPT2046_CBN_SPI)
	int uncali_x_default[5] = {  3267,  831, 3139, 715, 1845 };
	int uncali_y_default[5] = { 3638,  3664, 564,  591, 2087 };
	int screen_x[5] = { 70,  410, 70, 410, 240};
	int screen_y[5] = { 50, 50,  740, 740, 400};
#endif
#endif

#define POINT_PADDING 50
static struct calibration *pCalibration = NULL;

static int calculate_refpoint(struct calibration *calib, char *sequence_cali, int len) 
{
	// init calibrate args default
	DBG_PRINTK("len: %d, para: %s \n", len, sequence_cali);
	sscanf(sequence_cali, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
		&calib->uncali_x[0],&calib->uncali_y[0],
		&calib->uncali_x[1],&calib->uncali_y[1],
		&calib->uncali_x[2],&calib->uncali_y[2],
		&calib->uncali_x[3],&calib->uncali_y[3],
		&calib->uncali_x[4],&calib->uncali_y[4],
		&calib->screen.xMax,&calib->screen.yMax);
	
	calib->screen_x[0] = POINT_PADDING;
	calib->screen_y[0] = POINT_PADDING;
	            
	calib->screen_x[1] = calib->screen.xMax - POINT_PADDING;
	calib->screen_y[1] = POINT_PADDING;
	            
	calib->screen_x[2] = POINT_PADDING;
	calib->screen_y[2] = calib->screen.yMax - POINT_PADDING;
	            
	calib->screen_x[3] = calib->screen.xMax - POINT_PADDING;
	calib->screen_y[3] = calib->screen.yMax - POINT_PADDING;
	            
	calib->screen_x[4] = calib->screen.xMax / 2;
	calib->screen_y[4] = calib->screen.yMax / 2;
}

#define CONFIG "/data/private/calibargs"
static int check_calibration(struct calibration *calib) 
{
	int len = 0;
	char para[150];

	DBG_PRINTK("ERROR check_calibration\n");
	
	if (calib->isCalibrateing) {
		DBG_PRINTK("Calibrateing...\n");
		goto exit_calculate;
	}
	
	struct file *fp = filp_open(CONFIG, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		DBG_PRINTK("open %s failed.\n", CONFIG);
		goto exit_calculate;
	}

	fp->f_op->llseek(fp, 0, 0);
	len = fp->f_op->read(fp, para, 150, &fp->f_pos);
	if (len <= 20) {
		filp_close(fp, NULL);
		goto exit_calculate;
	}
	
	filp_close(fp, NULL);
	calculate_refpoint(calib, &para[0], len);
	
	return true;
	
exit_calculate:
	return false;
}

static ssize_t touch_debug_show(struct class *class,
    struct class_attribute *attr, char *_buf)
{
	if (pCalibration != NULL) {
	    return snprintf(_buf, PAGE_SIZE,
			"Args:\n "
			"\t screen[0]: %d, %d\n"
			"\t screen[1]: %d, %d\n"
			"\t screen[2]: %d, %d\n" 
			"\t screen[3]: %d, %d\n" 
			"\t screen[4]: %d, %d\n\n"
			"\t uncali[0]: %d, %d\n" 
			"\t uncali[1]: %d, %d\n" 
			"\t uncali[2]: %d, %d\n" 
			"\t uncali[3]: %d, %d\n" 
			"\t uncali[4]: %d, %d\n\n" 
			"Screen:\n"
			"\t xMax: %d"
			"\t yMax: %d\n\n"
			"ADCPoint:\n"
			"\t xPoint: %d"
			"\t yPoint: %d\n\n"
			"Status:\n" 
			"\t calibrate index: %d\n" 
			"\t isCalibrateing: %d\n" 
			"\t isAlreadyCalibrate: %d\n", 
			pCalibration->screen_x[0], pCalibration->screen_y[0],
			pCalibration->screen_x[1], pCalibration->screen_y[1],
			pCalibration->screen_x[2], pCalibration->screen_y[2],
			pCalibration->screen_x[3], pCalibration->screen_y[3],
			pCalibration->screen_x[4], pCalibration->screen_y[4],
			pCalibration->uncali_x[0], pCalibration->uncali_y[0],
			pCalibration->uncali_x[1], pCalibration->uncali_y[1],
			pCalibration->uncali_x[2], pCalibration->uncali_y[2],
			pCalibration->uncali_x[3], pCalibration->uncali_y[3],
			pCalibration->uncali_x[4], pCalibration->uncali_y[4],
			pCalibration->screen.xMax, pCalibration->screen.yMax,
			pCalibration->ADPoint.xMax, pCalibration->ADPoint.yMax,
			pCalibration->calibindex,
			pCalibration->isCalibrateing,
			pCalibration->isAlreadyCalibrate);
	}

	return 0;
}

//This code is Touch adc simple value
static ssize_t touch_adc_show(struct class *class,
    struct class_attribute *attr, char *_buf)
{
	if (pCalibration != NULL) {
		return snprintf(_buf, PAGE_SIZE, "%d,%d\n", 
			pCalibration->ADPoint.xMax, pCalibration->ADPoint.yMax);
	}

	return snprintf(_buf, PAGE_SIZE, "%s\n", "UNKNOWN");
}

static ssize_t touch_adc_store(struct class *cls,
    struct class_attribute *attr, const char *_buf, size_t _count)
{
	if (pCalibration != NULL) {
		pCalibration->isCalibrateing = false;
		calculate_refpoint(pCalibration, _buf, _count);
	}
	
    return _count; 
}

static ssize_t touch_recalibration_show(struct class *class,
    struct class_attribute *attr, char *_buf)
{
    int r;
	
	if (pCalibration != NULL) {
		r = TouchPanelSetCalibration(4, pCalibration->screen_x, pCalibration->screen_y, 
			        pCalibration->uncali_x, pCalibration->uncali_y);
		
		DBG_PRINTK("r = %d \n", r);
		
		if (r == 1) { 
			if (pCalibration->reinput)
				pCalibration->reinput(pCalibration->pdata, 
				        pCalibration->screen.xMax, pCalibration->screen.yMax);
			
			pCalibration->isAlreadyCalibrate = true;
			r = sprintf(_buf, "successful\n");
		} else {
			pCalibration->isAlreadyCalibrate = false;
			r = sprintf(_buf, "fail\n");
		}
	}
	
#if 0
    if (r == 1) {
    	memcpy(uncali_x_default, uncali_x, sizeof(uncali_x));
    	memcpy(uncali_y_default, uncali_y, sizeof(uncali_y));
    	DBG_PRINTK("touch_cali_status-0--%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
	                uncali_x_default[0], uncali_y_default[0],
	                uncali_x_default[1], uncali_y_default[1],
	                uncali_x_default[2], uncali_y_default[2],
	                uncali_x_default[3], uncali_y_default[3],
	                uncali_x_default[4], uncali_y_default[4]);
    	r = sprintf(_buf, "successful\n");
    } else {
     	DBG_PRINTK("touchpal calibration failed, use default value.\n");
    	r = TouchPanelSetCalibration(4, screen_x, screen_y, uncali_x_default, uncali_y_default);
    	DBG_PRINTK("touch_cali_status-1---%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
	                uncali_x_default[0], uncali_y_default[0],
	                uncali_x_default[1], uncali_y_default[1],
	                uncali_x_default[2], uncali_y_default[2],
	                uncali_x_default[3], uncali_y_default[3],
	                uncali_x_default[4], uncali_y_default[4]);
    	if (r == 1) {
    		r = sprintf(_buf, "recovery\n");
    	} else{
    		r = sprintf(_buf, "fail\n");
   		}
    }
    
    //DBG_PRINTK("Calibration status: _buf=<%s", _buf);
#endif

	return r;
}

static ssize_t touch_recalibration_store(struct class *cls,
    struct class_attribute *attr, const char *_buf, size_t _count)
{
	int value = simple_strtoul(_buf, NULL, 0);
	if (pCalibration != NULL && value >= 1) {
		// clear		
		DBG_PRINTK("INFO: clear calibration args. \n");
	    pCalibration->isCalibrateing = true;
	    pCalibration->isAlreadyCalibrate = false;
		if (pCalibration->reinput) {
			DBG_PRINTK("INFO: reinput valid area (%d, %d)\n", MAX_16BIT, MAX_16BIT);
			pCalibration->reinput(pCalibration->pdata, MAX_16BIT, MAX_16BIT);
		}
		
	}
	
	return _count;
}

static struct class_attribute touchit213_attrs[] = {
	__ATTR(debug, S_IRUGO | S_IWUGO, touch_debug_show, NULL),
	__ATTR(touchadc, S_IRUGO | S_IWUGO, touch_adc_show, touch_adc_store),
	__ATTR(recalibration, S_IRUGO | S_IWUGO, touch_recalibration_show, touch_recalibration_store),
};
static struct class *tp_class = NULL; 
static int rkfb_create_sysfs()
{
	int r, t;
	DBG_PRINTK("rkfb_create_sysfs into \n");
	
	tp_class = class_create(THIS_MODULE, "calib_ifaces");
	if (IS_ERR(tp_class)) {
		DBG_PRINTK("Create class calib_iface failed.\n");
		return -ENOMEM;
	}
	
	for (t = 0; t < ARRAY_SIZE(touchit213_attrs); t++) {
		r = class_create_file(tp_class, &touchit213_attrs[t]);
		if (r) {
			DBG_PRINTK("failed to create sysfs file\n");
			return r;
		}
	}

	DBG_PRINTK("rkfb_create_sysfs exit \n");
	return 0;
}

static  void rkfb_remove_sysfs()
{
	int i, t;
	
	for (t = 0; t < ARRAY_SIZE(touchit213_attrs); t++) {
		class_remove_file(tp_class, &touchit213_attrs[t]);
	}

    class_destroy(tp_class);
}

int tp_calib_iface_init(struct calibration *calib, struct axis *saxis)
{
    int r = 0;	
	
	calib->isAlreadyCalibrate = check_calibration(calib);
    if (calib->isAlreadyCalibrate) {
		DBG_PRINTK("Usage: Relocation axis depend on the calibration parameters. \n");
	    r = TouchPanelSetCalibration(4, calib->screen_x, calib->screen_y, 
			        calib->uncali_x, calib->uncali_y);
		if (r) {
			// TODO: Attention here. This assignment affects the external parameters
			saxis->yMax = calib->screen.yMax;
			saxis->xMax = calib->screen.xMax;
		    calib->isAlreadyCalibrate = true;
			DBG_PRINTK("Auto set calibration successfully.\n");	
		} else {		
			calib->isAlreadyCalibrate = false;
		    DBG_PRINTK("Auto set calibraion failed, reset data again please! \n");
		}
	} else {
		DBG_PRINTK("INFO: calibration parameters is not config.\n");
	}

	pCalibration = calib;
    /*
	 * Create ifaces for TP calibration.
	 */
	rkfb_create_sysfs();
	
	return r;
}

void tp_calib_iface_exit(void)
{
    rkfb_remove_sysfs();
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Touchit213 TPC driver @ Sunchip");
MODULE_AUTHOR("iFinelio Tower <luorongwei@sunchip-tech.com>");

