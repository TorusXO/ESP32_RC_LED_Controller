package org.qtproject.qt.android;

import android.app.Activity;
import android.content.Context;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;

/**
 * Keep Qt text fields inline when Android opens a landscape keyboard.
 *
 * Qt 6.10's QtEditText does not expose IME options through QML. Keep this
 * small compatibility bridge in Qt's package to access its editor options
 * without reflection or modifying the installed Qt SDK. Recheck this bridge
 * when upgrading Qt. Both listeners run on Android's UI thread.
 */
public final class InlineKeyboard {
    private final View root;
    private final ViewTreeObserver.OnGlobalFocusChangeListener focusListener;
    private final ViewTreeObserver.OnGlobalLayoutListener layoutListener;

    public InlineKeyboard(Activity activity) {
        root = activity.getWindow().getDecorView();
        // Focus changes occur before the IME creates its input connection.
        focusListener = (previous, current) -> configure(current, false);
        // Qt resets its options and requests layout on every keyboard show,
        // including reopening the keyboard on an already-focused field.
        layoutListener = () -> configure(root.findFocus(), true);
        root.getViewTreeObserver().addOnGlobalFocusChangeListener(focusListener);
        root.getViewTreeObserver().addOnGlobalLayoutListener(layoutListener);
    }

    private void configure(View focusedView, boolean restartInput) {
        if (!(focusedView instanceof QtEditText))
            return;

        QtEditText editor = (QtEditText) focusedView;
        int options = editor.m_imeOptions
                | EditorInfo.IME_FLAG_NO_EXTRACT_UI
                | EditorInfo.IME_FLAG_NO_FULLSCREEN;
        if (options == editor.m_imeOptions)
            return;

        editor.m_imeOptions = options;
        editor.m_optionsChanged = true;
        if (restartInput) {
            InputMethodManager manager = (InputMethodManager)
                    root.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
            if (manager != null)
                manager.restartInput(editor);
        }
    }

    public void dispose() {
        ViewTreeObserver observer = root.getViewTreeObserver();
        if (observer.isAlive()) {
            observer.removeOnGlobalFocusChangeListener(focusListener);
            observer.removeOnGlobalLayoutListener(layoutListener);
        }
    }
}
