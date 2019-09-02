/*
    Name:       firmware_tof_module.ino
    Created:	02/09/2019 20:45:54
    Author:     Sylvain Gaultier
*/


#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ToF_sensor.h>
#include <OneWireSInterface.h>
#include "register_storage.h"
#include "sensor_mgr.h"


enum ErrCode
{
    MAIN_SENSOR_ERROR = 1,
    AUX_SENSOR_ERROR =  2,
    RANGE_ERROR =       8,
    CHECKSUM_ERROR =    16,
    INSTRUCTION_ERROR = 64,
};


static RegisterStorage registers(RANGE_ERROR);
static OneWireSInterface slaveInterface(Serial, INSTRUCTION_ERROR, CHECKSUM_ERROR);
static SensorMgr sensorMgr(MAIN_SENSOR_ERROR, AUX_SENSOR_ERROR);
bool running;
bool f_reset_requested;


void factory_reset()
{
    f_reset_requested = true;
    running = false;
}

void soft_reset()
{
    running = false;
}

void setup() {}

void loop()
{
    running = true;
    f_reset_requested = false;
    registers.init();
    sensorMgr.begin();

    //slaveInterface.begin(...);
    while (running) {

    }
    slaveInterface.end();
    sensorMgr.end();

    if (f_reset_requested) {
        registers.resetEEPROM();
    }
    delay(50);
}
