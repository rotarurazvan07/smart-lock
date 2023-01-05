#include <Arduino.h>
#include <Keypad.h>

#include "board.h"

#include "app_pinpad.h"
#include "app_nfc.h"
#include "app_user_manager.h"
#include "app_display.h"

#include "utils.h"

static char keys[ROW_NUM][COLUMN_NUM] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

static byte rowPins[ROW_NUM] = {R1_PIN, R2_PIN, R3_PIN, R4_PIN};
static byte colPins[COLUMN_NUM] = {C1_PIN, C2_PIN, C3_PIN};

static Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROW_NUM, COLUMN_NUM);

static void vPinpadTask(void *pvParameters);

void PinpadInit()
{
    xTaskCreate(vPinpadTask, vPinpadTaskName, vPinpadTaskStackSize, NULL, vPinpadTaskPriority, NULL);
}

static void vPinpadTask(void *pvParameters)
{
    int temp_key_sz = 0;
    char temp_key[PIN_LENGTH + 1] = "";

    for (;;)
    {
        if (program_state == kStateReadPinpad)
        {
            char key = keypad.getKey();

            if (key)
            {
                temp_key[temp_key_sz++] = key;
                play_tone(kToneKeyPress);
                if (temp_key_sz >= PIN_LENGTH)
                {
                    LOG_MSG("Read pin from keypad: %s\nMoving to nfc reading\n", temp_key);

                    strcpy(user.password, temp_key);

                    memset((char *)temp_key, 0x00, PIN_LENGTH + 1);
                    temp_key_sz = 0;

                    program_state = kStateReadNFC;
                    play_tone(kToneSuccess);
                }
            }
        }
        delay(50);
    }
}