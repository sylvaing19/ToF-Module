#include "sensor_mgr.h"

#define MAIN_SENSOR_ADDR 42
#define AUX_SENSOR_ADDR 43

SensorMgr::SensorMgr(RegisterStorage & aRegisterStorage, uint8_t aMainSensorErrorCode, uint8_t aAuxSensorErrorCode) :
    mRegisters(aRegisterStorage)
{
    mainSensorWired = false;
    auxSensorWired = false;
    mainMeasureCount = 0;
    auxMeasureCount = 0;
}

void SensorMgr::begin()
{
}

void SensorMgr::end()
{
}

void SensorMgr::update()
{
}

uint8_t SensorMgr::status() const
{
    return uint8_t();
}
