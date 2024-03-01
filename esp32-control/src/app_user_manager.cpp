#include "Arduino.h"

#include "FS.h"
#include "SPIFFS.h"

#include "app_user_manager.h"
//#include "app_ble.h"

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

bool UserDatabaseSaveUser(char *user_password, char *user_nfc_id)
{
    File file = SPIFFS.open(USER_DB_FILE_PATH, FILE_READ);
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        LOG_MSG(line.c_str());
        char *check_user_password = strstr(line.c_str(), user_password);
        char *check_user_nfc_id = strstr(line.c_str(), user_nfc_id);

        if (check_user_password != NULL && check_user_nfc_id != NULL)
        {
            LOG_MSG("Username already found, try another!\n");
            //BleServerSend((uint8_t *)"AT+ADDUSER=DUPLICATE\r\n", strlen("AT+ADDUSER=DUPLICATE\r\n"));
            return false;
        }
    }
    file.close();

    file = SPIFFS.open(USER_DB_FILE_PATH);
    File new_file = SPIFFS.open("/user_db_new.txt", FILE_WRITE);
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        new_file.printf("%s", line.c_str());

    }
    file.close();
    new_file.printf("%s,%s\n", user_password, user_nfc_id);
    new_file.close();

    SPIFFS.remove(USER_DB_FILE_PATH);
    SPIFFS.rename("/user_db_new.txt", USER_DB_FILE_PATH);

    LOG_MSG("Added: %s, %s\n", user_password, user_nfc_id);
    return true;
}

// void UserDatabaseGetUsers()
// {
//     char message[512] = "AT+GETUSERS=";

//     File file = SPIFFS.open(USER_DB_FILE_PATH);
//     while (file.available())
//     {
//         String line = file.readStringUntil('\n');
//         LOG_MSG("%s\n", line.c_str());

//         strcat(message, strtok((char *)line.c_str(), ","));
//         strcat(message, ",");
//     }
//     file.close();

//     message[strlen(message)] = '\0';
//     strcat(message, "\r\n");
//     BleServerSend((uint8_t *)message, strlen(message));
// }

bool UserDatabaseMatchUser(char *user_password, char *user_nfc_id)
{
    File file = SPIFFS.open(USER_DB_FILE_PATH, FILE_READ);
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        LOG_MSG(line.c_str());
        char *check_user_password = strstr(line.c_str(), user_password);
        char *check_user_nfc_id = strstr(line.c_str(), user_nfc_id);

        if (check_user_password != NULL && check_user_nfc_id != NULL)
        {
            LOG_MSG("Found valid user in db\n");
            return true;
        }
    }

    file.close();
    return false;
}

// void UserDatabaseDeleteUser(char *user_name)
// {
//     File file = SPIFFS.open(USER_DB_FILE_PATH);
//     File new_file = SPIFFS.open("/user_db_new.txt", FILE_WRITE);
//     while (file.available())
//     {
//         String line = file.readStringUntil('\n');
//         char *check_user_name = strstr(line.c_str(), user_name);
//         if (check_user_name != NULL)
//         {
//             continue;
//         }
//         else
//         {
//             new_file.printf("%s", line.c_str());
//         }
//     }
//     file.close();
//     new_file.close();

//     SPIFFS.remove(USER_DB_FILE_PATH);
//     SPIFFS.rename("/user_db_new.txt", USER_DB_FILE_PATH);
// }
