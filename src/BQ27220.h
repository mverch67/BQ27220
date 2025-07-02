#pragma once

#include <Arduino.h>
#include <Wire.h>

class BQ27220
{
public:
    enum CURR_MODE {
        CURR_RAW,
        CURR_INSTANT,
        CURR_STANDBY,
        CURR_CHARGING,
        CURR_AVERAGE,
    };
    
    enum VOLT_MODE {
        VOLT,
        VOLT_CHARGING,
        VOLT_RAW
    };
    
    union battery_state {
        struct __st {
            uint16_t DSG : 1;
            uint16_t SYSDWN : 1;
            uint16_t TDA : 1;
            uint16_t BATTPRES : 1;
            uint16_t AUTH_GD : 1;
            uint16_t OCVGD : 1;
            uint16_t TCA : 1;
            uint16_t RSVD : 1;
            uint16_t CHGING : 1;
            uint16_t FC : 1;
            uint16_t OTD : 1;
            uint16_t OTC : 1;
            uint16_t SLEEP : 1;
            uint16_t OCVFALL : 1;
            uint16_t OCVCOMP : 1;
            uint16_t FD : 1;
        } st;
        uint16_t full;
    };
    

    BQ27220();
    bool init(TwoWire *wire, int sda, int scl, bool initWire = false);
    uint16_t getTemp();
    uint16_t getBatterySt(void);
    bool getIsCharging(void);
    uint16_t getRemainCap();
    uint16_t getFullChargeCap(void);
    uint16_t getChargePcnt(void);
    uint16_t getVolt(VOLT_MODE type);
    int16_t getCurr(CURR_MODE type);
    uint16_t getId();

private:
    TwoWire *wire;
    uint8_t addr;
    int scl;
    int sda;
    union battery_state bat_st;
    bool i2cReadBytes(uint8_t subAddress, uint8_t *dest, uint8_t count);
    bool i2cWriteBytes(uint8_t subAddress, uint8_t *src, uint8_t count);
    uint16_t readWord(uint16_t subAddress);
    uint16_t readCtrlWord(uint16_t fun);
};