package com.example.smartlockmanager;

import static android.bluetooth.BluetoothDevice.TRANSPORT_LE;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.util.Log;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

public class BLEManager {
    private static final String TAG = "BLEManager";
    private BluetoothGatt mBluetoothGatt;
    private final Context mContext;

    private final UUID SERVICE_UUID = UUID.fromString("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    private final UUID CHARACTERISTIC_UUID_RX = UUID.fromString("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
    private final UUID CHARACTERISTIC_UUID_TX = UUID.fromString("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
    private final UUID DESCRIPTOR_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
    private boolean connectionState = false;

    public BLEManager(Context context) {
        this.mContext = context;
    }

    public void scanAndConnect() {
        BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        BluetoothLeScanner scanner = mBluetoothAdapter.getBluetoothLeScanner();

        List<ScanFilter> filters = new ArrayList<>();
        filters.add(new ScanFilter.Builder() .setDeviceName("ESP32CONTROL").build());

        ScanSettings scanSettings = new ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_POWER)
                .setCallbackType(ScanSettings.CALLBACK_TYPE_FIRST_MATCH)
                .setMatchMode(ScanSettings.MATCH_MODE_AGGRESSIVE)
                .setNumOfMatches(ScanSettings.MATCH_NUM_ONE_ADVERTISEMENT)
                .setReportDelay(0L)
                .build();

        if (scanner != null) {
            scanner.startScan(filters, scanSettings, scanCallback);
            Log.d(TAG, "scan started");
        }  else {
            Log.e(TAG, "could not get scanner object");
        }
    }

    public void sendMessage(String message) {
        System.out.println("Sending over BLE: " + message);
        BluetoothGattCharacteristic writeChar = mBluetoothGatt.getService(SERVICE_UUID).getCharacteristic(CHARACTERISTIC_UUID_RX);
        writeChar.setValue(message.getBytes(StandardCharsets.UTF_8));
        mBluetoothGatt.writeCharacteristic(writeChar);
    }

    public boolean getConnectionState() {
        return this.connectionState;
    }
    
    private final ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            BluetoothDevice device = result.getDevice();
            Log.d(TAG, "Found: " + device.getName() + " " + device.getAddress());

            mBluetoothGatt = device.connectGatt(mContext, false, BLEGattCallback, TRANSPORT_LE);
        }

        @Override
        public void onBatchScanResults(List<ScanResult> results) {}

        @Override
        public void onScanFailed(int errorCode) {}
    };

    private final BluetoothGattCallback BLEGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                gatt.discoverServices();
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                List<BluetoothGattService> services = gatt.getServices();
                for (BluetoothGattService service : services) {
                    if (!service.getUuid().equals(SERVICE_UUID))
                        continue;

                    List<BluetoothGattCharacteristic> gattCharacteristics = service.getCharacteristics();

                    for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {
                        if (!gattCharacteristic.getUuid().equals(CHARACTERISTIC_UUID_TX))
                            continue;

                        final int charaProp = gattCharacteristic.getProperties();

                        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
                            mBluetoothGatt.setCharacteristicNotification(gattCharacteristic, true);
                            BluetoothGattDescriptor descriptor = gattCharacteristic.getDescriptor(DESCRIPTOR_UUID);
                            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                            mBluetoothGatt.writeDescriptor(descriptor);
                        } else {
                            Log.w(TAG, "Characteristic does not support notify");
                        }
                    }
                }
            } else {
                Log.w(TAG, "onServicesDiscovered received: " + status);
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicRead(gatt, characteristic, status);
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicWrite(gatt, characteristic, status);
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
            System.out.println("Got raw: " + Arrays.toString(characteristic.getValue()));
            /* Message received here */
            MainActivity.getInstance().processBLEMessage(new String(characteristic.getValue(), StandardCharsets.UTF_8));
        }

        @Override
        public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            super.onDescriptorRead(gatt, descriptor, status);
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            super.onDescriptorWrite(gatt, descriptor, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (descriptor.getCharacteristic().getUuid().equals(CHARACTERISTIC_UUID_TX)) {
                    Log.i(TAG, "Successfully subscribed");
                    gatt.requestMtu(512);
                }
            } else {
                Log.e(TAG, "Error subscribing");
            }
        }

        @Override
        public void onReliableWriteCompleted(BluetoothGatt gatt, int status) {
            super.onReliableWriteCompleted(gatt, status);
        }

        @Override
        public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
            super.onReadRemoteRssi(gatt, rssi, status);
        }

        @Override
        public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
            super.onMtuChanged(gatt, mtu, status);
            System.out.println("Configured MTU, connection established completely");
            connectionState = true;
        }
    };
}
