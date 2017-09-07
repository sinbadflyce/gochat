package red.tel.chat.camera;

import android.annotation.TargetApi;
import android.hardware.Camera;
import android.hardware.camera2.CameraCharacteristics;
import android.util.SizeF;

/**
 * Created by vmodev on 9/6/17.
 */

public class ExtraProperties {
    float verticalViewingAngle;
    float horizontalViewingAngle;

    ExtraProperties(Camera.Parameters params) {
        verticalViewingAngle = params.getVerticalViewAngle();
        horizontalViewingAngle = params.getHorizontalViewAngle();
    }

    @TargetApi(21)
    ExtraProperties(CameraCharacteristics chars) {
        float[] maxFocus = chars.get(CameraCharacteristics.LENS_INFO_AVAILABLE_FOCAL_LENGTHS);
        SizeF size = chars.get(CameraCharacteristics.SENSOR_INFO_PHYSICAL_SIZE);
        verticalViewingAngle = (float) Math.toDegrees(2 * Math.atan(size.getWidth() / (maxFocus[0] * 2)));
        horizontalViewingAngle = (float) Math.toDegrees(2 * Math.atan(size.getHeight() / (maxFocus[0] * 2)));
    }

    public float getHorizontalViewingAngle() {
        return horizontalViewingAngle;
    }

    public float getVerticalViewingAngle() {
        return verticalViewingAngle;
    }
}
