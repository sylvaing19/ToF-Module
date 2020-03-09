/*
    Name:       src.ino
    Created:	07/09/2019 22:42:46
    Author:     sg-msi\Sylvain
*/

#include "OneWireMInterface.h"
#include "ToF_module.h"

/*
    This example is supposed to work as-is with Arduino UNO/Mega and Teensy 3.x
    For other platforms, you may need to change the HardwareSerial used
*/

#if defined ARDUINO_AVR_MEGA2560 || defined ARDUINO_AVR_MEGA || defined ARDUINO_AVR_ADK || defined ARDUINO_AVR_LEONARDO || defined TEENSYDUINO
#define SERIAL_TOF Serial1
#define SERIAL_DBG Serial
#define BAUDRATE_DBG 115200
#else
#define SERIAL_TOF Serial
#endif

#define TOF_MODULE_ID 1
#define TOF_MODULE_BAUDRATE 200000
#define USING_AUX_SENSOR false


void display_debug(const char* name, uint8_t com_status, const ToF_module & tof)
{
#ifdef SERIAL_DBG
    SERIAL_DBG.print(name);
    SERIAL_DBG.print(": com=");
    SERIAL_DBG.print(com_status);
    SERIAL_DBG.print(" tof=");
    SERIAL_DBG.println(tof.status());
#endif
    if (com_status != OW_STATUS_OK || tof.error()) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
    }
}

void setup() {}

void loop()
{
#ifdef SERIAL_DBG
    SERIAL_DBG.begin(BAUDRATE_DBG);
#endif
    delay(1000);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    uint32_t start = millis();
    uint32_t now = 0;
    uint32_t last = 0;

    /* One-wire interface */
    /* The provided HardwareSerial must be used only by one interface */
    OneWireMInterface tofInterface(SERIAL_TOF);

    /* ToF Module */
    /* Define as many as needed, if they use the same interface they must have distinct IDs */
    ToF_module tof(tofInterface, TOF_MODULE_ID);

    /* Begin the interface like a regular Stream, providing the baudrate and optionally the timeout */
    /* The default timeout is 50 ms */
    tofInterface.begin(TOF_MODULE_BAUDRATE);

    OneWireStatus com_status;
    /* Module initialization */
    com_status = tof.init();
    display_debug("init", com_status, tof);
    if (com_status != OW_STATUS_OK) {
        /*
            If the init failed, the Status Return Level is set to 0
            Afterwards, commands will still be sent but without expecting an answer,
            thus they will always succeed even though nothing will be received from
            the sensor.
        */
        delay(5000);
        goto end;
    }

    if (tof.statusReturnLevel() == 0) {
        /*
            If the Status Return Level is 0, it will not be possible to read the
            measurements of the sensor.
        */
#ifdef SERIAL_DBG
        SERIAL_DBG.println("Status Return Level is 0. Cannot read tof registers.");
#endif
        delay(10000);
        goto end;
    }

    /* Start measurements (only necessary if AUTO_START is set to false) */
    com_status = tof.enable();
    display_debug("enable", com_status, tof);
    com_status = tof.auxEnable();
    display_debug("auxEnable", com_status, tof);

    /* If possible, display the entire memory of the tof */
#ifdef SERIAL_DBG
    SERIAL_DBG.println("Addr\tValue");
    for (size_t i = 0; i < 50; i++) {
        uint8_t data = 0;
        com_status = tof.read(i, data);
        if (com_status != OW_STATUS_OK) {
            SERIAL_DBG.print("Errno: ");
            SERIAL_DBG.println(com_status);
        }
        SERIAL_DBG.print(i);
        SERIAL_DBG.print("\t");
        SERIAL_DBG.println(data);
    }
#endif

    if (!tof.mainWired() || (USING_AUX_SENSOR && !tof.auxWired())) {
        /*
            If the module failed to communicate with a sensor, it will mark it as
            not wired. Ranging measurement will not be attempted on non-wired sensors.
        */
#ifdef SERIAL_DBG
        SERIAL_DBG.println("The sensors are not wired to the module.");
#endif
        delay(10000);
        goto end;
    }

    

    /* Get distance */
    while (now < 60000) {
        now = millis() - start;
        TofValue val1 = tof.readRange();
        TofValue val2 = tof.auxReadRange();
#ifdef SERIAL_DBG
        if (val1 != SENSOR_NOT_UPDATED || val2 != SENSOR_NOT_UPDATED) {
            SERIAL_DBG.print(now - last);
            SERIAL_DBG.print("\t");
            SERIAL_DBG.print(val1);
            SERIAL_DBG.print("\t");
            SERIAL_DBG.println(val2);
            last = now;
        }
        if (now - last > 3000) {
            break;
        }
#endif
    }

end:
    /* Close the interface */
    tofInterface.end();
#ifdef SERIAL_DBG
    SERIAL_DBG.end();
#endif
}
