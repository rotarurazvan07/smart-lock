#include <Arduino.h>

#include "board.h"

#include "app_ble.h"
#include "app_nfc.h"
#include "app_pinpad.h"
#include "app_user_manager.h"
#include "app_vision.h"
#include "app_display.h"

#include "utils.h"

void setup()
{
    Serial.begin(9600);

    program_state = kStateReadPinpad;

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(17, OUTPUT);

    // BleServerInit();

    UserDatabaseInit();

    NFCInit();

    PinpadInit();

   // VisionInit();

    DisplayInit();
}

void loop() {}