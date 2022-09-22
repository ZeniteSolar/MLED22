/**
 * @file machine.h
 *
 * @defgroup MACHINE State Machine Module
 *
 * @brief Implements the main state machine of the system.
 *
 */

#ifndef MACHINE_H
#define MACHINE_H

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "conf.h"

// Equations for mode 2 (CTC with TOP OCR2A)
// Note the resolution. For example.. at 150hz, ICR1 = PWM_TOP = 159, so it
#define MACHINE_TIMER_TOP ((F_CPU / (2 * MACHINE_TIMER_PRESCALER)) / (MACHINE_TIMER_FREQUENCY)-1)

#ifdef ADC_ON
#include "adc.h"
#endif
#ifdef USART_ON
#include "usart.h"
#endif
#include "dbg_vrb.h"
#ifdef CAN_ON
#include "can.h"
#include "can_app.h"
extern const uint8_t can_filter[];
#endif

typedef enum state_machine
{
    STATE_INITIALIZING,
    STATE_IDLE,
    STATE_RUNNING,
    STATE_ERROR,
    STATE_RESET,
} state_machine_t;

typedef union system_flags
{
    struct
    {
        // MIC
        uint16_t boat_switch_on : 1;
        uint16_t motor_switch_on : 1;
        uint16_t pot_zero : 1;
        uint16_t dms_switch : 1;
        uint16_t reverse_switch : 1;
        // MCS
        uint16_t boat_on : 1;
        uint16_t boat_charging : 1;
        // MAM
        uint16_t motor_running : 1;
        uint16_t motor_idle : 1;
        uint16_t motor_waiting_contactor : 1;
        uint16_t motor_error : 1;
        // MCB
        uint16_t mcbs_ok : 1;
        
    };
    uint16_t all__;
} system_flags_t;

typedef union pump_flags
{
    struct
    {
        uint8_t pump1_on : 1;
        uint8_t pump2_on : 1;
    };
    uint8_t all__;
} pump_flags_t;

typedef union error_flags
{
    struct
    {
        uint8_t no_canbus : 1;
    };
    uint8_t all;
} error_flags_t;


// machine checks
void check_buffers(void);
void reset_measurements(void);
void average_measurements(void);

// debug functions
void print_configurations(void);
void print_system_flags(void);
void print_error_flags(void);

// machine tasks
void task_initializing(void);
void task_idle(void);
void task_running(void);
void task_error(void);
void task_reset(void);
void task_waiting_reset(void);

// the machine itself
void set_machine_initial_state(void);
void machine_init(void);
void machine_run(void);
void set_state_error(void);
void set_state_initializing(void);
void set_state_idle(void);
void set_state_running(void);
void set_state_reset(void);
void set_state_waiting_reset(void);

// input functions
void read_switches(void);
void read_potentiometers(void);
void read_boat_on(void);
void read_pump_switches(void);
void reset_switches(void);
void acumulate_potentiometers(void);
void average_potentiometers(void);

void buzzer(uint8_t buzzer_frequency, uint8_t buzzer_rhythm_on, uint8_t buzzer_rhythm_off);

// machine variables
extern volatile state_machine_t state_machine;
extern volatile pump_flags_t pump_flags;
extern volatile system_flags_t system_flags;
extern volatile error_flags_t error_flags;
extern volatile uint16_t charge_count_error;
extern volatile uint8_t relay_clk;
extern volatile uint8_t first_boat_off;
extern volatile uint8_t machine_clk;
extern volatile uint8_t machine_clk_divider;
extern volatile uint8_t total_errors; // Contagem de ERROS
extern volatile uint16_t charge_count_error;
extern volatile uint8_t reset_clk;

// other variables
extern volatile uint8_t led_clk_div;

#endif /* ifndef MACHINE_H */
