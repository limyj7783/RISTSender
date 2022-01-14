package com.example.ristsender;

public class RIST
{
    static
    {
        System.loadLibrary("ristprotocol");
    }


    public native int getTestValue();
}
