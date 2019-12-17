/*
 * drivers/input/touchscreen/calibration_ts.c - calibration for rk2818 spi xpt2046 device and console
 *
 * Copyright (C) 2010 ROCKCHIP, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include <linux/fs.h>

#include "calibration_ts.h"
#include "largenum_ts.h"

#define MAX_POINT_ERROR 6

#undef DEBUGMSG
#define DEBUGMSG(level, fmt, args...) \
	do { \
		printk(KERN_INFO "<calibration_ts>: "fmt, ##args); \
	} while (0)

typedef struct {
    PLARGENUM   pa11, pa12, pa13;
    PLARGENUM   pa21, pa22, pa23;
    PLARGENUM   pa31, pa32, pa33;
}   MATRIX33, *PMATRIX33;

typedef struct {
    int   a1;
    int   b1;
    int   c1;
    int   a2;
    int   b2;
    int   c2;
    int   delta;
}
CALIBRATION_PARAMETER, *PCALIBRATION_PARAMETER;

static unsigned char            v_Calibrated = 0;
static CALIBRATION_PARAMETER    v_CalcParam = {
	.a1		=18670 ,
	.b1		=98,
	.c1		= -2230109,
	.a2   	= 291,
	.b2  	= 12758,
	.c2  	= -5118934,
	.delta	= 91931,
};
	
static CALIBRATION_PARAMETER v_CalcParam_bak = {
#if 1
	.a1 =17704 ,
	.b1 =-20,
	.c1 = -1460283,
	.a2 = 382,
	.b2 = 12685,
	.c2 = -5595261,
	.delta = 88403,
#endif
};

unsigned char
ErrorAnalysis (
    int   cCalibrationPoints,     //@PARM The number of calibration points
    int   *pScreenXBuffer,        //@PARM List of screen X coords displayed
    int   *pScreenYBuffer,        //@PARM List of screen Y coords displayed
    int   *pUncalXBuffer,         //@PARM List of X coords collected
    int   *pUncalYBuffer          //@PARM List of Y coords collected
    );

void
ComputeMatrix33 (
    PLARGENUM   pResult,
    PMATRIX33   pMatrix
    );

unsigned char
TouchPanelSetCalibration(
    int   cCalibrationPoints,     //@PARM The number of calibration points
    int   *pScreenXBuffer,        //@PARM List of screen X coords displayed
    int   *pScreenYBuffer,        //@PARM List of screen Y coords displayed
    int   *pUncalXBuffer,         //@PARM List of X coords collected
    int   *pUncalYBuffer          //@PARM List of Y coords collected
    )
{
    LARGENUM    a11;
    LARGENUM    a21, a22;
    LARGENUM    a31, a32, a33;
    LARGENUM    b11, b12, b13;
    LARGENUM    b21, b22, b23;
    LARGENUM    lnScreenX;
    LARGENUM    lnScreenY;
    LARGENUM    lnTouchX;
    LARGENUM    lnTouchY;
    LARGENUM    lnTemp;
    LARGENUM    delta;
    LARGENUM    a1, b1, c1;
    LARGENUM    a2, b2, c2;
    MATRIX33    Matrix;
    int       cShift;
    int       minShift;
    int         i;

    DEBUGMSG(1, "calibrating %d point set\r\n", cCalibrationPoints);
    
	//
	// If the calibration data is being cleared, set the flag so
	// that the conversion operation is a noop.
	//
	
    if ( cCalibrationPoints == 0 ) {
        v_Calibrated = 0;
        return 1;
    }

    //
    // Compute these large numbers
    //
    LargeNumSet(&a11, 0);
    LargeNumSet(&a21, 0);
    LargeNumSet(&a31, 0);
    LargeNumSet(&a22, 0);
    LargeNumSet(&a32, 0);
    LargeNumSet(&a33, cCalibrationPoints);
    LargeNumSet(&b11, 0);
    LargeNumSet(&b12, 0);
    LargeNumSet(&b13, 0);
    LargeNumSet(&b21, 0);
    LargeNumSet(&b22, 0);
    LargeNumSet(&b23, 0);
    for(i=0; i<cCalibrationPoints; i++){
        LargeNumSet(&lnTouchX, pUncalXBuffer[i]);
        LargeNumSet(&lnTouchY, pUncalYBuffer[i]);
        LargeNumSet(&lnScreenX, pScreenXBuffer[i]);
        LargeNumSet(&lnScreenY, pScreenYBuffer[i]);
        LargeNumMult(&lnTouchX, &lnTouchX, &lnTemp);
        LargeNumAdd(&a11, &lnTemp, &a11);
        LargeNumMult(&lnTouchX, &lnTouchY, &lnTemp);
        LargeNumAdd(&a21, &lnTemp, &a21);
        LargeNumAdd(&a31, &lnTouchX, &a31);
        LargeNumMult(&lnTouchY, &lnTouchY, &lnTemp);
        LargeNumAdd(&a22, &lnTemp, &a22);
        LargeNumAdd(&a32, &lnTouchY, &a32);
        LargeNumMult(&lnTouchX, &lnScreenX, &lnTemp);
        LargeNumAdd(&b11, &lnTemp, &b11);
        LargeNumMult(&lnTouchY, &lnScreenX, &lnTemp);
        LargeNumAdd(&b12, &lnTemp, &b12);
        LargeNumAdd(&b13, &lnScreenX, &b13);
        LargeNumMult(&lnTouchX, &lnScreenY, &lnTemp);
        LargeNumAdd(&b21, &lnTemp, &b21);
        LargeNumMult(&lnTouchY, &lnScreenY, &lnTemp);
        LargeNumAdd(&b22, &lnTemp, &b22);
        LargeNumAdd(&b23, &lnScreenY, &b23);
    }

    Matrix.pa11 = &a11;
    Matrix.pa21 = &a21;
    Matrix.pa31 = &a31;
    Matrix.pa12 = &a21;
    Matrix.pa22 = &a22;
    Matrix.pa32 = &a32;
    Matrix.pa13 = &a31;
    Matrix.pa23 = &a32;
    Matrix.pa33 = &a33;
    ComputeMatrix33(&delta, &Matrix);

    Matrix.pa11 = &b11;
    Matrix.pa21 = &b12;
    Matrix.pa31 = &b13;
    ComputeMatrix33(&a1, &Matrix);

    Matrix.pa11 = &a11;
    Matrix.pa21 = &a21;
    Matrix.pa31 = &a31;
    Matrix.pa12 = &b11;
    Matrix.pa22 = &b12;
    Matrix.pa32 = &b13;
    ComputeMatrix33(&b1, &Matrix);

    Matrix.pa12 = &a21;
    Matrix.pa22 = &a22;
    Matrix.pa32 = &a32;
    Matrix.pa13 = &b11;
    Matrix.pa23 = &b12;
    Matrix.pa33 = &b13;
    ComputeMatrix33(&c1, &Matrix);

    Matrix.pa13 = &a31;
    Matrix.pa23 = &a32;
    Matrix.pa33 = &a33;
    Matrix.pa11 = &b21;
    Matrix.pa21 = &b22;
    Matrix.pa31 = &b23;
    ComputeMatrix33(&a2, &Matrix);

    Matrix.pa11 = &a11;
    Matrix.pa21 = &a21;
    Matrix.pa31 = &a31;
    Matrix.pa12 = &b21;
    Matrix.pa22 = &b22;
    Matrix.pa32 = &b23;
    ComputeMatrix33(&b2, &Matrix);

    Matrix.pa12 = &a21;
    Matrix.pa22 = &a22;
    Matrix.pa32 = &a32;
    Matrix.pa13 = &b21;
    Matrix.pa23 = &b22;
    Matrix.pa33 = &b23;
    ComputeMatrix33(&c2, &Matrix);

#if 1
    {
        LARGENUM    halfDelta;
        //
        // Take care of possible truncation error in later mapping operations
        //
        if(IsLargeNumNegative(&delta)){
            LargeNumDivInt32(&delta, -2, &halfDelta);
        } else {
            LargeNumDivInt32(&delta, 2, &halfDelta);
        }
        LargeNumAdd(&c1, &halfDelta, &c1);
        LargeNumAdd(&c2, &halfDelta, &c2);
    }
#endif

    //
    // All the numbers are determined now.
    // Let's scale them back to 32 bit world
    //
    minShift = 0;
    cShift = LargeNumBits(&a1) - MAX_COEFF_PRECISION;
    if(cShift > minShift){
        minShift = cShift;
    }
    cShift = LargeNumBits(&b1) - MAX_COEFF_PRECISION;
    if(cShift > minShift){
        minShift = cShift;
    }
    cShift = LargeNumBits(&a2) - MAX_COEFF_PRECISION;
    if(cShift > minShift){
        minShift = cShift;
    }
    cShift = LargeNumBits(&b2) - MAX_COEFF_PRECISION;
    if(cShift > minShift){
        minShift = cShift;
    }
    cShift = LargeNumBits(&c1) - MAX_TERM_PRECISION;
    if(cShift > minShift){
        minShift = cShift;
    }
    cShift = LargeNumBits(&c2) - MAX_TERM_PRECISION;
    if(cShift > minShift){
        minShift = cShift;
    }
    cShift = LargeNumBits(&delta) - 31;
    if(cShift > minShift){
        minShift = cShift;
    }

    //
    // Now, shift count is determined, shift all the numbers
    //  right to obtain the 32-bit signed values
    //
    if(minShift){
        LargeNumRAShift(&a1, minShift);
        LargeNumRAShift(&a2, minShift);
        LargeNumRAShift(&b1, minShift);
        LargeNumRAShift(&b2, minShift);
        LargeNumRAShift(&c1, minShift);
        LargeNumRAShift(&c2, minShift);
        LargeNumRAShift(&delta, minShift);
    }
    v_CalcParam.a1      = a1.u.s32.u[0];
    v_CalcParam.b1      = b1.u.s32.u[0];
    v_CalcParam.c1      = c1.u.s32.u[0];
    v_CalcParam.a2      = a2.u.s32.u[0];
    v_CalcParam.b2      = b2.u.s32.u[0];
    v_CalcParam.c2      = c2.u.s32.u[0];
    v_CalcParam.delta   = delta.u.s32.u[0];
	
     // Don't allow delta to be zero, since it gets used as a divisor
    if( ! v_CalcParam.delta ) {
        //RETAILMSG(1,("TouchPanelSetCalibration: delta of 0 invalid\r\n")));
        //RETAILMSG(1,("\tCalibration failed.\r\n")));
        v_CalcParam.delta = 1;  // any non-zero value to prevents DivByZero traps later
        v_Calibrated = 0;
    } else {
		v_Calibrated = 1;
	}
	
    return ErrorAnalysis(
                    cCalibrationPoints,
                    pScreenXBuffer,
                    pScreenYBuffer,
                    pUncalXBuffer,
                    pUncalYBuffer
                );
}

void
ComputeMatrix33(
    PLARGENUM   pResult,
    PMATRIX33   pMatrix
    )
{
    LARGENUM    lnTemp;

    LargeNumMult(pMatrix->pa11, pMatrix->pa22, &lnTemp);
    LargeNumMult(pMatrix->pa33, &lnTemp, pResult);
    LargeNumMult(pMatrix->pa21, pMatrix->pa32, &lnTemp);
    LargeNumMult(pMatrix->pa13, &lnTemp, &lnTemp);
    LargeNumAdd(pResult, &lnTemp, pResult);
    LargeNumMult(pMatrix->pa12, pMatrix->pa23, &lnTemp);
    LargeNumMult(pMatrix->pa31, &lnTemp, &lnTemp);
    LargeNumAdd(pResult, &lnTemp, pResult);
    LargeNumMult(pMatrix->pa13, pMatrix->pa22, &lnTemp);
    LargeNumMult(pMatrix->pa31, &lnTemp, &lnTemp);
    LargeNumSub(pResult, &lnTemp, pResult);
    LargeNumMult(pMatrix->pa12, pMatrix->pa21, &lnTemp);
    LargeNumMult(pMatrix->pa33, &lnTemp, &lnTemp);
    LargeNumSub(pResult, &lnTemp, pResult);
    LargeNumMult(pMatrix->pa23, pMatrix->pa32, &lnTemp);
    LargeNumMult(pMatrix->pa11, &lnTemp, &lnTemp);
    LargeNumSub(pResult, &lnTemp, pResult);
}

void
TouchPanelCalibrateAPoint(
    int   UncalX,     //@PARM The uncalibrated X coordinate
    int   UncalY,     //@PARM The uncalibrated Y coordinate
    int   *pCalX,     //@PARM The calibrated X coordinate
    int   *pCalY      //@PARM The calibrated Y coordinate
    )
{
    int   x, y;

    if ( v_Calibrated ) {
		//
		// Note the *4 in the expression below.  This is a workaround
		// on behalf of gwe.  It provides a form of
		// sub-pixel accuracy desirable for inking
		//
    	x = (v_CalcParam.a1 * UncalX + v_CalcParam.b1 * UncalY +
         	v_CalcParam.c1) * 4 / v_CalcParam.delta;
    	y = (v_CalcParam.a2 * UncalX + v_CalcParam.b2 * UncalY +
         	v_CalcParam.c2) * 4 / v_CalcParam.delta;
    } else {
		// Modify Tower 20191213, check ErrorAnalysis()-> maxErr < (errThreshold * errThreshold
#if 0
	 	x = (v_CalcParam_bak.a1 * UncalX + v_CalcParam_bak.b1 * UncalY +
			v_CalcParam_bak.c1) * 4 / v_CalcParam_bak.delta;
		y = (v_CalcParam_bak.a2 * UncalX + v_CalcParam_bak.b2 * UncalY +
		  v_CalcParam_bak.c2) * 4 / v_CalcParam_bak.delta;
#else
		x = (v_CalcParam.a1 * UncalX + v_CalcParam.b1 * UncalY +
         	v_CalcParam.c1) * 4 / v_CalcParam.delta;
    	y = (v_CalcParam.a2 * UncalX + v_CalcParam.b2 * UncalY +
         	v_CalcParam.c2) * 4 / v_CalcParam.delta;
#endif
    }
	 
    if ( x < 0 ) {
        x = 0;
    }

    if  (y < 0 ) {
        y = 0;
    }

    *pCalX = x;
    *pCalY = y;
}

unsigned char
ErrorAnalysis(
    int   cCalibrationPoints,     //@PARM The number of calibration points
    int   *pScreenXBuffer,        //@PARM List of screen X coords displayed
    int   *pScreenYBuffer,        //@PARM List of screen Y coords displayed
    int   *pUncalXBuffer,         //@PARM List of X coords collected
    int   *pUncalYBuffer          //@PARM List of Y coords collected
    )
{
    int     i;
    unsigned int  maxErr, err;
    int   x,y;
    int   dx,dy;
    unsigned int  errThreshold = MAX_POINT_ERROR;  // Can be overridden by registry entry
    
    maxErr = 0;
    DEBUGMSG(1, "   Screen    =>    Mapped\r\n");
    for (i = 0; i < cCalibrationPoints; i++){
        TouchPanelCalibrateAPoint(  pUncalXBuffer[i],
                                    pUncalYBuffer[i],
                                    &x,
                                    &y
                                    );
        x /= 4;
        y /= 4;
		
        DEBUGMSG(1, "(%4d, %4d) => (%4d, %4d)\r\n", 
			    pScreenXBuffer[i],
                pScreenYBuffer[i],
                x,
                y);
		
        dx = x - pScreenXBuffer[i];
        dy = y - pScreenYBuffer[i];
		
        err = dx * dx + dy * dy;
        if(err > maxErr){
            maxErr = err;
        }
    }
	
    DEBUGMSG(1, "Maximum error (square of Euclidean distance in screen units) = %u\r\n", maxErr);

    if (maxErr < (errThreshold * errThreshold)) {
		DEBUGMSG(
			1,
			" v_CalcParam.a1 =%d \n"	 
			" v_CalcParam.b1 =%d\n"	
			" v_CalcParam.c1 = %d\n"	 
			" v_CalcParam.a2 = %d\n" 
			" v_CalcParam.b2 = %d\n"	
			" v_CalcParam.c2 = %d\n"
			" v_CalcParam.delta = %d\n",
			v_CalcParam.a1 , \
			v_CalcParam.b1 , \
			v_CalcParam.c1 , \
			v_CalcParam.a2 , \
			v_CalcParam.b2 , \
			v_CalcParam.c2 , \
			v_CalcParam.delta);
        return 1;
    } else {
		memset(&v_CalcParam, 0, sizeof(v_CalcParam));
		v_Calibrated = 0;
		v_CalcParam.a1    =  v_CalcParam_bak.a1;
		v_CalcParam.b1    =  v_CalcParam_bak.b1 ;
		v_CalcParam.c1    =  v_CalcParam_bak.c1;
		v_CalcParam.a2    =  v_CalcParam_bak.a2;
		v_CalcParam.b2    =  v_CalcParam_bak.b2;
		v_CalcParam.c2    =  v_CalcParam_bak.c2;
		v_CalcParam.delta =  v_CalcParam_bak.delta;
        return 0;
    }
}

