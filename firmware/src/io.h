/**
 * @file io.h
 *
 * @defgroup IO IOs pins management
 *
 * @brief  IO IOs pins management
 *
 */

#ifndef IO_H
#define IO_H

typedef enum
{
    FALSE,
    TRUE
} logic_level_t

    typedef enum
{
    INPUT,
    OUTPUT
} impedance_t

    typedef struct
{
    const uint8_t ddr;
    const uint8_t pin;
    const uint8_t port;
    const uint8_t pos;
} io_t;

typedef struct
{
    io_t io;
    logic_level_t pull_up;
    output_t impedance;
    const logic_level_t default_value;
} pin_t;

inline void gpio_config(pin_t *pin)
{
    set_bit(pin->io.ddr, pin->pull_up);
    set_bit(pin->io.ddr, pin->impedance);
    set_bit(pin->io.ddr, pin->pull_up);


}

inline void gpio_set(io_t *io, logic_level_t logic_level)
{

}

inline logic_level_t gpio_read(io_t *io)
{
}

#endif