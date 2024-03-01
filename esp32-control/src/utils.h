#ifndef UTILS_H
#define UTILS_H

#define ENABLE_DEBUG 1
#if ENABLE_DEBUG
#define LOG_MSG Serial.printf
#else
#define LOG_MSG(...)
#endif

typedef enum _program_state
{
    kStateReadPinpad,
    kStateReadNFC,

    kStateRegisterNFC,
    kStateRegisterPinpad,

    kStateRegisterSuccess,
    kStateRegisterFail,
    kStateReadSuccess,
    kStateReadFail,

    kStateNone
} program_state_t;

typedef enum _buzzer_tone
{
    kToneKeyPress,
    kToneSuccess,
    kToneFail
} buzzer_tone_t;

void byte_array_to_string(byte array[], unsigned int len, char buffer[]);
void reset_temp_user();
void play_tone(buzzer_tone_t buzzer_tone);

extern program_state_t program_state;

#endif // UTILS_H