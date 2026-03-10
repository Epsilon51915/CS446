#include "modify_value.h"

void modify_value_up(int *value) {
    *value = *value + 1;
}

void modify_value_down(int *value) {
    *value = *value - 1;
}
