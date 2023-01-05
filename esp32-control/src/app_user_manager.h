#ifndef APP_USER_MANAGER_H
#define APP_USER_MANAGER_H

#define FORMAT_SPIFFS_IF_FAILED true

#define USER_NAME_SIZE     32
#define USER_PASSWORD_SIZE 6
#define USER_NFC_ID_SIZE   21

#define USER_DB_FILE_PATH "/user_db.txt"
typedef struct User
{
    char name[USER_NAME_SIZE + 1] = "";
    char password[USER_PASSWORD_SIZE + 1] = "";
    char nfc_id[USER_NFC_ID_SIZE + 1] = "";
} User;

void UserDatabaseInit();
void UserDatabaseReset();
void UserDatabaseSaveUser(char *user_name, char *user_password, char *user_nfc_id);
void UserDatabaseDeleteUser(char *user_name);
void UserDatabaseGetUsers();
bool UserDatabaseMatchUser(char *user_name, char *user_password, char *user_nfc_id);

extern User user;

#endif // APP_USER_MANAGER_H