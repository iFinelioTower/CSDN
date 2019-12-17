package com.cx.calibration;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View.OnTouchListener;
import android.widget.Toast;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "Calibration";

    private static final String CALIBRATE = "/sys/class/touchscreen/touchit_213/calibrate";

    private Window mWindow;
    CrossView mFullScreenView;

    int UI_SCREEN_WIDTH = 1280;
    int UI_SCREEN_HEIGHT = 720;

    int index;
    private Calibrate cal;

    static void setNotTitle(Activity act) {
        act.requestWindowFeature(Window.FEATURE_NO_TITLE);
    }

    static void setFullScreen(Activity act) {
        //setNotTitle(act);
        act.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
    }

    // Storage Permissions
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE };
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
        super.onCreate(savedInstanceState);
        setFullScreen(this);
        SystemUiVisibility();
        verifyStoragePermissions(this);

        DisplayMetrics dm = getResources().getDisplayMetrics();
        UI_SCREEN_WIDTH = dm.widthPixels;
        UI_SCREEN_HEIGHT = dm.heightPixels + 48;
        Log.e(TAG, "width-display :" + dm.widthPixels + ", heigth-display :" + dm.heightPixels);

        index = 0;
        cal = new Calibrate();
        mFullScreenView = new CrossView(this);

                setContentView(mFullScreenView);
                cal.startCalibration();

                mFullScreenView.setOnTouchListener(new OnTouchListener() {
                    StringBuilder sb = new StringBuilder();

                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        Log.i(TAG, "OnTouch " + event.getX() + "," + event.getY());
                        v.invalidate();
                        if (index < 4) {
                            Log.i(TAG, " time on TouchListener " + index);
                            sb.append(cal.getADCPoing() + ",");
                        }

                if (index == 4) {
                    sb.append(cal.getADCPoing() + ",");
                    sb.append(UI_SCREEN_WIDTH + "," + UI_SCREEN_HEIGHT); //Screen pixel.

                    cal.saveADCPoing(sb.toString());

                    boolean result = cal.checkCalibration();
                    if (result) {
                        Toast.makeText(getBaseContext(), "Calibrate done!", Toast.LENGTH_SHORT).show();
                        cal.WriteStringToFile(sb.toString());
                    } else {
                        Toast.makeText(getBaseContext(), "Calibrate failure, please try again!", Toast.LENGTH_SHORT).show();
                    }

                    MainActivity.this.finish();
                }

                index++;
                return false;
            }
        });
    }

    /**
     * 去掉底部导航栏
     */
    private void SystemUiVisibility() {
        mWindow = getWindow();
        WindowManager.LayoutParams params = mWindow.getAttributes();
        params.systemUiVisibility = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_IMMERSIVE
                | View.SYSTEM_UI_FLAG_FULLSCREEN;
        mWindow.setAttributes(params);
    }

    public class CrossView extends View {
        public CrossView(Context context) {
            super(context);
        }

        public void onDraw(Canvas canvas) {
            Paint paint = new Paint();
            paint.setColor(Color.GREEN);

            if (index == 0) {
                canvas.drawLine(40, 50, 60, 50, paint);
                canvas.drawLine(50, 40, 50, 60, paint);
                paint.setColor(Color.WHITE);
            } else if (index == 1) {
                canvas.drawLine(UI_SCREEN_WIDTH - 60, 50, UI_SCREEN_WIDTH - 40,
                        50, paint);
                canvas.drawLine(UI_SCREEN_WIDTH - 50, 40, UI_SCREEN_WIDTH - 50,
                        60, paint);
                paint.setColor(Color.WHITE);

            } else if (index == 2) {
                canvas.drawLine(40, UI_SCREEN_HEIGHT - 50, 60,
                        UI_SCREEN_HEIGHT - 50, paint);
                canvas.drawLine(50, UI_SCREEN_HEIGHT - 60, 50,
                        UI_SCREEN_HEIGHT - 40, paint);
                paint.setColor(Color.WHITE);

            } else if (index == 3) {
                canvas.drawLine(UI_SCREEN_WIDTH - 60, UI_SCREEN_HEIGHT - 50,
                        UI_SCREEN_WIDTH - 40, UI_SCREEN_HEIGHT - 50, paint);
                canvas.drawLine(UI_SCREEN_WIDTH - 50, UI_SCREEN_HEIGHT - 60,
                        UI_SCREEN_WIDTH - 50, UI_SCREEN_HEIGHT - 40, paint);
                paint.setColor(Color.WHITE);

            } else if (index == 4) {
                canvas.drawLine(UI_SCREEN_WIDTH / 2 - 10, UI_SCREEN_HEIGHT / 2,
                        UI_SCREEN_WIDTH / 2 + 10, UI_SCREEN_HEIGHT / 2, paint);
                canvas.drawLine(UI_SCREEN_WIDTH / 2, UI_SCREEN_HEIGHT / 2 - 10,
                        UI_SCREEN_WIDTH / 2, UI_SCREEN_HEIGHT / 2 + 10, paint);
                paint.setColor(Color.WHITE);

            } else {
                Log.d(TAG, "Finish calibrate!");
            }

            super.onDraw(canvas);
        }
    }

    public static void verifyStoragePermissions(Activity activity) {
        // Check if we have write permission
        int permission1 = ActivityCompat.checkSelfPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE);
        int permisson2 = ActivityCompat.checkSelfPermission(activity, Manifest.permission.READ_EXTERNAL_STORAGE);
        if (permission1 != PackageManager.PERMISSION_GRANTED && permisson2 != PackageManager.PERMISSION_GRANTED) {
            // We don't have permission so prompt the user
            ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE, REQUEST_EXTERNAL_STORAGE);
        }
    }
}
