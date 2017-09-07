package red.tel.chat.camera;

/**
 * Created by vmodev on 9/6/17.
 */

public enum Facing {
    /**
     * Back-facing camera sensor.
     */
    BACK(0),

    /**
     * Front-facing camera sensor.
     */
    FRONT(1);

    final static Facing DEFAULT = BACK;

    private int value;

    Facing(int value) {
        this.value = value;
    }

    int value() {
        return value;
    }

    static Facing fromValue(int value) {
        Facing[] list = Facing.values();
        for (Facing action : list) {
            if (action.value() == value) {
                return action;
            }
        }
        return null;
    }
}
