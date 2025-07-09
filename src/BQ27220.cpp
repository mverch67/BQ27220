#include "BQ27220.h"

#define BQ27220_I2C_ADDRESS 0x55 // device addr
#define BQ27220_DEVICE_ID 0x0220 // device id

// commands
#define BQ27220_COMMAND_CONTROL         0x00
#define BQ27220_COMMAND_TEMP            0x06
#define BQ27220_COMMAND_BATTERY_ST      0x0A
#define BQ27220_COMMAND_VOLT            0x08
#define BQ27220_COMMAND_BAT_STA         0x0A
#define BQ27220_COMMAND_CURR            0x0C
#define BQ27220_COMMAND_REMAIN_CAPACITY 0x10
#define BQ27220_COMMAND_FCHG_CAPACITY   0x12
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
#define BQ27220_COMMAND_OPER_STATUS     0x3B
#define BQ27220_COMMAND_DESIGN_CAPACITY 0x3C
#define BQ27220_COMMAND_MAC_DATA_SUM    0x60
#define BQ27220_COMMAND_MAC_DATA_LEN    0x61
#define BQ27220_COMMAND_BLK_DATA_CTRL   0x61
#define BQ27220_COMMAND_ANALOG_CURR     0x79
#define BQ27220_COMMAND_RAW_CURR        0x7A
#define BQ27220_COMMAND_RAW_VOLT        0x7C
#define BQ27220_COMMAND_RAW_TEMP        0x7E

#define BQ27220_CONTROL_CONTROL_STATUS         0x0000
#define BQ27220_CONTROL_DEVICE_NUMBER          0x0001
#define BQ27220_CONTROL_FW_VERSION             0x0002
#define BQ27220_CONTROL_HW_VERSION             0x0003
#define BQ27220_CONTROL_BOARD_OFFSET           0x0009
#define BQ27220_CONTROL_CC_OFFSET              0x000A
#define BQ27220_CONTROL_CC_OFFSET_SAVE         0x000B
#define BQ27220_CONTROL_OCV_CMD                0x000C
#define BQ27220_CONTROL_BAT_INSERT             0x000D
#define BQ27220_CONTROL_BAT_REMOVE             0x000E
#define BQ27220_CONTROL_SET_SNOOZE             0x0013
#define BQ27220_CONTROL_CLEAR_SNOOZE           0x0014
#define BQ27220_CONTROL_SET_PROFILE_1          0x0015
#define BQ27220_CONTROL_SET_PROFILE_2          0x0016
#define BQ27220_CONTROL_SET_PROFILE_3          0x0017
#define BQ27220_CONTROL_SET_PROFILE_4          0x0018
#define BQ27220_CONTROL_SET_PROFILE_5          0x0019
#define BQ27220_CONTROL_SET_PROFILE_6          0x001A
#define BQ27220_CONTROL_CAL_TOGGLE             0x002D
#define BQ27220_CONTROL_SEALED                 0x0030
#define BQ27220_CONTROL_RESET                  0x0041
#define BQ27220_CONTROL_OERATION_STATUS        0x0054
#define BQ27220_CONTROL_GAUGING_STATUS         0x0056
#define BQ27220_CONTROL_EXIT_CAL               0x0080
#define BQ27220_CONTROL_ENTER_CAL              0x0081
#define BQ27220_CONTROL_ENTER_CFG_UPDATE       0x0090
#define BQ27220_CONTROL_EXIT_CFG_UPDATE_REINIT 0x0091
#define BQ27220_CONTROL_EXIT_CFG_UPDATE        0x0092
#define BQ27220_CONTROL_RETURN_TO_ROM          0x0F00

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
        wire->setClock(400000); // Set I2C clock to 400kHz
#if 0
        // Check if the device is present
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

uint16_t BQ27220::getId()
{
    return 0x0220;
}

uint16_t BQ27220::getTemp()
{
    return readWord(BQ27220_COMMAND_TEMP);
}

uint16_t BQ27220::getBatterySt(void)
{
    return readWord(BQ27220_COMMAND_BATTERY_ST);
}

bool BQ27220::getIsCharging(void)
{
    uint16_t ret = readWord(BQ27220_COMMAND_BATTERY_ST);
    bat_st.full = ret;
    return !bat_st.st.DSG;
}

uint16_t BQ27220::getRemainCap()
{
    return readWord(BQ27220_COMMAND_REMAIN_CAPACITY);
}

uint16_t BQ27220::getFullChargeCap(void)
{
    return readWord(BQ27220_COMMAND_FCHG_CAPACITY);
}

uint16_t BQ27220::getDesignCap()
{
    return readWord(BQ27220_COMMAND_DESIGN_CAPACITY);
}

uint16_t BQ27220::getChargePcnt(void)
{
    return readWord(BQ27220_COMMAND_STATE_CHARGE);
}

uint16_t BQ27220::getTimeToEmpty(void)
{
    return readWord(BQ27220_COMMAND_TTE);
}

uint16_t BQ27220::getTimeToFull(void)
{
    return readWord(BQ27220_COMMAND_TTF);
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

bool BQ27220::setDesignCap(uint16_t designCapacity)
{
    bool sealed = isSealed();
    if (sealed) unseal();

    // Enter config update mode
    writeCtrlWord(BQ27220_CONTROL_ENTER_CFG_UPDATE);
    delay(100);

    // Wait upto 1s for device until entering config update mode
    int time = 0;
    while (readCtrlWord(BQ27220_COMMAND_OPER_STATUS) & 0x0010) {
        delay(10);
        if (time++ > 100) return false;
    }

    // Write 0x9F to 0x3E to access the MSB of Design Capacity
    uint8_t msb_dc = 0x9F;
    i2cWriteBytes(0x3E, &msb_dc, 1);

    // Write 0x92 to 0x3F to access the LSB of Design Capacity
    uint8_t lsb_dc = 0x92;
    i2cWriteBytes(0x3F, &lsb_dc, 1);

    uint8_t oldChksum;
    i2cReadBytes(BQ27220_COMMAND_MAC_DATA_SUM, &oldChksum, 1);

    uint8_t dataLen;
    i2cReadBytes(BQ27220_COMMAND_MAC_DATA_LEN, &dataLen, 1);

    // Write new Design Capacity to 0x40
    uint8_t lsb = designCapacity & 0xFF;
    uint8_t msb = designCapacity >> 8;
    uint8_t addr = 0x40;
    i2cWriteBytes(addr, &msb, 1);
    i2cWriteBytes(addr + 1, &lsb, 1);

    // Read the whole 32-byte block data to calculate checksum
    uint8_t block[32];
    i2cReadBytes(0x40, block, 32);

    // calculate checksum
    uint8_t csum = 0;
    for (uint8_t i = 0; i < 32; i++) csum += block[i];
    csum = 0xFF - csum;
    i2cWriteBytes(0x60, &csum, 1);

    // write block length
    uint8_t zero = 0x24;
    i2cWriteBytes(0x61, &zero, 1);

    // Exit config update mode; may take up to 1s
    writeCtrlWord(BQ27220_CONTROL_EXIT_CFG_UPDATE);
    time = 0;
    while ((readCtrlWord(BQ27220_COMMAND_OPER_STATUS) & 0x0010) != 0) {
        delay(10);
        if (time++ > 100) return false;
    }

    if (sealed) seal();
    return true;
}

// --- helper functions ---

uint16_t BQ27220::readWord(uint16_t subAddress)
{
    uint8_t data[2];
    i2cReadBytes(subAddress, data, 2);
    return ((uint16_t)data[1] << 8) | data[0];
}

uint16_t BQ27220::readCtrlWord(uint16_t fun)
{
    uint8_t msb = (fun >> 8);
    uint8_t lsb = (fun & 0x00FF);
    uint8_t cmd[2] = {lsb, msb};
    uint8_t data[2] = {0};

    i2cWriteBytes((uint8_t)BQ27220_COMMAND_CONTROL, cmd, 2);

    if (i2cReadBytes((uint8_t)0, data, 2))
    {
        return ((uint16_t)data[1] << 8) | data[0];
    }
    return 0;
}

bool BQ27220::writeCtrlWord(uint16_t cmd)
{
    uint8_t msb = (cmd >> 8);
    uint8_t lsb = (cmd & 0xFF);
    uint8_t buffer[2] = { lsb, msb };
    return i2cWriteBytes(BQ27220_COMMAND_CONTROL, buffer, 2);
}

bool BQ27220::i2cReadBytes(uint8_t subAddress, uint8_t *dest, uint8_t count)
{
    wire->beginTransmission(addr);
    wire->write(subAddress);
    wire->endTransmission(true);

    uint8_t bytesRead = wire->requestFrom(addr, count);
    for (int i = 0; i < bytesRead && i < count; i++)
    {
        if (wire->available()) {
            dest[i] = wire->read();
        } else {
            dest[i] = 0;
        }
    }
    return bytesRead == count;
}

bool BQ27220::i2cWriteBytes(uint8_t subAddress, uint8_t *src, uint8_t count)
{
    wire->beginTransmission(addr);
    wire->write(subAddress);
    for (int i = 0; i < count; i++)
    {
        wire->write(src[i]);
    }
    wire->endTransmission(true);
    return true;
}

bool BQ27220::updateChecksum(uint8_t blockOffset)
{
    // Read the entire 32-byte block
    uint8_t block[32];
    for (uint8_t i = 0; i < 32; i++) {
        i2cReadBytes(blockOffset + i, &block[i], 1);
    }

    // Calculate checksum: 0xFF - (sum of all bytes)
    uint8_t csum = 0;
    for (uint8_t i = 0; i < 32; i++) {
        csum += block[i];
    }
    csum = 0xFF - csum;

    // Write checksum to the checksum register (usually blockOffset + 0x20)
    uint8_t checksumReg = blockOffset + 0x20;
    return i2cWriteBytes(checksumReg, &csum, 1);
}

bool BQ27220::fullAccess() {
    uint8_t key[2] = { (uint8_t)(BQ27220_SUPER_KEY & 0xFF), (uint8_t)(BQ27220_SUPER_KEY >> 8) };
    bool ok1 = i2cWriteBytes(BQ27220_COMMAND_CONTROL, key, 2);
    delay(1);
    bool ok2 = i2cWriteBytes(BQ27220_COMMAND_CONTROL, key, 2);
    delay(10);
    return ok1 && ok2;
}

bool BQ27220::isSealed() {
    uint16_t flags = readCtrlWord(BQ27220_CONTROL_CONTROL_STATUS);
    return (flags & 0x2000) != 0;
}

bool BQ27220::unseal() {
    uint8_t key1[2] = { (uint8_t)(BQ27220_UNSEAL_KEY1 & 0xFF), (uint8_t)(BQ27220_UNSEAL_KEY1 >> 8) };
    uint8_t key2[2] = { (uint8_t)(BQ27220_UNSEAL_KEY2 & 0xFF), (uint8_t)(BQ27220_UNSEAL_KEY2 >> 8) };
    bool ok1 = i2cWriteBytes(BQ27220_COMMAND_CONTROL, key1, 2);
    delay(1);
    bool ok2 = i2cWriteBytes(BQ27220_COMMAND_CONTROL, key2, 2);
    delay(10);
    return ok1 && ok2;
}

bool BQ27220::seal() {
    bool ok = writeCtrlWord(BQ27220_CONTROL_SEALED);
    delay(10);
    return ok;
}
