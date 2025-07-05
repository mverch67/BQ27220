#include "BQ27220.h"

#define BQ27220_I2C_ADDRESS 0x55 // Device I2C address
#define BQ27220_DEVICE_ID 0x0220  // Device ID

// Commands
#define BQ27220_COMMAND_CONTROL         0x00
#define BQ27220_COMMAND_TEMP            0x06
#define BQ27220_COMMAND_BATTERY_STATUS  0x0A
#define BQ27220_COMMAND_VOLT            0x08
#define BQ27220_COMMAND_BAT_STATUS      0x0A
#define BQ27220_COMMAND_CURR            0x0C
#define BQ27220_COMMAND_REMAIN_CAPACITY 0x10
#define BQ27220_COMMAND_FULL_CHARGE_CAP 0x12
#define BQ27220_COMMAND_AVG_CURR        0x14
#define BQ27220_COMMAND_TTE             0x16
#define BQ27220_COMMAND_TTF             0x18
#define BQ27220_COMMAND_STANDBY_CURR    0x1A
#define BQ27220_COMMAND_STTE            0x1C
#define BQ27220_COMMAND_MAX_LOAD_CURR   0x1E
#define BQ27220_COMMAND_MAX_LOAD_TTE    0x20
#define BQ27220_COMMAND_RAW_COULOMB_CNT 0x22
#define BQ27220_COMMAND_AVG_PWR         0x24
#define BQ27220_COMMAND_INT_TEMP        0x28
#define BQ27220_COMMAND_STATE_CHARGE    0x2C
#define BQ27220_COMMAND_STATE_HEALTH    0x2E
#define BQ27220_COMMAND_CHARGING_VOLT   0x30
#define BQ27220_COMMAND_CHARGING_CURR   0x32
#define BQ27220_COMMAND_ANALOG_CURR     0x79
#define BQ27220_COMMAND_RAW_CURR        0x7A
#define BQ27220_COMMAND_RAW_VOLT        0x7C
#define BQ27220_COMMAND_RAW_TEMP        0x7E

// Control Subcommands
#define BQ27220_CONTROL_STATUS           0x0000
#define BQ27220_CONTROL_DEVICE_NUMBER    0x0001
#define BQ27220_CONTROL_FW_VERSION       0x0002
#define BQ27220_CONTROL_HW_VERSION       0x0003
#define BQ27220_CONTROL_CONTROL_STATUS   0x0006
#define BQ27220_CONTROL_BOARD_OFFSET     0x0009
#define BQ27220_CONTROL_CC_OFFSET        0x000A
#define BQ27220_CONTROL_CC_OFFSET_SAVE   0x000B
#define BQ27220_CONTROL_OCV_CMD          0x000C
#define BQ27220_CONTROL_BAT_INSERT       0x000D
#define BQ27220_CONTROL_BAT_REMOVE       0x000E
#define BQ27220_CONTROL_SET_SNOOZE       0x0013
#define BQ27220_CONTROL_CLEAR_SNOOZE     0x0014
#define BQ27220_CONTROL_SET_PROFILE_1    0x0015
#define BQ27220_CONTROL_SET_PROFILE_2    0x0016
#define BQ27220_CONTROL_SET_PROFILE_3    0x0017
#define BQ27220_CONTROL_SET_PROFILE_4    0x0018
#define BQ27220_CONTROL_SET_PROFILE_5    0x0019
#define BQ27220_CONTROL_SET_PROFILE_6    0x001A
#define BQ27220_CONTROL_CAL_TOGGLE       0x002D
#define BQ27220_CONTROL_SEALED           0x0030
#define BQ27220_CONTROL_RESET            0x0041
#define BQ27220_CONTROL_OPERATION_STATUS 0x0054
#define BQ27220_CONTROL_GAUGING_STATUS   0x0056
#define BQ27220_CONTROL_EXIT_CAL         0x0080
#define BQ27220_CONTROL_ENTER_CAL        0x0081
#define BQ27220_CONTROL_ENTER_CFG_UPDATE 0x0090
#define BQ27220_CONTROL_EXIT_CFG_UPDATE_REINIT 0x0091
#define BQ27220_CONTROL_EXIT_CFG_UPDATE  0x0092
#define BQ27220_CONTROL_RETURN_TO_ROM    0x0F00

#define BQ27220_UNSEAL_KEY1 0x0414
#define BQ27220_UNSEAL_KEY2 0x3672
#define BQ27220_SUPER_KEY   0xFFFF


BQ27220::BQ27220() : addr{BQ27220_I2C_ADDRESS} {}

bool BQ27220::init(TwoWire *wire, int sda, int scl, bool initWire)
{
    this->wire = wire;
    this->sda = sda;
    this->scl = scl;

#if defined(ARCH_ESP32)
    if (initWire) {
        if (sda >= 0 && scl >= 0) {
            wire->setPins(sda, scl);
        }
        wire->begin();
        wire->setClock(400000); // Set I2C clock speed to 400kHz
#if 0
        // Check if device is present (disabled here)
        uint16_t id = readCtrlWord(BQ27220_CONTROL_DEVICE_NUMBER);
        if (id != BQ27220_DEVICE_ID)
        {
            return false; // Device not found
        }
#endif
    }
#endif
    return true;
}

uint16_t BQ27220::getTemp()
{
    return readWord(BQ27220_COMMAND_TEMP);
}

uint16_t BQ27220::getBatterySt(void)
{
    return readWord(BQ27220_COMMAND_BATTERY_STATUS);
}

bool BQ27220::getIsCharging(void)
{
    uint16_t ret = readWord(BQ27220_COMMAND_BATTERY_STATUS);
    bat_st.full = ret;
    return !bat_st.st.DSG;
}

uint16_t BQ27220::getRemainCap()
{
    return readWord(BQ27220_COMMAND_REMAIN_CAPACITY);
}

uint16_t BQ27220::getFullChargeCap(void)
{
    return readWord(BQ27220_COMMAND_FULL_CHARGE_CAP);
}

uint16_t BQ27220::getChargePcnt(void)
{
    return readWord(BQ27220_COMMAND_STATE_CHARGE);
}

uint16_t BQ27220::getVolt(VOLT_MODE type)
{
    switch (type)
    {
    case VOLT:
        return readWord(BQ27220_COMMAND_VOLT);
    case VOLT_CHARGING:
        return readWord(BQ27220_COMMAND_CHARGING_VOLT);
    case VOLT_RAW:
        return readWord(BQ27220_COMMAND_RAW_VOLT);
    default:
        break;
    }
    return 0;
}

int16_t BQ27220::getCurr(CURR_MODE type)
{
    switch (type)
    {
    case CURR_RAW:
        return (int16_t)readWord(BQ27220_COMMAND_RAW_CURR);
    case CURR_INSTANT:
        return (int16_t)readWord(BQ27220_COMMAND_CURR);
    case CURR_STANDBY:
        return (int16_t)readWord(BQ27220_COMMAND_STANDBY_CURR);
    case CURR_CHARGING:
        return (int16_t)readWord(BQ27220_COMMAND_CHARGING_CURR);
    case CURR_AVERAGE:
        return (int16_t)readWord(BQ27220_COMMAND_AVG_CURR);
    default:
        break;
    }
    return -1;
}

uint16_t BQ27220::getId()
{
    return BQ27220_DEVICE_ID;
}

uint16_t BQ27220::getDesignCapacity()
{
    return readWord(0x3C); // Design Capacity register address
}

bool BQ27220::setDesignCapacity(uint16_t newDesignCapacity)
{
    // 1. Unseal (here we assume device is not sealed; a separate method should handle it)

    // 2. Enter Configuration Update Mode
    writeCtrlWord(BQ27220_CONTROL_ENTER_CFG_UPDATE);

    delay(100); // small delay for mode switch

    // 3. Write new value to Design Capacity register (0x3C)
    uint8_t payload[2] = {
        static_cast<uint8_t>(newDesignCapacity & 0xFF),
        static_cast<uint8_t>((newDesignCapacity >> 8) & 0xFF)
    };
    i2cWriteBytes(0x3C, payload, 2);

    // 4. Checksum is omitted here, but in real applications it should be calculated and written

    // 5. Exit Configuration Update Mode and Reinitialize
    writeCtrlWord(BQ27220_CONTROL_EXIT_CFG_UPDATE_REINIT);

    delay(100);

    return true; // Considered always successful in this example
}

bool BQ27220::unseal() {
    // Unseal command - typically requires two steps
    bool ok1 = writeCtrlWord(BQ27220_UNSEAL_KEY1);
    bool ok2 = writeCtrlWord(BQ27220_UNSEAL_KEY2);
    delay(10);
    return ok1 && ok2;
}

bool BQ27220::seal() {
    // Seal command
    return writeCtrlWord(BQ27220_CONTROL_SEALED);
}

bool BQ27220::isSealed() {
    // Read Flags register
    uint16_t flags = readCtrlWord(BQ27220_CONTROL_CONTROL_STATUS);  // 0x0006: Control Status (Flags)
    // Bit 13 (0x2000) indicates sealed status
    return (flags & 0x2000) != 0;
}

uint16_t BQ27220::readWord(uint16_t subAddress)
{
    uint8_t data[2];
    i2cReadBytes((uint8_t)subAddress, data, 2);
    return ((uint16_t)data[1] << 8) | data[0];
}

bool BQ27220::writeCtrlWord(uint16_t cmd)
{
    uint8_t msb = (cmd >> 8);
    uint8_t lsb = (cmd & 0xFF);
    uint8_t buffer[2] = { lsb, msb };
    return i2cWriteBytes(BQ27220_COMMAND_CONTROL, buffer, 2);
}

uint16_t BQ27220::readCtrlWord(uint16_t fun)
{
    uint8_t msb = (fun >> 8);
    uint8_t lsb = (fun & 0xFF);
    uint8_t cmd[2] = { lsb, msb };
    uint8_t data[2] = { 0 };

    i2cWriteBytes(BQ27220_COMMAND_CONTROL, cmd, 2);

    if (i2cReadBytes(0, data, 2))
    {
        return ((uint16_t)data[1] << 8) | data[0];
    }
    return 0xFFFF;
}

bool BQ27220::i2cWriteBytes(uint8_t subAddress, uint8_t *data, uint8_t length)
{
    wire->beginTransmission(addr);
    wire->write(subAddress);
    for (uint8_t i = 0; i < length; i++) {
        wire->write(data[i]);
    }
    return (wire->endTransmission() == 0);
}

bool BQ27220::i2cReadBytes(uint8_t subAddress, uint8_t *data, uint8_t length)
{
    wire->beginTransmission(addr);
    wire->write(subAddress);
    if (wire->endTransmission(false) != 0) { // Send repeated start
        return false;
    }
    uint8_t bytesRead = wire->requestFrom(addr, length);
    if (bytesRead != length) {
        return false;
    }
    for (uint8_t i = 0; i < length; i++) {
        data[i] = wire->read();
    }
    return true;
}
