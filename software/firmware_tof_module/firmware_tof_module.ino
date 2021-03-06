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
#include <SoftwareSerial.h>

#define DEBUG 1
#define INPUT_VOLTAGE_UPDATE_PERIOD 10000 // ms


enum ErrCode
{
    MAIN_SENSOR_ERROR = 1,
    AUX_SENSOR_ERROR =  2,
    RANGE_ERROR =       8,
    CHECKSUM_ERROR =    16,
    INSTRUCTION_ERROR = 64,
};

#if DEBUG
SoftwareSerial debug(PIN_DEBUG_C, PIN_DEBUG_D);
#endif
static RegisterStorage registers(RANGE_ERROR);
#if DEBUG
static OneWireSInterface slaveInterface(Serial, INSTRUCTION_ERROR, CHECKSUM_ERROR, OneWireInterface::NO_DIR_PORT, &debug);
static SensorMgr sensorMgr(registers, &debug);
#else
static OneWireSInterface slaveInterface(Serial, INSTRUCTION_ERROR, CHECKSUM_ERROR, OneWireInterface::NO_DIR_PORT);
static SensorMgr sensorMgr(registers);
#endif
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
#if DEBUG
    debug.println("factory reset");
#endif
    f_reset_requested = true;
    running = false;
}

void soft_reset()
{
#if DEBUG
    debug.println("soft reset");
#endif
    running = false;
}

void main_sensor_ready()
{
#if DEBUG
    static bool led_state = false;
    led_state = !led_state;
    digitalWrite(PIN_DEBUG_A, led_state);
#endif

    sensorMgr.mainSensorReady();
}

void aux_sensor_ready()
{
#if DEBUG
    static bool led_state = false;
    led_state = !led_state;
    digitalWrite(PIN_DEBUG_B, led_state);
#endif

    sensorMgr.auxSensorReady();
}

void setup()
{
#if DEBUG
    debug.begin(115200);
    debug.println("Debug serial");
    pinMode(PIN_DEBUG_A, OUTPUT);
    pinMode(PIN_DEBUG_B, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
#endif
    slaveInterface.setReadCallback(read);
    slaveInterface.setWriteCallback(write);
    slaveInterface.setSoftResetCallback(soft_reset);
    slaveInterface.setFactoryResetCallback(factory_reset);
    pinMode(MAIN_SENSOR_INT_PIN, INPUT);
    pinMode(AUX_SENSOR_INT_PIN, INPUT);
    attachInterrupt(0, main_sensor_ready, FALLING);
    attachInterrupt(1, aux_sensor_ready, FALLING);
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
    slaveInterface.setID(registers.getId());
    slaveInterface.setRDT((uint32_t)registers.getReturnDelayTime() * 2);
    slaveInterface.setSRL(registers.getStatusReturnLevel());

#if DEBUG
    debug.println("Registers:");
    uint8_t d[51];
    registers.read(0, 51, d);
    for (size_t i = 0; i < 51; i++) {
        debug.print(i);
        debug.print("\t");
        debug.println(d[i]);
    }
    debug.println("-end-");
#endif

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

#if DEBUG
        static uint32_t led_timer = 0;
        static bool led_state = false;
        if (now - led_timer > 500) {
            led_timer = now;
            led_state = !led_state;
            digitalWrite(LED_BUILTIN, led_state);
        }
#endif

    }
    slaveInterface.end();
    sensorMgr.end();

    if (f_reset_requested) {
        registers.resetEEPROM();
    }
}
