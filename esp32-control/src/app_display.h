#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#define SCREEN_WIDTH       128 // OLED display width, in pixels
#define SCREEN_HEIGHT      64 // OLED display height, in pixels
#define DIPLAY_I2C_ADDRESS 0x3C

#define vDisplayTaskStackSize 2048
#define vDisplayTaskPriority  3
#define vDisplayTaskName      "Display"

void DisplayInit();

#endif // APP_DISPLAY_H