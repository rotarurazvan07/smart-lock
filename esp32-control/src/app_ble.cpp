// #include "Arduino.h"

// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>

// #include "app_ble.h"
// #include "app_user_manager.h"
// #include "app_nfc.h"
// #include "app_vision.h"
// #include "app_display.h"

// #include "utils.h"

// static BLEServer *pServer = NULL;
// static BLEService *pService = NULL;
// static BLECharacteristic *pTxCharacteristic = NULL;

// static bool deviceConnected = false;

// class BLEServCallbacks : public BLEServerCallbacks
// {
//     void onConnect(BLEServer *pServer)
//     {
//         deviceConnected = true;
//     };

//     void onDisconnect(BLEServer *pServer)
//     {
//         deviceConnected = false;
//         delay(500);
//         pServer->startAdvertising();
//     }
// };

// class BLECallbacks : public BLECharacteristicCallbacks
// {
//     void onWrite(BLECharacteristic *pCharacteristic)
//     {
//         std::string rxValue = pCharacteristic->getValue();
//         if (rxValue.length() > 0)
//         {
//             LOG_MSG("Received BLE UART string: %s\n", rxValue.c_str());

//             processMessage((char *)rxValue.c_str());
//         }
//     }

//     void processMessage(char *msg)
//     {
//         if (strstr(msg, "AT+ADDUSER="))
//         {
//             char line[40] = "";
//             sscanf(msg, "AT+ADDUSER=%s\r\n", line);

//             strcpy(user.name, strtok(line, ","));
//             strcpy(user.password, strtok(NULL, ","));

//             LOG_MSG("Saved user name: %s and password: %s\nMoving to NFC REGISTER\n", user.name, user.password);

//             program_state = kStateRegisterNFC;
//         }
//         else if (strstr(msg, "AT+DELETEUSER="))
//         {
//             char name[40] = "";
//             sscanf(msg, "AT+DELETEUSER=%s\r\n", name);

//             LOG_MSG("Deleting user: %s\n", name);

//             UserDatabaseDeleteUser(name);
//             VisionSend(msg);
//         }
//         else if (strstr(msg, "AT+GETUSERS="))
//         {
//             UserDatabaseGetUsers();
//         }
//         else if (strstr(msg, "AT+RESETDB="))
//         {
//             UserDatabaseReset();
//         }
//     }
// };

// void BleServerInit()
// {
//     BLEDevice::init(BLE_NAME);
//     BLEDevice::setMTU(BLE_MTU_SIZE);

//     pServer = BLEDevice::createServer();
//     pServer->setCallbacks(new BLEServCallbacks());

//     pService = pServer->createService(SERVICE_UUID);

//     pTxCharacteristic = pService->createCharacteristic(
//         CHARACTERISTIC_UUID_TX,
//         BLECharacteristic::PROPERTY_NOTIFY);
//     pTxCharacteristic->addDescriptor(new BLE2902());

//     BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
//         CHARACTERISTIC_UUID_RX,
//         BLECharacteristic::PROPERTY_WRITE);
//     pRxCharacteristic->setCallbacks(new BLECallbacks());

//     pService->start();

//     pServer->getAdvertising()->addServiceUUID(pService->getUUID());
//     pServer->getAdvertising()->start();
// }

// void BleServerSend(uint8_t *data, size_t size)
// {
//     if (deviceConnected)
//     {
//         LOG_MSG("Sending over BLE: %s\n", (char *)data);
//         pTxCharacteristic->setValue(data, size);
//         pTxCharacteristic->notify();
//         delay(10);
//     }
// }