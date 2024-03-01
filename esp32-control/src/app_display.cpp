#include "Arduino.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "app_display.h"
#include "app_user_manager.h"

#include "utils.h"

Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

static void vDisplayTask(void *pvParameters);

void DisplayInit()
{
    if (!oledDisplay.begin(SSD1306_SWITCHCAPVCC, DIPLAY_I2C_ADDRESS))
    {
        LOG_MSG("SSD1306 allocation failed");
        for (;;)
            ;
    }

    delay(2000);
    oledDisplay.clearDisplay();
    oledDisplay.setTextSize(1);
    oledDisplay.setTextColor(WHITE);

    xTaskCreate(vDisplayTask, vDisplayTaskName, vDisplayTaskStackSize, NULL, vDisplayTaskPriority, NULL);
}

static void writeToDisplay(char *msg)
{
    int16_t x, y;
    uint16_t width, height;

    oledDisplay.getTextBounds(msg, 0, 0, &x, &y, &width, &height);

    oledDisplay.clearDisplay();
    oledDisplay.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);
    oledDisplay.println(msg);
    oledDisplay.display();
}

static void vDisplayTask(void *pvParameters)
{
    program_state_t prev_program_state = kStateNone;

    for (;;)
    {
        if (program_state != prev_program_state)
        {
            prev_program_state = program_state;
            switch (program_state)
            {
            case kStateReadPinpad:
                writeToDisplay((char *)"(Reading) Input the pin on the keypad");
                break;
            case kStateRegisterPinpad:
                writeToDisplay((char *)"(Register) Input the pin on the keypad");
                break;
            case kStateReadNFC:
                writeToDisplay((char *)"(Reading) Tap the card on the NFC reader");
                break;
            case kStateRegisterNFC:
                writeToDisplay((char *)"(Register) Tap the card on the NFC reader");
                break;
            case kStateRegisterSuccess:
                writeToDisplay((char *)"Succesfully registered user");
                break;
            case kStateRegisterFail:
                writeToDisplay((char *)"Registration failed");
                break;
            case kStateReadSuccess:
                writeToDisplay((char *)"Access granted. Successfully recognised user");
                break;
            case kStateReadFail:
                writeToDisplay((char *)"(Reading) Access denied. One method was wrong");
                break;
            default:
                break;
            }
        }
        delay(50);
    }
}