package com.spiralknightsinvoked.rcledcontroller;

import android.view.KeyEvent;
import android.view.InputDevice;
import android.view.KeyCharacterMap;
import android.view.MotionEvent;
import android.os.SystemClock;

import org.qtproject.qt.android.bindings.QtActivity;

/**
 * Qt's Android input bridge intentionally drops Android gamepad buttons.
 * Retroid exposes its controls using those keycodes, so map them to ordinary
 * Qt-visible keys before forwarding the event to Qt.
 */
public class RetroidActivity extends QtActivity {
    private boolean leftStickUp;
    private boolean leftStickDown;

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int mappedKeyCode = mapControllerButton(event.getKeyCode());
        boolean dpadEvent = isDpadKey(event.getKeyCode());
        if (mappedKeyCode != event.getKeyCode() || dpadEvent) {
            event = new KeyEvent(
                event.getDownTime(),
                event.getEventTime(),
                event.getAction(),
                mappedKeyCode,
                event.getRepeatCount(),
                event.getMetaState(),
                KeyCharacterMap.VIRTUAL_KEYBOARD,
                event.getScanCode(),
                event.getFlags(),
                InputDevice.SOURCE_KEYBOARD
            );
        }

        return super.dispatchKeyEvent(event);
    }

    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent event) {
        if ((event.getSource() & InputDevice.SOURCE_JOYSTICK) ==
                InputDevice.SOURCE_JOYSTICK &&
            event.getActionMasked() == MotionEvent.ACTION_MOVE) {
            float vertical = event.getAxisValue(MotionEvent.AXIS_Y);
            boolean up = vertical < -0.55f;
            boolean down = vertical > 0.55f;

            if (up != leftStickUp) {
                leftStickUp = up;
                dispatchSyntheticScroll(
                    KeyEvent.KEYCODE_PAGE_UP,
                    up ? KeyEvent.ACTION_DOWN : KeyEvent.ACTION_UP
                );
            }

            if (down != leftStickDown) {
                leftStickDown = down;
                dispatchSyntheticScroll(
                    KeyEvent.KEYCODE_PAGE_DOWN,
                    down ? KeyEvent.ACTION_DOWN : KeyEvent.ACTION_UP
                );
            }

        }

        return super.dispatchGenericMotionEvent(event);
    }

    private void dispatchSyntheticScroll(int keyCode, int action) {
        long now = SystemClock.uptimeMillis();
        dispatchKeyEvent(new KeyEvent(
            now,
            now,
            action,
            keyCode,
            0,
            0,
            KeyCharacterMap.VIRTUAL_KEYBOARD,
            0,
            KeyEvent.FLAG_VIRTUAL_HARD_KEY,
            InputDevice.SOURCE_KEYBOARD
        ));
    }

    private static int mapControllerButton(int keyCode) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            return KeyEvent.KEYCODE_ESCAPE;
        }
        if (keyCode == KeyEvent.KEYCODE_BUTTON_MODE ||
            keyCode == KeyEvent.KEYCODE_BUTTON_SELECT ||
            keyCode == KeyEvent.KEYCODE_BUTTON_START ||
            keyCode == KeyEvent.KEYCODE_MENU) {
            return KeyEvent.KEYCODE_F1;
        }
        if (keyCode == KeyEvent.KEYCODE_BUTTON_A) {
            return KeyEvent.KEYCODE_ENTER;
        }
        if (keyCode == KeyEvent.KEYCODE_BUTTON_B) {
            return KeyEvent.KEYCODE_A;
        }
        if (keyCode == KeyEvent.KEYCODE_BUTTON_X) {
            return KeyEvent.KEYCODE_X;
        }
        if (keyCode == KeyEvent.KEYCODE_BUTTON_Y) {
            return KeyEvent.KEYCODE_Y;
        }
        if (keyCode == KeyEvent.KEYCODE_BUTTON_L1 ||
            keyCode == KeyEvent.KEYCODE_BUTTON_L2) {
            return KeyEvent.KEYCODE_MEDIA_PREVIOUS;
        }
        if (keyCode == KeyEvent.KEYCODE_BUTTON_R1 ||
            keyCode == KeyEvent.KEYCODE_BUTTON_R2) {
            return KeyEvent.KEYCODE_MEDIA_NEXT;
        }
        return keyCode;
    }

    private static boolean isDpadKey(int keyCode) {
        return keyCode == KeyEvent.KEYCODE_DPAD_UP ||
            keyCode == KeyEvent.KEYCODE_DPAD_DOWN ||
            keyCode == KeyEvent.KEYCODE_DPAD_LEFT ||
            keyCode == KeyEvent.KEYCODE_DPAD_RIGHT;
    }
}
