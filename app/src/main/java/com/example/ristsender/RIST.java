package com.example.ristsender;

public class RIST
{
    static
    {
        System.loadLibrary("ristprotocol");
    }


    public native int getTestValue();
    public native int SendStart(String url, int port);
    public native int SendStop();
    public native int ReceiveStart();
    public native int ReceiveStop();
}
