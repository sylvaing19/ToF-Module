#include "ToF_module.h"

ToF_module::ToF_module(OneWireMInterface & aInterface, uint8_t aId) :
    mInterface(aInterface), mStatusReturnLevel(0), mID(aId)
{
    mStatus = TOF_STATUS_OK;
    mMainWired = false;
    mAuxWired = false;
}

OneWireStatus ToF_module::init()
{
    mStatusReturnLevel = 1;
    OneWireStatus ret = ping();
    if (ret != OW_STATUS_OK) {
        mStatusReturnLevel = 0;
        return ret;
    }
    ret = read(TOF_STATUS_RETURN_LEVEL, mStatusReturnLevel);
    if (ret == OW_STATUS_TIMEOUT) {
        mStatusReturnLevel = 0;
        return OW_STATUS_OK;
    }
    else if (ret != OW_STATUS_OK) {
        mStatusReturnLevel = 0;
        return ret;
    }
    uint8_t wiringStatus = 0;
    ret = read(TOF_WIRING_STATUS, wiringStatus);
    if (ret == OW_STATUS_OK) {
        mMainWired = wiringStatus & 1;
        mAuxWired = wiringStatus & 2;
    }
    return ret;
}

bool ToF_module::internalError() const
{
    return (mStatus & (
        TOF_STATUS_INPUT_VOLTAGE_ERROR |
        TOF_STATUS_MAIN_SENSOR_ERROR |
        TOF_STATUS_AUX_SENSOR_ERROR)) != 0;
}

bool ToF_module::commandError() const
{
    return (mStatus & (
        TOF_STATUS_RANGE_ERROR |
        TOF_STATUS_CHECKSUM_ERROR |
        TOF_STATUS_INSTRUCTION_ERROR)) != 0;
}

int ToF_module::setId(uint8_t newId)
{
    OneWireStatus ret = write(TOF_ID, newId);
    if (ret == OW_STATUS_OK && !commandError()) {
        mID = newId;
        return EXIT_SUCCESS;
    }
    else {
        return EXIT_FAILURE;
    }
}

uint16_t ToF_module::model()
{
    uint16_t result = 0;
    if (read(TOF_MODEL_NUMBER, result) == OW_STATUS_OK) {
        return result;
    }
    else {
        return 0;
    }
}

uint8_t ToF_module::firmware()
{
    uint8_t result = 0;
    if (read(TOF_FIRMWARE_VERSION, result) == OW_STATUS_OK) {
        return result;
    }
    else {
        return 0;
    }
}

int ToF_module::communicationSpeed(uint32_t aBaudrate)
{
    uint8_t value = 2000000 / aBaudrate - 1;
    if (value == 0) { // forbid 2MBd rate, because it is out of spec, and can be difficult to undo
        return EXIT_FAILURE;
    }
    OneWireStatus ret = write(TOF_BAUDRATE, value);
    if (ret == OW_STATUS_OK && !commandError()) {
        return EXIT_SUCCESS;
    }
    else {
        return EXIT_FAILURE;
    }
}

int ToF_module::statusReturnLevel(uint8_t aSRL)
{
    OneWireStatus ret = write(TOF_STATUS_RETURN_LEVEL, aSRL);
    if (ret == OW_STATUS_OK && !commandError()) {
        mStatusReturnLevel = aSRL;
        return EXIT_SUCCESS;
    }
    else {
        return EXIT_FAILURE;
    }
}

int ToF_module::isPolling(bool &main, bool &aux)
{
    OneWireStatus ret;
    ret = read(TOF_MAIN_POLLING, main);
    bool ret1 = ret == OW_STATUS_OK && !commandError();
    ret = read(TOF_AUX_POLLING, aux);
    bool ret2 = ret == OW_STATUS_OK && !commandError();

    if (ret1 && ret2) {
        return EXIT_SUCCESS;
    }
    else {
        return EXIT_FAILURE;
    }
}

int ToF_module::setPolling(bool main, bool aux)
{
    OneWireStatus ret;
    ret = write(TOF_MAIN_POLLING, main);
    bool ret1 = ret == OW_STATUS_OK && !commandError();
    ret = write(TOF_AUX_POLLING, aux);
    bool ret2 = ret == OW_STATUS_OK && !commandError();

    if (ret1 && ret2) {
        return EXIT_SUCCESS;
    }
    else {
        return EXIT_FAILURE;
    }
}

float ToF_module::inputVoltage()
{
    uint8_t voltage = 0;
    if (read(TOF_INPUT_VOLTAGE, voltage) == OW_STATUS_OK) {
        return (float)voltage * 4 / 100;
    }
    else {
        return 0;
    }
}

OneWireStatus ToF_module::enable(bool e)
{
    return write(TOF_MAIN_ENABLED, e);
}

OneWireStatus ToF_module::auxEnable(bool e)
{
    return write(TOF_AUX_ENABLED, e);
}

uint8_t ToF_module::available()
{
    uint8_t mcslr = 0;
    if (mMainWired && read(TOF_MAIN_MCSLR, mcslr) == OW_STATUS_OK) {
        return mcslr;
    }
    else {
        return 0;
    }
}

TofValue ToF_module::readRange()
{
    uint8_t result[3] = { 0, };
    if (mMainWired && read(TOF_MAIN_MCSLR, result) == OW_STATUS_OK) {
        if (mStatus & TOF_STATUS_MAIN_SENSOR_ERROR) {
            return (TofValue)SENSOR_DEAD;
        }
        else if (result[0] == 0) {
            return (TofValue)SENSOR_NOT_UPDATED;
        }
        else {
            uint16_t res = (uint16_t)result[1] + ((uint16_t)result[2] << 8);
            return (TofValue)res;
        }
    }
    else {
        return (TofValue)SENSOR_NOT_UPDATED;
    }
}

uint8_t ToF_module::auxAvailable()
{
    uint8_t mcslr = 0;
    if (mAuxWired && read(TOF_AUX_MCSLR, mcslr) == OW_STATUS_OK) {
        return mcslr;
    }
    else {
        return 0;
    }
}

TofValue ToF_module::auxReadRange()
{
    uint8_t result[3] = { 0, };
    if (mAuxWired && read(TOF_AUX_MCSLR, result) == OW_STATUS_OK) {
        if (mStatus & TOF_STATUS_AUX_SENSOR_ERROR) {
            return (TofValue)SENSOR_DEAD;
        }
        else if (result[0] == 0) {
            return (TofValue)SENSOR_NOT_UPDATED;
        }
        else {
            uint16_t res = (uint16_t)result[1] + ((uint16_t)result[2] << 8);
            return (TofValue)res;
        }
    }
    else {
        return (TofValue)SENSOR_NOT_UPDATED;
    }
}
