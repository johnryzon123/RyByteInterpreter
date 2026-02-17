// common.c
#include "common.h"

RyValue RyValue::operator/(const RyValue &other) const {
    if (isNumber() && other.isNumber()) {
        return RyValue(asNumber() / other.asNumber());
    }
    return RyValue(to_string() + other.to_string());
}