/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.smartlockmanager;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.text.InputFilter;
import android.text.InputType;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.util.Arrays;
import java.util.List;

/**
 * Setup display fragments and ensure the device supports Bluetooth.
 */
public class MainActivity extends AppCompatActivity {
    public static MainActivity instance;

    private static final String TAG = "MA";
    private static final int ACCESS_COARSE_LOCATION_REQUEST = 2;
    private static final int REQUEST_ENABLE_BT = 1;
    private BLEManager mBLEManager;

    private ArrayAdapter<String> userListAdapter;

    private Handler connectionCheckHandler = new Handler();

    private ProgressBar simpleProgressBar = null;
    private TextView simpleProgressBarText = null;
    private ListView usersListView = null;

    public static MainActivity getInstance() {
        return instance;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        instance = this;

        dealWithPermissions();

        simpleProgressBar = findViewById(R.id.simpleProgressBar);
        simpleProgressBarText = findViewById(R.id.simpleProgressBarText);
        usersListView = findViewById(R.id.usersListView);
        usersListView.setOnItemClickListener((adapterView, view, position, l) -> {
            String value=userListAdapter.getItem(position);
            System.out.println("Clicked on " + value);

            AlertDialog.Builder alert = new AlertDialog.Builder(this);

            alert.setTitle(value);
            alert.setMessage("Operations:");

            LinearLayout lila1= new LinearLayout(this);
            lila1.setOrientation(LinearLayout.VERTICAL);

            final Button deleteButton = new Button(this);
            deleteButton.setText("Delete");

            lila1.addView(deleteButton);
            alert.setView(lila1);
            alert.setNegativeButton("Cancel", (dialog, whichButton) -> {
                // Canceled.
            });

            AlertDialog ad = alert.show();

            deleteButton.setOnClickListener(view1 -> {
                deleteUser(value);

                ad.cancel();
            });
        });

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE, WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE);
        simpleProgressBar.setVisibility(View.VISIBLE);
        simpleProgressBarText.setVisibility(View.VISIBLE);
        simpleProgressBarText.setText("Connecting...");

        mBLEManager = new BLEManager(MainActivity.this);
        mBLEManager.scanAndConnect();

        connectionCheckHandler.post(new Runnable() {
            @Override
            public void run() {
                System.out.println("Checking connection");
                if (mBLEManager.getConnectionState()) {
                    System.out.println("Connected");
                    getWindow().clearFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE);
                    simpleProgressBar.setVisibility(View.GONE);
                    simpleProgressBarText.setVisibility(View.GONE);

                    Toast toast=Toast.makeText(getApplicationContext(),"Connection to board successful",Toast.LENGTH_LONG);
                    toast.setMargin(50,50);
                    toast.show();

                    getUsers();

                    connectionCheckHandler.removeCallbacks(this);
                } else
                    connectionCheckHandler.postDelayed(this, 1000);
            }
        });
    }

    private void getUsers() {
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE, WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE);

        runOnUiThread(() ->simpleProgressBar.setVisibility(View.VISIBLE));
        runOnUiThread(() ->simpleProgressBarText.setVisibility(View.VISIBLE));
        runOnUiThread(() ->simpleProgressBarText.setText("Getting users..."));

        mBLEManager.sendMessage("AT+GETUSERS=\r\n");
    }

    private void addUser(String name, String password) {
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE, WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE);

        runOnUiThread(() ->simpleProgressBar.setVisibility(View.VISIBLE));
        runOnUiThread(() ->simpleProgressBarText.setVisibility(View.VISIBLE));
        runOnUiThread(() ->simpleProgressBarText.setText("Adding user..."));

        mBLEManager.sendMessage("AT+ADDUSER=" + name + "," + password + "\r\n");
    }

    private void deleteUser(String name) {
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE, WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE);

        runOnUiThread(() ->simpleProgressBar.setVisibility(View.VISIBLE));
        runOnUiThread(() ->simpleProgressBarText.setVisibility(View.VISIBLE));
        runOnUiThread(() ->simpleProgressBarText.setText("Deleting user..."));

        mBLEManager.sendMessage("AT+DELETEUSER=" + name + "\r\n");
    }

    public void onAddUserFab(View view) {
        System.out.println("Add user clicked");

        AlertDialog.Builder alert = new AlertDialog.Builder(this);

        alert.setTitle("Register");
        alert.setMessage("Input name and password");

        LinearLayout lila1= new LinearLayout(this);
        lila1.setOrientation(LinearLayout.VERTICAL);

        final EditText nameInput = new EditText(this);
        nameInput.setFilters(new InputFilter[] {new InputFilter.LengthFilter(32)});
        nameInput.setHint("Enter name here");

        final EditText passwordInput = new EditText(this);
        passwordInput.setFilters(new InputFilter[] {new InputFilter.LengthFilter(6)});
        passwordInput.setInputType(InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_VARIATION_PASSWORD);
        passwordInput.setHint("Enter password here");

        lila1.addView(nameInput);
        lila1.addView(passwordInput);
        alert.setView(lila1);

        alert.setPositiveButton("Ok", (dialog, whichButton) -> {
            String name = String.valueOf(nameInput.getText());
            String password = String.valueOf(passwordInput.getText());
            addUser(name, password);
        });

        alert.setNegativeButton("Cancel", (dialog, whichButton) -> {
            // Canceled.
        });

        alert.show();
    }

    public void processBLEMessage(String message) {
        String command = message.replace("\n", "").replace("\r", "");
        System.out.println("Got BLE message: " + command);

        runOnUiThread(() -> simpleProgressBar.setVisibility(View.GONE));
        runOnUiThread(() -> simpleProgressBarText.setVisibility(View.GONE));
        runOnUiThread(() -> getWindow().clearFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE));

        if (command.contains("AT+GETUSERS=")) {
            String usersRaw = command.substring(command.indexOf("=") + 1);
            if (usersRaw.length() == 0) {
                runOnUiThread(() -> usersListView.setAdapter(null));
            } else {
                List<String> userList = Arrays.asList(usersRaw.split(","));
                userListAdapter = new ArrayAdapter<>(this,
                        android.R.layout.simple_list_item_1, android.R.id.text1, userList);
                runOnUiThread(() -> usersListView.setAdapter(userListAdapter));
            }
        }
        if (command.contains("AT+ADDUSER=")) {
            String response = command.substring(command.indexOf("=") + 1);
            if (response.contains("OK")) {
                getUsers();
            }
        }
        if (command.contains("AT+DELETEUSER=")) {
            String response = command.substring(command.indexOf("=") + 1);
            if (response.contains("OK")) {
                getUsers();
            }
        }
    }

    private void dealWithPermissions() {
        if (getApplicationContext().checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[] { Manifest.permission.ACCESS_COARSE_LOCATION }, ACCESS_COARSE_LOCATION_REQUEST);
        }

        if (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_DENIED)
        {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S)
            {
                ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 2);
            }
        }

        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (!bluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }
    }
}