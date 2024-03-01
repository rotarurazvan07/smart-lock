#include "Arduino.h"

#include "board.h"

#include "app_nfc.h"
#include "app_pinpad.h"
#include "app_user_manager.h"

#include "utils.h"

program_state_t program_state;

void byte_array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i * 2 + 0] = nib1 < 0xA ? '0' + nib1 : 'A' + nib1 - 0xA;
        buffer[i * 2 + 1] = nib2 < 0xA ? '0' + nib2 : 'A' + nib2 - 0xA;
    }
    buffer[len * 2] = '\0';
}

void reset_temp_user()
{
    memset((char *)user.password, 0x00, USER_PASSWORD_SIZE);
    memset((char *)user.nfc_id, 0x00, USER_NFC_ID_SIZE);
}

void play_tone(buzzer_tone_t buzzer_tone)
{
    switch (buzzer_tone)
    {
    case kToneKeyPress:
        tone(BUZZER_PIN, 800, 50);
        break;
    case kToneSuccess:
        tone(BUZZER_PIN, 700, 100);
        delay(200);
        noTone(BUZZER_PIN);
        tone(BUZZER_PIN, 800, 100);
        delay(200);
        noTone(BUZZER_PIN);
        tone(BUZZER_PIN, 1000, 200);
        break;
    case kToneFail:
        tone(BUZZER_PIN, 1000, 100);
        delay(200);
        noTone(BUZZER_PIN);
        tone(BUZZER_PIN, 600, 100);
        delay(200);
        noTone(BUZZER_PIN);
        tone(BUZZER_PIN, 400, 200);
        break;
    default:
        break;
    }
}
