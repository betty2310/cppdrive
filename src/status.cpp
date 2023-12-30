#include "status.h"

#include <string>

std::string status_str(Status code) {
    switch (code) {
        case USERNAME_EXIST:
            return "Username is existed!";
            break;
        // success message
        case USER_NOT_FOUND:
            return "User Not Found!!";
            break;
        case USER_IS_BLOCKED:
            return "User is Blocked!!";
            break;
        case BLOCKED_USER:
            return "User has been Blocked!!";
            break;
        case PASSWORD_INVALID:
            return "Your input password invalid!!";
            break;
        // error message
        case LOGIN_SUCCESS:
            return "Login Successfully!";
            break;
        // error message
        case LOGIN_FAIL:
            return "Wrong username or password!";
            break;
        case USER_IS_ONLINE:
            return "The User Was In Online!!";
            break;
        case ACCOUNT_IS_EXIST:
            return "The Account was existed!!";
            break;
        // invalid message
        case REGISTER_SUCCESS:
            return "Register Successfully!!";
            break;
        case LOGOUT_SUCCESS:
            return "Logout SuccessFully!!";
            break;
        case FILE_NOT_FOUND:
            return "User Not Found!!";
            break;
        case COMMAND_INVALID:
            return "Command was invalid!!";
            break;
        case USERNAME_OR_PASSWORD_INVALID:
            return "Username or Password invalid!!";
            break;
        // server error message
        case SERVER_ERROR:
            return "Something error!!";
            break;
        default:
            break;
    }
    return "";
}