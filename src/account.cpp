#include "account.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Account new_account(char *username, char *password, char *is_active) {
    Account acc = (Account) malloc(sizeof(struct account));
    acc->username = (char *) malloc(sizeof(char) * MAX_CHARS);
    acc->password = (char *) malloc(sizeof(char) * MAX_CHARS);
    acc->is_active = (char *) malloc(sizeof(char) * MAX_CHARS);
    strcpy(acc->username, username);
    strcpy(acc->password, password);
    strcpy(acc->is_active, is_active);
    acc->next = NULL;
    return acc;
}

Account add_account(Account account_list, char *username, char *password, char *is_active) {
    Account new_acc = new_account(username, password, is_active);
    if (account_list == NULL) {
        account_list = new_acc;
    } else {
        Account tmp = account_list;
        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = new_acc;
    }
    return account_list;
}

Account read_account(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("Cannot open file %s to read account information !\n", filename);
        return NULL;
    }
    Account account_list = NULL;
    char *username = (char *) malloc(sizeof(char) * MAX_CHARS);
    char *password = (char *) malloc(sizeof(char) * MAX_CHARS);
    char *is_active = (char *) malloc(sizeof(char) * MAX_CHARS);
    while (fscanf(f, "%s %s %s", username, password, is_active) != EOF) {
        account_list = add_account(account_list, username, password, is_active);
    }
    fclose(f);
    return account_list;
}

int process_login(Account account_list, char *username, char *password) {
    Account tmp = account_list;
    while (tmp != NULL) {
        if (strcmp(tmp->username, username) == 0) {
            if (strcmp(tmp->password, password) == 0) {
                if (strcmp(tmp->is_active, "active") == 0) {
                    printf("%s",tmp->is_active);
                    return VALID_CREDENTIALS;   // valid credentials
                } else {
                    return ACCOUNT_NOT_ACTIVE;   // credentials correct but account is not active
                }
            } else {
                return WRONG_PASSWORD;   // username found, but wrong password
            }
        }
        tmp = tmp->next;
    }
    return USERNAME_REQUIRED;   // username not found
}