#ifndef SENSOR_MGR_H
#define SENSOR_MGR_H

#include <Arduino.h>
#include <Wire.h>
#include <ToF_sensor.h>


class SensorMgr
{
public:
    SensorMgr(uint8_t aMainSensorErrorCode, uint8_t aAuxSensorErrorCode);

    void begin();
    void end();
    uint8_t wiringStatus() const;

};


#endif
