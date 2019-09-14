#include "sensor.h"

/* The sensor will be set as faulty if it cannot give a valid reading in
 * more than TIMEOUT (ms) with:
 * PERIOD = inter-measurement period (may be zero), in ms
 * TIMEOUT = max(MINIMAL_FAULT_TIMER, PERIOD_FAULT_TIMER * PERIOD)
 */
#define PERIOD_FAULT_TIMER 3
#define MINIMAL_FAULT_TIMER 100 // ms

Sensor::Sensor(RegisterStorage & aRegisterStorage, uint8_t aIndex,
    uint8_t aAddress, uint8_t aResetPin, uint8_t aRegEnabled,
    uint8_t aRegMinRange, uint8_t aRegMaxRange, uint8_t aRegQualityThreshold,
    uint8_t aRegPeriod, uint8_t aRegPolling, uint8_t aRegMeasureCount,
    uint8_t aRegRange, uint8_t aRegRawRange, uint8_t aRegQuality) :
    mRegisters(aRegisterStorage),
    mSensor(aAddress, aResetPin),
    mIndex(aIndex),
    mRegEnabled(aRegEnabled),
    mRegMinRange(aRegMinRange),
    mRegMaxRange(aRegMaxRange),
    mRegQualityThreshold(aRegQualityThreshold),
    mRegPeriod(aRegPeriod),
    mRegPolling(aRegPolling),
    mRegMeasureCount(aRegMeasureCount),
    mRegRange(aRegRange),
    mRegRawRange(aRegRawRange),
    mRegQuality(aRegQuality)
{
    mStatus = 0;
    end();
}

void Sensor::begin()
{
    mStatus = 0;
    if (mSensor.powerON(false) == EXIT_SUCCESS) {
        mWired = true;
        uint8_t wiringStatus;
        mRegisters.read(REG_WIRING_STATUS, wiringStatus);
        wiringStatus |= (1 << mIndex);
        mRegisters.writeRAM(REG_WIRING_STATUS, wiringStatus);
    }
    uint8_t auto_start;
    mRegisters.read(REG_AUTO_START, auto_start);
    mRegisters.writeRAM(mRegEnabled, auto_start);
    mRegisters.read(mRegPolling, mPolling);
}

void Sensor::end()
{
    mSensor.standby();
    mWired = false;
    mMeasureCount = 0;
    mLastMeasureTime = 0;
    mMeasurementReady = false;
    mPolling = false;
    mRegisters.writeRAM(mRegEnabled, (uint8_t)0);
    mRegisters.writeRAM(mRegMeasureCount, (uint8_t)0);
    mRegisters.writeRAM(mRegRange, (uint16_t)0);
    mRegisters.writeRAM(mRegRawRange, (uint16_t)0);
    mRegisters.writeRAM(mRegQuality, (uint16_t)0);
    uint8_t wiringStatus;
    mRegisters.read(REG_WIRING_STATUS, wiringStatus);
    wiringStatus &= ~(1 << mIndex);
    mRegisters.writeRAM(REG_WIRING_STATUS, wiringStatus);
}

void Sensor::update()
{
    if (!mWired) {
        return;
    }

    bool enabled;
    uint32_t period;
    uint32_t now = millis();
    mRegisters.read(mRegEnabled, enabled);
    mRegisters.read(mRegPeriod, period);

    if (mSensor.measurementStarted()) {
        if (!enabled) {
            mSensor.stopMeasurement();
        }
    }
    else if (enabled) {
        mSensor.startMeasurement(period);
        mMeasurementReady = false;
        mLastMeasureTime = now;
    }

    if (!mSensor.measurementStarted()) {
        return;
    }

    if (now - mLastMeasureTime > 
            max(PERIOD_FAULT_TIMER * period, MINIMAL_FAULT_TIMER)) {
        mStatus = (1 << mIndex);
        end();
        return;
    }

    if (!mPolling && !mMeasurementReady) {
        return;
    }
    mMeasurementReady = false;

    uint16_t min_range;
    uint16_t max_range;
    uint16_t quality_threshold;
    mRegisters.read(mRegMinRange, min_range);
    mRegisters.read(mRegMaxRange, max_range);
    mRegisters.read(mRegQualityThreshold, quality_threshold);

    mSensor.setRange(min_range, max_range);
    mSensor.setQualityThreshold(quality_threshold);

    SensorValue range = 0;
    uint16_t rawRange;
    uint16_t quality;
    int ret = mSensor.getFullMeasure(range, rawRange, quality);
    if (ret == EXIT_FAILURE) {
        return;
    }
    if (mMeasureCount < 254) {
        mMeasureCount++;
    }
    mLastMeasureTime = now;
    mRegisters.writeRAM(mRegMeasureCount, mMeasureCount);
    mRegisters.writeRAM(mRegRange, (uint16_t)range);
    mRegisters.writeRAM(mRegRawRange, rawRange);
    mRegisters.writeRAM(mRegQuality, quality);
}

void Sensor::resetMeasureCount()
{
    mMeasureCount = 0;
    mRegisters.writeRAM(mRegMeasureCount, mMeasureCount);
}
