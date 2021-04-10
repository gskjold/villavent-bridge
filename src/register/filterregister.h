#ifndef _FILTERREGISTER_H
#define _FILTERREGISTER_H

#include "register.h"

#define REG_FILTER_PER 601
#define REG_FILTER_DAYS 602

class FilterRegister : public Register {
    public:
        FilterRegister() : Register("filter", 600, 2, 60000, false) {
            addRegister(REG_FILTER_PER, "filter/period");
            addRegister(REG_FILTER_DAYS, "filter/age");
        };

        boolean setFormattedValue(int address, String &value);
};

#endif
