package com.cx.calibration;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintStream;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.widget.Toast;

/**
 * Created by gusx on 2019/12/2.
 */

public class Calibrate {
    private static final String TAG = "Calibration";

    private static final String CONFIG = "/data/private/calibargs";
    private static final String TOUCH_ADC = "/sys/class/calib_ifaces/touchadc";
    private static final String RECALIBRATION = "/sys/class/calib_ifaces/recalibration";

    private calibration cal;
    public Calibrate() {
        cal = new calibration();
    }

    class calibration {

    };

    public String getADCPoing() {
        File file = new File(TOUCH_ADC);
        if (file.exists()) {
            try {
                BufferedReader reader = new BufferedReader(new FileReader(TOUCH_ADC));
                String result = reader.readLine();
                Log.e(TAG, "getADCPoing: " + result);
                return result;
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }

        return null;
    }

    public void WriteStringToFile(String data) {
        try {
            FileOutputStream fos = new FileOutputStream(CONFIG);
            fos.write(data.getBytes());
            fos.close();
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    public boolean saveADCPoing(String adc) {
        File file = new File(TOUCH_ADC);
        if (file.exists()) {
            try {
                FileWriter wr = new FileWriter(TOUCH_ADC);
                wr.write(adc);
                wr.flush();
                wr.close();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return true;
    }

    public void startCalibration() {
        File file = new File(RECALIBRATION);
        if (file.exists()) {
            try {
                FileWriter wr = new FileWriter(RECALIBRATION);
                wr.write("1");
                wr.flush();
                wr.close();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public boolean checkCalibration() {
        File file = new File(RECALIBRATION);
        if (file.exists()) {
            try {
                BufferedReader reader = new BufferedReader(new FileReader(RECALIBRATION));
                String result = reader.readLine();
                Log.e(TAG, "checkCalibration: " + result);
                if (result.contains("successful")) {
                    return true;
                }
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }

        return false;
    }
}
