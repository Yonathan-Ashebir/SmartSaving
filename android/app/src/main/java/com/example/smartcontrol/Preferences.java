package com.example.smartcontrol;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;
import android.webkit.JavascriptInterface;

public class Preferences {
    private SharedPreferences preferences;
    private SharedPreferences.Editor editor;
    private boolean autoCommit = true;
    private Context temp;

    public Preferences(Context context) {temp = context;
        this.preferences = context.getSharedPreferences("game", Context.MODE_PRIVATE);
        this.editor = this.preferences.edit();
    }

    @JavascriptInterface
    public void setAutoCommit(boolean autoCommit) {
        this.autoCommit = autoCommit;
    }

    @JavascriptInterface
    public void setString(String k, String v) {
        Log.d("yoni","set string: " + k + " " + v);
        editor.putString(k, v);
        if (autoCommit) editor.apply();
    }

    @JavascriptInterface
    public void setBoolean(String k, boolean v) {
        editor.putBoolean(k, v);
        if (autoCommit) editor.apply();
    }

    @JavascriptInterface
    public void setInteger(String k, int v) {
        editor.putInt(k, v);
        if (autoCommit) editor.apply();
    }

    @JavascriptInterface
    public void setFloat(String k, float v) {
        editor.putFloat(k, v);
        if (autoCommit) editor.apply();
    }

    @JavascriptInterface
    public void setLong(String k, long v) {
        editor.putLong(k, v);
        if (autoCommit) editor.apply();
    }

    @JavascriptInterface
    public String getString(String k) {
        return this.preferences.getString(k, "");
    }

    @JavascriptInterface
    public int getInteger(String k) {
        return this.preferences.getInt(k, 0);
    }

    @JavascriptInterface
    public float getFloat(String k) {
        return this.preferences.getFloat(k, 0f);
    }

    @JavascriptInterface
    public long getLong(String k) {
        return this.preferences.getLong(k, 0l);
    }
     @JavascriptInterface
    public boolean getBoolean(String k){
         return this.preferences.getBoolean(k, false);
     }

}
