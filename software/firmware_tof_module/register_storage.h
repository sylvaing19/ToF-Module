#ifndef REGISTER_STORAGE_H
#define REGISTER_STORAGE_H


#include <Arduino.h>


class RegisterStorage
{
public:
    RegisterStorage(uint8_t aRangeErrorCode);

    void init();
    void resetEEPROM();

};


#endif
