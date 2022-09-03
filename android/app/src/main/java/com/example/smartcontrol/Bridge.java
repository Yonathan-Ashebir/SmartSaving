package com.example.smartcontrol;

import android.webkit.JavascriptInterface;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

class Bridge {
    @JavascriptInterface
    public Object invokeMethod(Object obj, String name, Object... params) throws NoSuchMethodException, InvocationTargetException, IllegalAccessException, NoSuchFieldException {
        try {
            return getMethod(obj, name, params).invoke(obj, params);
        } catch (Exception exception) {
            exception.printStackTrace();
            throw exception;
        }
    }

    @JavascriptInterface
    public Object invokeMethod(Object obj, String name) throws NoSuchMethodException, InvocationTargetException, IllegalAccessException, NoSuchFieldException {
        return invokeMethod(obj, name, null);
    }


    @JavascriptInterface
    public Object getField(Object obj, String name) throws NoSuchFieldException, IllegalAccessException {
        if (obj == null || name == null || name == "") throw new IllegalArgumentException();
        String[] indents = name.split("[.]", 0);
        if (indents.length < 1) throw new IllegalArgumentException();
        Object result = obj;
        for (String indent : indents) {
            result = _getField(result, indent);
        }
        return result;
    }

    private Object _getField(Object obj, String name) throws NoSuchFieldException, IllegalAccessException {
        try {
            return obj.getClass().getField(name).get(obj);
        } catch (Exception exception) {
            exception.printStackTrace();
            throw exception;
        }
    }

    private Method getMethod(Object obj, String name, Object... params) throws NoSuchMethodException, NoSuchFieldException, IllegalAccessException {
        if (name == null || name == "") throw new IllegalArgumentException();
        try {
            Class[] paramClasses = null;
            if (params != null) {
                paramClasses = new Class[params.length];
                for (int i = 0; i < params.length; i++) {
                    paramClasses[i] = params[i].getClass();
                }
            }
            if (name.indexOf(".") != -1) {
                String _name = name.substring(name.lastIndexOf("."));
                if (_name == "") throw new IllegalArgumentException();
                obj = getField(obj, name.substring(0, name.lastIndexOf(".")));
                name = _name;
            }
            return obj.getClass().getMethod(name, paramClasses);
        } catch (Exception exception) {
            exception.printStackTrace();
            throw exception;
        }
    }
}