#ifndef APP_PINPAD_H
#define APP_PINPAD_H

#define ROW_NUM    4
#define COLUMN_NUM 3
#define PIN_LENGTH 6

#define vPinpadTaskStackSize 2048
#define vPinpadTaskPriority  3
#define vPinpadTaskName      "Pinpad"

void PinpadInit();

#endif // APP_PINPAD_H