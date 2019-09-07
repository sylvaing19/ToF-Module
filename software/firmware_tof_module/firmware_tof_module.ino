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
#include "utils.h"

#define INPUT_VOLTAGE_UPDATE_PERIOD 10000 // ms


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
static SensorMgr sensorMgr(registers);
bool running;
bool f_reset_requested;


void read(uint8_t address, uint8_t size, uint8_t *data)
{
    registers.read(address, size, data);
    if (check_buffer_intersect(address, size, REG_MAIN_RANGE, 6)) {
        sensorMgr.resetMainMeasureCount();
    }
    if (check_buffer_intersect(address, size, REG_AUX_RANGE, 6)) {
        sensorMgr.resetAuxMeasureCount();
    }
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

void main_sensor_ready()
{
    sensorMgr.mainSensorReady();
}

void aux_sensor_ready()
{
    sensorMgr.auxSensorReady();
}

void setup()
{
    slaveInterface.setReadCallback(read);
    slaveInterface.setWriteCallback(write);
    slaveInterface.setSoftResetCallback(soft_reset);
    slaveInterface.setFactoryResetCallback(factory_reset);
    pinMode(MAIN_SENSOR_INT_PIN, INPUT);
    pinMode(AUX_SENSOR_INT_PIN, INPUT);
    attachInterrupt(MAIN_SENSOR_INT_PIN, main_sensor_ready, FALLING);
    attachInterrupt(AUX_SENSOR_INT_PIN, aux_sensor_ready, FALLING);
}

void loop()
{
    uint32_t last_vcc_update = 0;
    uint32_t now;
    running = true;
    f_reset_requested = false;
    registers.init();
    sensorMgr.begin();
    slaveInterface.begin(baudrate(registers.getBaudrate()));
    while (running) {
        /* Input voltage update */
        now = millis();
        if (now - last_vcc_update > INPUT_VOLTAGE_UPDATE_PERIOD) {
            last_vcc_update = now;
            long vcc = read_vcc();
            vcc /= 40;
            registers.writeRAM(REG_INPUT_VOLTAGE, (uint8_t)vcc);
        }

        /* Sensors update */
        sensorMgr.update();
        slaveInterface.setHardwareStatus(sensorMgr.status());

        /* Communication with master */
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
