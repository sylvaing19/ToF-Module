/*
    Name:       firmware_tof_module.ino
    Created:	02/09/2019 20:45:54
    Author:     Sylvain Gaultier
*/


#include <Arduino.h>
#include <Wire.h>
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
static SensorMgr sensorMgr(registers, MAIN_SENSOR_ERROR, AUX_SENSOR_ERROR);
bool running;
bool f_reset_requested;


void read(uint8_t address, uint8_t size, uint8_t *data)
{
    registers.read(address, size, data);
}

uint8_t write(uint8_t address, uint8_t size, const uint8_t *data)
{
    return registers.write(address, size, data);
}

void factory_reset()
{
    f_reset_requested = true;
    running = false;
}

void soft_reset()
{
    running = false;
}

uint32_t baudrate(uint8_t stored_baudrate)
{
    return 2000000 / ((uint32_t)stored_baudrate + 1);
}

void setup()
{
    slaveInterface.setReadCallback(read);
    slaveInterface.setWriteCallback(write);
    slaveInterface.setSoftResetCallback(soft_reset);
    slaveInterface.setFactoryResetCallback(factory_reset);
}

void loop()
{
    running = true;
    f_reset_requested = false;
    registers.init();
    sensorMgr.begin();
    slaveInterface.begin(baudrate(registers.getBaudrate()));
    while (running) {
        sensorMgr.update();
        slaveInterface.setHardwareStatus(sensorMgr.status());
        slaveInterface.communicate();

        /* Update slaveInterface settings if needed */
        if (!slaveInterface.waitingToSendPacket()) {
            if (registers.idChanged()) {
                slaveInterface.setID(registers.getId());
            }
            if (registers.baudrateChanged()) {
                slaveInterface.end();
                slaveInterface.begin(baudrate(registers.getBaudrate()));
            }
            if (registers.returnDelayTimeChanged()) {
                slaveInterface.setRDT((uint32_t)registers.getReturnDelayTime() * 2);
            }
            if (registers.statusReturnLevelChanged()) {
                slaveInterface.setSRL(registers.getStatusReturnLevel());
            }
        }
    }
    slaveInterface.end();
    sensorMgr.end();

    if (f_reset_requested) {
        registers.resetEEPROM();
    }
}
