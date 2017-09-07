package red.tel.chat.camera;

import android.content.Context;
import android.hardware.SensorManager;
import android.support.annotation.NonNull;
import android.util.SparseIntArray;
import android.view.Display;
import android.view.OrientationEventListener;
import android.view.Surface;

/**
 * Created by vmodev on 9/6/17.
 */

public class OrientationHelper {
    private static final SparseIntArray DISPLAY_ORIENTATIONS = new SparseIntArray();
    static {
        DISPLAY_ORIENTATIONS.put(Surface.ROTATION_0, 0);
        DISPLAY_ORIENTATIONS.put(Surface.ROTATION_90, 90);
        DISPLAY_ORIENTATIONS.put(Surface.ROTATION_180, 180);
        DISPLAY_ORIENTATIONS.put(Surface.ROTATION_270, 270);
    }

    private final OrientationEventListener mListener;
    private final Callbacks mCallbacks;
    private Display mDisplay;
    private int mLastKnownDisplayOffset = -1;
    private int mLastOrientation = -1;

    interface Callbacks {
        void onDisplayOffsetChanged(int displayOffset);
        void onDeviceOrientationChanged(int deviceOrientation);
    }

    OrientationHelper(Context context, @NonNull Callbacks callbacks) {
        mCallbacks = callbacks;
        mListener = new OrientationEventListener(context, SensorManager.SENSOR_DELAY_NORMAL) {

            @Override
            public void onOrientationChanged(int orientation) {
                int or = 0;
                if (orientation == OrientationEventListener.ORIENTATION_UNKNOWN) {
                    or = 0;
                } else if (orientation >= 315 || orientation < 45) {
                    or = 0;
                } else if (orientation >= 45 && orientation < 135) {
                    or = 90;
                } else if (orientation >= 135 && orientation < 225) {
                    or = 180;
                } else if (orientation >= 225 && orientation < 315) {
                    or = 270;
                }

                if (or != mLastOrientation) {
                    mLastOrientation = or;
                    mCallbacks.onDeviceOrientationChanged(mLastOrientation);
                }

                // Let's see if display rotation has changed.. but how could it ever change...??
                // This makes no sense apparently. I'll leave it for now.
                if (mDisplay != null) {
                    final int offset = mDisplay.getRotation();
                    if (mLastKnownDisplayOffset != offset) {
                        mLastKnownDisplayOffset = offset;
                        mCallbacks.onDisplayOffsetChanged(DISPLAY_ORIENTATIONS.get(offset));
                    }
                }
            }

        };
    }

    void enable(Display display) {
        mDisplay = display;
        mListener.enable();
        mLastKnownDisplayOffset = DISPLAY_ORIENTATIONS.get(display.getRotation());
        mCallbacks.onDisplayOffsetChanged(mLastKnownDisplayOffset);
    }

    void disable() {
        mListener.disable();
        mDisplay = null;
    }
}
