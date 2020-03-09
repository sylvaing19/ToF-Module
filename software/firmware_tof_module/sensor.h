#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <ToF_sensor.h>
#include "register_storage.h"


class Sensor
{
public:
    Sensor(RegisterStorage &aRegisterStorage, uint8_t aIndex, uint8_t aAddress,
        uint8_t aResetPin, uint8_t aRegEnabled, uint8_t aRegMinRange,
        uint8_t aRegMaxRange, uint8_t aRegQualityThreshold, uint8_t aRegPeriod,
        uint8_t aRegPolling, uint8_t aRegMeasureCount, uint8_t aRegRange,
        uint8_t aRegRawRange, uint8_t aRegQuality, Stream *errStream = nullptr);

    void begin();
    void end();
    void update();

    void resetMeasureCount();
    uint8_t status() const { return mStatus; }
    bool isWired() const { return mWired; }
    void measurementReady() { mMeasurementReady = true; }

private:
    RegisterStorage &mRegisters;
    ToF_longRange mSensor;
    uint8_t mStatus;
    bool mWired;
    uint8_t mMeasureCount;
    uint32_t mLastMeasureTime;
    volatile bool mMeasurementReady;
    bool mPolling;

    const uint8_t mIndex; // Sensor's index for r/w in bytes where each bit is reserved to a different sensor
    const uint8_t mRegEnabled;
    const uint8_t mRegMinRange;
    const uint8_t mRegMaxRange;
    const uint8_t mRegQualityThreshold;
    const uint8_t mRegPeriod;
    const uint8_t mRegPolling;
    const uint8_t mRegMeasureCount;
    const uint8_t mRegRange;
    const uint8_t mRegRawRange;
    const uint8_t mRegQuality;

    Stream *mErrorStream;
};


#endif
