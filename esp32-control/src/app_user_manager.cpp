#include "Arduino.h"

#include "FS.h"
#include "SPIFFS.h"

#include "app_user_manager.h"
#include "app_ble.h"

#include "utils.h"

User user;

void UserDatabaseInit()
{
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {
        LOG_MSG("SPIFFS Mount Failed");
        for(;;);
    }

    if (!SPIFFS.exists(USER_DB_FILE_PATH))
        UserDatabaseReset();
}

void UserDatabaseReset()
{
    File file = SPIFFS.open(USER_DB_FILE_PATH, FILE_WRITE);
    file.close();
}

void UserDatabaseSaveUser(char *user_name, char *user_password, char *user_nfc_id)
{
    File file = SPIFFS.open(USER_DB_FILE_PATH);
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        char *check_user_name = strstr(line.c_str(), user_name);
        if (check_user_name != NULL)
        {
            LOG_MSG("Username already found, try another!");
            BleServerSend((uint8_t *)"AT+ADDUSER=DUPLICATE\r\n", strlen("AT+ADDUSER=DUPLICATE\r\n"));
            return;
        }
    }
    file.close();

    file = SPIFFS.open(USER_DB_FILE_PATH, FILE_APPEND);
    file.printf("%s,%s,%s\n", user_name, user_password, user_nfc_id);
    file.close();

    BleServerSend((uint8_t *)"AT+ADDUSER=OK\r\n", strlen("AT+ADDUSER=OK\r\n"));
    LOG_MSG("Added: %s, %s, %s\n", user_name, user_password, user_nfc_id);
}

void UserDatabaseGetUsers()
{
    char message[512] = "AT+GETUSERS=";

    File file = SPIFFS.open(USER_DB_FILE_PATH);
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        LOG_MSG("%s", line.c_str());

        strcat(message, strtok((char *)line.c_str(), ","));
        strcat(message, ",");
    }
    file.close();

    message[strlen(message)] = '\0';
    strcat(message, "\r\n");
    BleServerSend((uint8_t *)message, strlen(message));
}

bool UserDatabaseMatchUser(char *user_name, char *user_password, char *user_nfc_id)
{
    File file = SPIFFS.open(USER_DB_FILE_PATH);
    while (file.available())
    {
        String line = file.readStringUntil('\n');

        char *check_user_name = strstr(line.c_str(), user_name);
        char *check_user_password = strstr(line.c_str(), user_password);
        char *check_user_nfc_id = strstr(line.c_str(), user_nfc_id);

        if (check_user_name != NULL && check_user_password != NULL && check_user_nfc_id != NULL)
        {
            LOG_MSG("Found user: %s\n", user_name);
            return true;
        }
    }

    file.close();
    return false;
}

void UserDatabaseDeleteUser(char *user_name)
{
    File file = SPIFFS.open(USER_DB_FILE_PATH);
    File new_file = SPIFFS.open("/user_db_new.txt", FILE_WRITE);
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        char *check_user_name = strstr(line.c_str(), user_name);
        if (check_user_name != NULL)
        {
            continue;
        }
        else
        {
            new_file.printf("%s", line.c_str());
        }
    }
    file.close();
    new_file.close();

    SPIFFS.remove(USER_DB_FILE_PATH);
    SPIFFS.rename("/user_db_new.txt", USER_DB_FILE_PATH);
}
