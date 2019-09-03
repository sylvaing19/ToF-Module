#include "register_storage.h"
#include "version.h"
#include <EEPROM.h>

/* Magic numbers to check if EEPROM was initialized */
#define MAGIC_ADDR 60
#define MAGIC_DATA_0 35
#define MAGIC_DATA_1 78
#define MAGIC_DATA_2 149
#define MAGIC_DATA_3 39


RegisterStorage::RegisterStorage(uint8_t aRangeErrorCode) :
    mRangeErrorCode(aRangeErrorCode)
{
    mIdChanged = false;
    mBaudrateChanged = false;
    mReturnDelayTimeChanged = false;
    mStatusReturnLevelChanged = false;
}

void RegisterStorage::init()
{
    if (!checkMagic()) {
        resetEEPROM();
    }
    for (size_t i = 0; i < EEPROM_AREA_SIZE; i++) {
        mData[i] = EEPROM.read(i);
    }
    for (size_t i = EEPROM_AREA_SIZE; i < REGISTER_SIZE; i++) {
        mData[i] = 0;
    }
    mData[REG_MAIN_ENABLED] = mData[REG_AUTO_START];
    mData[REG_AUX_ENABLED] = mData[REG_AUTO_START];
}

void RegisterStorage::resetEEPROM()
{
    for (size_t i = 0; i < EEPROM_AREA_SIZE; i++) {
        EEPROM.write(i, eeprom_default[i]);
    }
}

void RegisterStorage::read(uint8_t address, uint8_t size, uint8_t * data)
{
    size_t bytes_count = min((size_t)address + (size_t)size, REGISTER_SIZE) - (size_t)address;

    for (size_t i = 0; i < bytes_count; i++) {
        data[i] = mData[address + i];
    }
    for (size_t i = bytes_count; i < size; i++) {
        data[i] = 0;
    }
}

uint8_t RegisterStorage::write(uint8_t address, uint8_t size, const uint8_t * data)
{
    if (mData[REG_LOCK] == 1) {
        return mRangeErrorCode;
    }

    size_t end_addr = min((size_t)address + (size_t)size, REGISTER_SIZE);
    uint8_t ret = 0;

    for (size_t i = address; i < end_addr; i++) {
        if (!writable[i] || 
                data[i - address] < min_range[i] ||
                data[i - address] > max_range[i]) {
            ret = mRangeErrorCode;
            continue;
        }
        mData[i] = data[i - address];
        if (i < EEPROM_AREA_SIZE) {
            EEPROM.write(i, mData[i]);
            if (i == REG_ID) {
                mIdChanged = true;
            }
            else if (i == REG_BAUDRATE) {
                mBaudrateChanged = true;
            }
            else if (i == REG_RETURN_DELAY_TIME) {
                mReturnDelayTimeChanged = true;
            }
            else if (i == REG_STATUS_RETURN_LEVEL) {
                mStatusReturnLevelChanged = true;
            }
        }
    }
    if (end_addr - address < size) {
        ret = mRangeErrorCode;
    }
    return ret;
}

void RegisterStorage::writeRAM(uint8_t address, uint8_t size, const uint8_t * data)
{
    if (address < EEPROM_AREA_SIZE) {
        return;
    }
    size_t end_addr = min((size_t)address + (size_t)size, REGISTER_SIZE);
    for (size_t i = address; i < end_addr; i++) {
        if (data[i - address] >= min_range[i] && data[i - address] <= max_range[i]) {
            mData[i] = data[i - address];
        }
    }
}

bool RegisterStorage::checkMagic()
{
    return EEPROM.read(MAGIC_ADDR) == MAGIC_DATA_0 &&
        EEPROM.read(MAGIC_ADDR + 1) == MAGIC_DATA_1 &&
        EEPROM.read(MAGIC_ADDR + 2) == MAGIC_DATA_2 &&
        EEPROM.read(MAGIC_ADDR + 3) == MAGIC_DATA_3;
}


bool RegisterStorage::writable[REGISTER_SIZE] = {
    /* EEPROM area */
    false, // REG_MODEL_NUMBER = 0x00
    false,
    false, // REG_FIRMWARE_VERSION = 0x02

    true, // REG_ID = 0x03
    true, // REG_BAUDRATE = 0x04
    true, // REG_RETURN_DELAY_TIME = 0x05
    true, // REG_STATUS_RETURN_LEVEL = 0x06

    true, // REG_MAIN_MIN_RANGE = 0x07
    true,
    true, // REG_MAIN_MAX_RANGE = 0x09
    true,
    true, // REG_MAIN_QUALITY_THRESHOLD = 0x0B
    true,
    true, // REG_MAIN_PERIOD = 0x0D
    true,
    true,
    true,

    true, // REG_AUX_MIN_RANGE = 0x11
    true,
    true, // REG_AUX_MAX_RANGE = 0x13
    true,
    true, // REG_AUX_QUALITY_THRESHOLD = 0x15
    true,
    true, // REG_AUX_PERIOD = 0x17
    true,
    true,
    true,

    true, // REG_AUTO_START = 0x1B

    false, // Reserved (0x1C - 0x1F)
    false,
    false,
    false,

    /* RAM area */
    true, // REG_MAIN_ENABLED = 0x20
    true, // REG_AUX_ENABLED = 0x21
    false, // REG_WIRING_STATUS = 0x22

    false, // REG_MAIN_MCSLR = 0x23
    false, // REG_MAIN_RANGE = 0x24
    false,
    false, // REG_MAIN_RAW_RANGE = 0x26
    false,
    false, // REG_MAIN_QUALITY = 0x28
    false,

    false, // REG_AUX_MCSLR = 0x2A
    false, // REG_AUX_RANGE = 0x2B
    false,
    false, // REG_AUX_RAW_RANGE = 0x2D
    false,
    false, // REG_AUX_QUALITY = 0x2F
    false,

    true, // REG_LOCK = 0x31
};

uint8_t RegisterStorage::min_range[REGISTER_SIZE] = {
    /* EEPROM area */
    0, // REG_MODEL_NUMBER = 0x00
    0,
    0, // REG_FIRMWARE_VERSION = 0x02

    0, // REG_ID = 0x03
    1, // REG_BAUDRATE = 0x04
    0, // REG_RETURN_DELAY_TIME = 0x05
    0, // REG_STATUS_RETURN_LEVEL = 0x06

    4, // REG_MAIN_MIN_RANGE = 0x07
    0,
    4, // REG_MAIN_MAX_RANGE = 0x09
    0,
    0, // REG_MAIN_QUALITY_THRESHOLD = 0x0B
    0,
    0, // REG_MAIN_PERIOD = 0x0D
    0,
    0,
    0,

    4, // REG_AUX_MIN_RANGE = 0x11
    0,
    4, // REG_AUX_MAX_RANGE = 0x13
    0,
    0, // REG_AUX_QUALITY_THRESHOLD = 0x15
    0,
    0, // REG_AUX_PERIOD = 0x17
    0,
    0,
    0,

    0, // REG_AUTO_START = 0x1B

    0, // Reserved (0x1C - 0x1F)
    0,
    0,
    0,

    /* RAM area */
    0, // REG_MAIN_ENABLED = 0x20
    0, // REG_AUX_ENABLED = 0x21
    0, // REG_WIRING_STATUS = 0x22

    0, // REG_MAIN_MCSLR = 0x23
    0, // REG_MAIN_RANGE = 0x24
    0,
    0, // REG_MAIN_RAW_RANGE = 0x26
    0,
    0, // REG_MAIN_QUALITY = 0x28
    0,

    0, // REG_AUX_MCSLR = 0x2A
    0, // REG_AUX_RANGE = 0x2B
    0,
    0, // REG_AUX_RAW_RANGE = 0x2D
    0,
    0, // REG_AUX_QUALITY = 0x2F
    0,

    0, // REG_LOCK = 0x31
};

uint8_t RegisterStorage::max_range[REGISTER_SIZE] = {
    /* EEPROM area */
    255, // REG_MODEL_NUMBER = 0x00
    255,
    255, // REG_FIRMWARE_VERSION = 0x02

    253, // REG_ID = 0x03
    207, // REG_BAUDRATE = 0x04
    254, // REG_RETURN_DELAY_TIME = 0x05
    2, // REG_STATUS_RETURN_LEVEL = 0x06

    255, // REG_MAIN_MIN_RANGE = 0x07
    255,
    255, // REG_MAIN_MAX_RANGE = 0x09
    255,
    255, // REG_MAIN_QUALITY_THRESHOLD = 0x0B
    255,
    255, // REG_MAIN_PERIOD = 0x0D
    255,
    255,
    255,

    255, // REG_AUX_MIN_RANGE = 0x11
    255,
    255, // REG_AUX_MAX_RANGE = 0x13
    255,
    255, // REG_AUX_QUALITY_THRESHOLD = 0x15
    255,
    255, // REG_AUX_PERIOD = 0x17
    255,
    255,
    255,

    1, // REG_AUTO_START = 0x1B

    255, // Reserved (0x1C - 0x1F)
    255,
    255,
    255,

    /* RAM area */
    1, // REG_MAIN_ENABLED = 0x20
    1, // REG_AUX_ENABLED = 0x21
    3, // REG_WIRING_STATUS = 0x22

    254, // REG_MAIN_MCSLR = 0x23
    255, // REG_MAIN_RANGE = 0x24
    255,
    255, // REG_MAIN_RAW_RANGE = 0x26
    255,
    255, // REG_MAIN_QUALITY = 0x28
    255,

    254, // REG_AUX_MCSLR = 0x2A
    255, // REG_AUX_RANGE = 0x2B
    255,
    255, // REG_AUX_RAW_RANGE = 0x2D
    255,
    255, // REG_AUX_QUALITY = 0x2F
    255,

    1, // REG_LOCK = 0x31
};

uint8_t RegisterStorage::eeprom_default[EEPROM_AREA_SIZE] = {
    /* EEPROM area */
    MODEL_NB_LW, // REG_MODEL_NUMBER = 0x00
    MODEL_NB_HW,
    FIRMWARE_VERSION, // REG_FIRMWARE_VERSION = 0x02

    1, // REG_ID = 0x03
    4, // REG_BAUDRATE = 0x04
    250, // REG_RETURN_DELAY_TIME = 0x05
    2, // REG_STATUS_RETURN_LEVEL = 0x06

    0x1E, // REG_MAIN_MIN_RANGE = 0x07
    0x00,
    0xBC, // REG_MAIN_MAX_RANGE = 0x09
    0x02,
    0xFA, // REG_MAIN_QUALITY_THRESHOLD = 0x0B
    0x00,
    0x00, // REG_MAIN_PERIOD = 0x0D
    0x00,
    0x00,
    0x00,

    0x1E, // REG_AUX_MIN_RANGE = 0x11
    0x00,
    0xBC, // REG_AUX_MAX_RANGE = 0x13
    0x02,
    0xFA, // REG_AUX_QUALITY_THRESHOLD = 0x15
    0x00,
    0x00, // REG_AUX_PERIOD = 0x17
    0x00,
    0x00,
    0x00,

    1, // REG_AUTO_START = 0x1B

    0, // Reserved (0x1C - 0x1F)
    0,
    0,
    0,
};

