#include "Arduino.h"

#include <SPI.h>
#include <MFRC522.h>

#include "board.h"
#include "utils.h"

#include "app_ble.h"
#include "app_nfc.h"
#include "app_user_manager.h"
#include "app_vision.h"
#include "app_display.h"

#include "utils.h"

MFRC522 mfrc522(SS_PIN, RST_PIN);

static void vNfcTask(void *pvParameters);

void NFCInit()
{
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
    mfrc522.PCD_Init();
    delay(4);
    mfrc522.PCD_DumpVersionToSerial();
    xTaskCreate(vNfcTask, vNfcTaskName, vNfcTaskStackSize, NULL, vNfcTaskPriority, NULL);
}

static void vNfcTask(void *pvParameters)
{
    for (;;)
    {
        if (program_state == kStateReadNFC || program_state == kStateRegisterNFC)
        {
            if (mfrc522.PICC_IsNewCardPresent())
            {
                if (mfrc522.PICC_ReadCardSerial())
                {
                    char nfc_id[22] = "";
                    byte_array_to_string(mfrc522.uid.uidByte, mfrc522.uid.size, nfc_id);

                    LOG_MSG("Read NFC tag: %s\n", nfc_id);

                    if (program_state == kStateRegisterNFC)
                    {
                        strcpy(user.nfc_id, nfc_id);

                        LOG_MSG("Registered nfc, moving to vision registration\n");

                        char msg[50] = "";
                        snprintf(msg, 50, "AT+REGISTER=%s\n", user.name);
                        VisionSend((const char *)msg);

                        program_state = kStateRegisterFace;
                        play_tone(kToneSuccess);
                    }
                    else if (program_state == kStateReadNFC)
                    {
                        Serial.println("Reading nfc tag success, moving to vision recognition");

                        strcpy(user.nfc_id, nfc_id);
                        VisionSend((const char *)"AT+READUSER=\n");

                        program_state = kStateReadFace;
                        play_tone(kToneSuccess);
                    }

                    mfrc522.PICC_HaltA();
                    mfrc522.PCD_StopCrypto1();
                }
            }
        }
        delay(50);
    }
}