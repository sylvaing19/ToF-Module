#ifndef SENSOR_MGR_H
#define SENSOR_MGR_H

#include <Arduino.h>
#include <Wire.h>
#include "sensor.h"
#include "register_storage.h"

#define MAIN_SENSOR_INT_PIN 2
#define AUX_SENSOR_INT_PIN 3


class SensorMgr
{
public:
    SensorMgr(RegisterStorage &aRegisterStorage);

    void begin();
    void end();
    void update();
    uint8_t status() const;
    void resetMainMeasureCount();
    void resetAuxMeasureCount();
    void mainSensorReady();
    void auxSensorReady();

private:
    Sensor mainSensor;
    Sensor auxSensor;
};


#endif
