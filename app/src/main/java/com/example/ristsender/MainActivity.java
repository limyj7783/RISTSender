package com.example.ristsender;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.Button;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    RIST rist;
    Button sendStart;
    Button receiveStart;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        rist = new RIST();

        sendStart = (Button)findViewById(R.id.btn_SendStart);
        sendStart.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View view)
            {
                rist.SendStart("", 8000);
            }
        }) ;

        receiveStart = (Button)findViewById(R.id.btn_ReceiveStart);
        receiveStart.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View view)
            {
                rist.ReceiveStart();
            }
        }) ;
    }
}