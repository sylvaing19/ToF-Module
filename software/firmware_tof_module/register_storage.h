#ifndef REGISTER_STORAGE_H
#define REGISTER_STORAGE_H

#include <Arduino.h>

#define REGISTER_SIZE 51
#define EEPROM_AREA_SIZE 32


enum RegisterMap
{
    /* EEPROM area */
    REG_MODEL_NUMBER = 0x00,
    REG_FIRMWARE_VERSION = 0x02,

    REG_ID = 0x03,
    REG_BAUDRATE = 0x04,
    REG_RETURN_DELAY_TIME = 0x05,
    REG_STATUS_RETURN_LEVEL = 0x06,

    REG_MAIN_MIN_RANGE = 0x07,
    REG_MAIN_MAX_RANGE = 0x09,
    REG_MAIN_QUALITY_THRESHOLD = 0x0B,
    REG_MAIN_PERIOD = 0x0D,

    REG_AUX_MIN_RANGE = 0x11,
    REG_AUX_MAX_RANGE = 0x13,
    REG_AUX_QUALITY_THRESHOLD = 0x15,
    REG_AUX_PERIOD = 0x17,

    REG_AUTO_START = 0x1B,
    REG_MAIN_POLLING = 0x1C,
    REG_AUX_POLLING = 0x1D,

    /* RAM area */
    REG_MAIN_ENABLED = 0x20,
    REG_AUX_ENABLED = 0x21,
    REG_WIRING_STATUS = 0x22,

    REG_MAIN_MCSLR = 0x23,
    REG_MAIN_RANGE = 0x24,
    REG_MAIN_RAW_RANGE = 0x26,
    REG_MAIN_QUALITY = 0x28,

    REG_AUX_MCSLR = 0x2A,
    REG_AUX_RANGE = 0x2B,
    REG_AUX_RAW_RANGE = 0x2D,
    REG_AUX_QUALITY = 0x2F,

    REG_INPUT_VOLTAGE = 0x31,
    REG_LOCK = 0x32,
};


class RegisterStorage
{
public:
    RegisterStorage(uint8_t aRangeErrorCode);

    void init();
    void resetEEPROM();

    void read(uint8_t address, uint8_t size, uint8_t* data);
    uint8_t write(uint8_t address, uint8_t size, const uint8_t* data);

    /* Allow to write read-only registers but only in RAM area */
    void writeRAM(uint8_t address, uint8_t size, const uint8_t* data);

    template<class T>
    void read(uint8_t address, T& data)
    {
        read(address, sizeof(T), (uint8_t*)&data);
    }

    template<class T>
    uint8_t write(uint8_t address, const T& data)
    {
        return write(address, sizeof(T), (const uint8_t*)&data);
    }

    template<class T>
    void writeRAM(uint8_t address, const T& data)
    {
        writeRAM(address, sizeof(T), (const uint8_t*)&data);
    }

    bool idChanged() const { return mIdChanged; }
    bool baudrateChanged() const { return mBaudrateChanged; }
    bool returnDelayTimeChanged() const { return mReturnDelayTimeChanged; }
    bool statusReturnLevelChanged() const { return mStatusReturnLevelChanged; }

    uint8_t getId() { mIdChanged = false; return mData[REG_ID]; }
    uint8_t getBaudrate() { mBaudrateChanged = false; return mData[REG_BAUDRATE]; }
    uint8_t getReturnDelayTime() { mReturnDelayTimeChanged = false; return mData[REG_RETURN_DELAY_TIME]; }
    uint8_t getStatusReturnLevel() { mStatusReturnLevelChanged = false; return mData[REG_STATUS_RETURN_LEVEL]; }

private:
    bool checkMagic();

    bool mIdChanged;
    bool mBaudrateChanged;
    bool mReturnDelayTimeChanged;
    bool mStatusReturnLevelChanged;

    uint8_t mData[REGISTER_SIZE];
    static bool writable[REGISTER_SIZE];
    static uint8_t min_range[REGISTER_SIZE];
    static uint8_t max_range[REGISTER_SIZE];
    static uint8_t eeprom_default[EEPROM_AREA_SIZE];

    const uint8_t mRangeErrorCode;
};


#endif
