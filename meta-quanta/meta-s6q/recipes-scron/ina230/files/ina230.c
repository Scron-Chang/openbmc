#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "ina230.h"
#include "main.h"

static double cur_lsb = 0.0;
static int file;

static ina230_init_arg ina230_init_args[] = {
    [0] = {
        .is_init = false,
        .config =
            {
                .MODE = 0b111, // Measure voltage of shunt resistor and bus(default).
                .VSH_CT = 0b100, // The Vshunt conversion time is 1.1ms(default).
                .VBUS_CT = 0b100, // The Vbus conversion time is 1.1ms(default).
                .AVG = 0b000, // Average number is 1(default).
            },
        .r_shunt = 0.001,
        .i_max = 6.0 },
};

uint8_t ina230_read(uint8_t reg, int *reading)
{
    double val = 0.0;
    uint32_t res = 0;

    if (ina230_init_args[0].is_init == false)
    {
        return FAILURE;
    }

    res = i2c_smbus_read_word_data(file, reg);
    if (res < 0) {
        return FAILURE;
    }

    printf("ina230_read<%.2X>: %.8X", reg, res);

    //switch (reg) {
    //    case INA230_PWR_OFFSET:
    //        // The power LSB is internally programmed to equal 25 times the current LSB.
    //        val = (double)(reg_val)*cur_lsb * 25.0;
    //        break;
    //    case INA230_CUR_OFFSET:
    //        val = (double)(reg_val)*cur_lsb;
    //        break;
    //    default:
    //        return SENSOR_NOT_FOUND;
    //}

    //sensor_val *sval = (sensor_val *)reading;
    //sval->integer = (int)val & 0xFFFF;
    //sval->fraction = (val - sval->integer) * 1000;
    return SUCCESS;
}

uint8_t ina230_init(int bus, int address)
{
    uint16_t calibration = 0;
    ina230_init_arg *init_args = ina230_init_args;

    char filename[20];
    char write_buf[10];

    snprintf(filename, 19, "/dev/i2c-%d", bus);
    file = open(filename, O_RDWR);
    if (file < 0) {
        printf("Can't open file: %s\n", filename);
        return FAILURE;
    }

    if (ioctl(file, I2C_SLAVE, address) < 0) {
        printf("ioctl err: %s\n", filename);
        return FAILURE;
    }

    if (init_args->r_shunt <= 0.0 || init_args->i_max <= 0.0) {
        printf("<error> INA230 has invalid initail arguments\n");
        return FAILURE;
    }

    /* Calibration = 0.00512 / (cur_lsb * r_shunt)
     * - 0.00512 : The fixed value in ina230 used to ensure scaling is maintained properly.
     * - cur_lsb : Maximum Expected Current(i_max) / 2^15.
     * - r_shunt : Value of the shunt resistor.
     * Ref: https://www.ti.com/product/INA230
     */
    cur_lsb = init_args->i_max / 32768.0;
    calibration = (uint16_t)(0.00512 / (cur_lsb * init_args->r_shunt));

    write_buf[0] = INA230_CFG_OFFSET;
    write_buf[1] = init_args->config.value & 0xFF;
    write_buf[2] = (init_args->config.value >> 8) & 0xFF;

    if (write(file, write_buf, 3) != 3) {
        printf("<error> INA230 initial failed while i2c writing\n");
        return FAILURE;
    }

    memset(write_buf, 0, sizeof(write_buf));
    write_buf[0] = INA230_CAL_OFFSET;

    if (calibration & 0x8000) {
        // The size of calibration is 16 bits, and the MSB is unused.
        printf("<warning> INA230 the calibration register overflowed (0x%.2X)\n",
               calibration);
        calibration = calibration & 0x7FFF;
    }

    write_buf[1] = calibration & 0xFF;
    write_buf[2] = (calibration >> 8) & 0xFF;

    if (write(file, write_buf, 3) != 3) {
        printf("<error> INA230 initial failed while i2c writing\n");
        return FAILURE;
    }

    printf(
        "Check the config: %.8X, and the cal: %.8X\n"
        "\tcur_lsb= %.1f, r_shunt= %.1f, i_max= %.f\n",
        i2c_smbus_read_word_data(file, INA230_CFG_OFFSET),
        i2c_smbus_read_word_data(file, INA230_CAL_OFFSET),
        cur_lsb, init_args->r_shunt, init_args->i_max
    );

    return SUCCESS;
}
