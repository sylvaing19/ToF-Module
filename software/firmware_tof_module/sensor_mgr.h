#ifndef SENSOR_MGR_H
#define SENSOR_MGR_H

#include <Arduino.h>
#include <Wire.h>
#include <ToF_sensor.h>
#include "register_storage.h"


class SensorMgr
{
public:
    SensorMgr(RegisterStorage &aRegisterStorage, uint8_t aMainSensorErrorCode, uint8_t aAuxSensorErrorCode);

    void begin();
    void end();
    void update();
    uint8_t status() const;
    void resetMainMeasureCount();
    void resetAuxMeasureCount();

private:
    RegisterStorage &mRegisters;
    bool mainSensorWired;
    bool auxSensorWired;
    ToF_longRange mainSensor;
    ToF_longRange auxSensor;
    uint8_t mainMeasureCount;
    uint8_t auxMeasureCount;
};


#endif
