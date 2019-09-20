#ifndef TOF_MODULE_H
#define TOF_MODULE_H

#include <stdint.h>
#include "OneWireMInterface.h"


typedef int32_t TofValue;
enum TofValueEnum {
    SENSOR_DEAD         = 0x00,
    SENSOR_NOT_UPDATED  = 0x01,
    OBSTACLE_TOO_CLOSE  = 0x02,
    NO_OBSTACLE         = 0x03
};


typedef uint8_t TofStatus;
enum TofStatusEnum {
    TOF_STATUS_OK                   = 0,
    TOF_STATUS_MAIN_SENSOR_ERROR    = 1,
    TOF_STATUS_AUX_SENSOR_ERROR     = 2,
    TOF_STATUS_INPUT_VOLTAGE_ERROR  = 4,
    TOF_STATUS_RANGE_ERROR          = 8,
    TOF_STATUS_CHECKSUM_ERROR       = 16,
    TOF_STATUS_INSTRUCTION_ERROR    = 64
};


enum TofRegisterMap
{
    /* EEPROM area */
    TOF_MODEL_NUMBER = 0x00,
    TOF_FIRMWARE_VERSION = 0x02,

    TOF_ID = 0x03,
    TOF_BAUDRATE = 0x04,
    TOF_RETURN_DELAY_TIME = 0x05,
    TOF_STATUS_RETURN_LEVEL = 0x06,

    TOF_MAIN_MIN_RANGE = 0x07,
    TOF_MAIN_MAX_RANGE = 0x09,
    TOF_MAIN_QUALITY_THRESHOLD = 0x0B,
    TOF_MAIN_PERIOD = 0x0D,

    TOF_AUX_MIN_RANGE = 0x11,
    TOF_AUX_MAX_RANGE = 0x13,
    TOF_AUX_QUALITY_THRESHOLD = 0x15,
    TOF_AUX_PERIOD = 0x17,

    TOF_AUTO_START = 0x1B,
    TOF_MAIN_POLLING = 0x1C,
    TOF_AUX_POLLING = 0x1D,

    /* RAM area */
    TOF_MAIN_ENABLED = 0x20,
    TOF_AUX_ENABLED = 0x21,
    TOF_WIRING_STATUS = 0x22,

    TOF_MAIN_MCSLR = 0x23,
    TOF_MAIN_RANGE = 0x24,
    TOF_MAIN_RAW_RANGE = 0x26,
    TOF_MAIN_QUALITY = 0x28,

    TOF_AUX_MCSLR = 0x2A,
    TOF_AUX_RANGE = 0x2B,
    TOF_AUX_RAW_RANGE = 0x2D,
    TOF_AUX_QUALITY = 0x2F,

    TOF_INPUT_VOLTAGE = 0x31,
    TOF_LOCK = 0x32,
};


class ToF_module
{
public:
    ToF_module(OneWireMInterface &aInterface, uint8_t aId);

    OneWireStatus init();

    TofStatus status() const { return mStatus; }
    bool error() const { return mStatus != TOF_STATUS_OK; }
    bool internalError() const;
    bool commandError() const;

    uint8_t id() const { return mID; }
    int setId(uint8_t newId);
    uint16_t model();
    uint8_t firmware();
    int communicationSpeed(uint32_t aBaudrate);
    int statusReturnLevel(uint8_t aSRL);
    uint8_t statusReturnLevel() const { return mStatusReturnLevel; }
    bool mainWired() const { return mMainWired; }
    bool auxWired() const { return mAuxWired; }
    int isPolling(bool &main, bool &aux);
    int setPolling(bool main, bool aux);
    float inputVoltage();

    OneWireStatus enable(bool e = true);
    OneWireStatus auxEnable(bool e = true);
    uint8_t available();
    TofValue readRange();
    uint8_t auxAvailable();
    TofValue auxReadRange();

    template<class T>
    inline OneWireStatus read(uint8_t aAddress, T& aData)
    {
        return mInterface.read<T>(mID, aAddress, aData, mStatusReturnLevel, &mStatus);
    }

    template<class T>
    inline OneWireStatus write(uint8_t aAddress, const T& aData)
    {
        return mInterface.write<T>(mID, aAddress, aData, mStatusReturnLevel, &mStatus);
    }

    OneWireStatus ping()
    {
        return mInterface.ping(mID, &mStatus);
    }

    OneWireStatus softReset()
    {
        return mInterface.softReset(mID, mStatusReturnLevel, &mStatus);
    }

    OneWireStatus factoryReset()
    {
        return mInterface.factoryReset(mID, mStatusReturnLevel, &mStatus);
    }

private:
    OneWireMInterface &mInterface;
    uint8_t mStatusReturnLevel;
    uint8_t mID;
    TofStatus mStatus;
    bool mMainWired;
    bool mAuxWired;
};


#endif
