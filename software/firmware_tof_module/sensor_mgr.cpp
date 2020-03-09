#include "sensor_mgr.h"

#define MAIN_SENSOR_ADDR 42
#define AUX_SENSOR_ADDR 43
#define MAIN_SENSOR_PIN 4
#define AUX_SENSOR_PIN 5


SensorMgr::SensorMgr(RegisterStorage &aRegisterStorage, Stream *errStream) :
    mainSensor(aRegisterStorage, 0, MAIN_SENSOR_ADDR, MAIN_SENSOR_PIN,
        REG_MAIN_ENABLED, REG_MAIN_MIN_RANGE, REG_MAIN_MAX_RANGE,
        REG_MAIN_QUALITY_THRESHOLD, REG_MAIN_PERIOD, REG_MAIN_POLLING,
        REG_MAIN_MCSLR, REG_MAIN_RANGE, REG_MAIN_RAW_RANGE, REG_MAIN_QUALITY,
        errStream),
    auxSensor(aRegisterStorage, 1, AUX_SENSOR_ADDR, AUX_SENSOR_PIN,
        REG_AUX_ENABLED, REG_AUX_MIN_RANGE, REG_AUX_MAX_RANGE,
        REG_AUX_QUALITY_THRESHOLD, REG_AUX_PERIOD, REG_AUX_POLLING,
        REG_AUX_MCSLR, REG_AUX_RANGE, REG_AUX_RAW_RANGE, REG_AUX_QUALITY,
        errStream)
{
    Wire.begin();
    end();
}

void SensorMgr::begin()
{
    mainSensor.begin();
    auxSensor.begin();
}

void SensorMgr::end()
{
    mainSensor.end();
    auxSensor.end();
}

void SensorMgr::update()
{
    mainSensor.update();
    auxSensor.update();
}

uint8_t SensorMgr::status() const
{
    return mainSensor.status() | auxSensor.status();
}

void SensorMgr::resetMainMeasureCount()
{
    mainSensor.resetMeasureCount();
}

void SensorMgr::resetAuxMeasureCount()
{
    auxSensor.resetMeasureCount();
}

void SensorMgr::mainSensorReady()
{
    mainSensor.measurementReady();
}

void SensorMgr::auxSensorReady()
{
    auxSensor.measurementReady();
}
