#include "Arduino.h"

#include "HardwareSerial.h"

#include "board.h"

#include "app_vision.h"
#include "app_user_manager.h"
#include "app_ble.h"
#include "app_display.h"

#include "utils.h"

HardwareSerial SerialPort(1);

static void processMessage(const char *msg);

static void vVisionTask(void *pvParameters)
{
    for (;;)
    {
        if (SerialPort.available())
        {
            String msg = SerialPort.readString();

            LOG_MSG("Receiving message from vision: %s\n", msg.c_str());

            processMessage(msg.c_str());
        }

        delay(10);
    }
}

static void processMessage(const char *msg)
{
    if (strstr(msg, "AT+REGISTER="))
    {
        int register_result;
        sscanf(msg, "AT+REGISTER=%d\r\n", &register_result);

        LOG_MSG("Received registration result: %d\nSaving user: %s\n", register_result, user.name);

        if (register_result > 0)
        {
            UserDatabaseSaveUser(user.name, user.password, user.nfc_id);
            BleServerSend((uint8_t *)"AT+ADDUSER=OK\r\n", strlen("AT+ADDUSER=OK\r\n"));
            program_state = kStateRegisterSuccess;
            play_tone(kToneSuccess);
        }
        else
        {
            // this is timeout
        }

        delay(2000);

        LOG_MSG("Reseting program state to reading\n");
        reset_temp_user();
        program_state = kStateReadPinpad;
    }
    else if (strstr(msg, "AT+DELETEUSER="))
    {
        int delete_result;
        sscanf(msg, "AT+DELETEUSER=%d\r\n", &delete_result);

        if (delete_result > 0)
        {
            LOG_MSG("User delete success\n");
            BleServerSend((uint8_t *)"AT+DELETEUSER=OK\r\n", strlen("AT+DELETEUSER=OK\r\n"));
        }
    }
    else if (strstr(msg, "AT+READUSER="))
    {
        sscanf(msg, "AT+READUSER=%s\r\n", &user.name);

        LOG_MSG("Vision recognised: %s\nAttempting to match...\n", user.name);

        bool result = UserDatabaseMatchUser(user.name, user.password, user.nfc_id);

        if (result)
        {
            LOG_MSG("Door opened\n");
            program_state = kStateReadSuccess;
            play_tone(kToneSuccess);
            digitalWrite(RELAY_PIN, LOW);
            delay(3000);
            digitalWrite(RELAY_PIN, HIGH);
            play_tone(kToneSuccess);
        }
        else
        {
            LOG_MSG("One method was wrong\n");
            program_state = kStateReadFail;
            play_tone(kToneFail);
        }

        delay(2000);

        LOG_MSG("Reseting program state to reading\n");
        reset_temp_user();
        program_state = kStateReadPinpad;
    }
}

void VisionSend(const char *msg)
{
    LOG_MSG("Sending message to vision: %s\n", msg);
    SerialPort.print(msg);
}

void VisionInit()
{
    SerialPort.begin(115200, SERIAL_8N1, VISION_UART_RX_PIN, VISION_UART_TX_PIN);
    xTaskCreate(vVisionTask, vVisionTaskName, vVisionTaskStackSize, NULL, vVisionTaskPriority, NULL);
}
