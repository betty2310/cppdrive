#include <validate.h>

#include <cstdio>
#include <cstring>

int validate_ip(const char *ip) {
    int value_1 = -1;
    int value_2 = -1;
    int value_3 = -1;
    int value_4 = -1;
    int count = 0;
    int i = 0;

    while (ip[i] != '\0') {
        if (ip[i] == '.')
            count++;
        i++;
    }

    if (count != 3)
        return INVALID_IP;
    else {
        sscanf(ip, "%d.%d.%d.%d", &value_1, &value_2, &value_3, &value_4);

        if (value_1 < 0 || value_2 < 0 || value_3 < 0 || value_4 < 0 || value_1 > 255 ||
            value_2 > 255 || value_3 > 255 || value_4 > 255)
            return INVALID_IP;
        else
            return 1;
    }
}