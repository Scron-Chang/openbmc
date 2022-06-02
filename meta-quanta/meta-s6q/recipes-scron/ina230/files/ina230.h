#ifndef __INA230_H__
#define __INA230_H__

#include <stdint.h>
#include <stdbool.h>

enum INA230_OFFSET {
    INA230_CFG_OFFSET = 0x00,
    INA230_PWR_OFFSET = 0x03,
    INA230_CUR_OFFSET = 0x04,
    INA230_CAL_OFFSET = 0x05,
};

typedef struct _ina230_init_arg {
    /* value to set configuration register */
    union {
        uint16_t value;
        struct {
            uint16_t MODE : 3;
            uint16_t VSH_CT : 3;
            uint16_t VBUS_CT : 3;
            uint16_t AVG : 3;
            uint16_t reserved : 3;
            uint16_t RST : 1;
        };
    } config;

    /* Shunt resistor value */
    double r_shunt;

    /* Expected maximum current */
    double i_max;

    /* Initialize function will set following arguments, no need to give value */
    bool is_init;

} ina230_init_arg;

extern uint8_t ina230_init(int bus, int address);

#endif //__INA230_H__
