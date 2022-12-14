#include "machine.h"

volatile state_machine_t state_machine;
volatile pump_flags_t pump_flags;
volatile system_flags_t system_flags;
volatile error_flags_t error_flags;
volatile uint16_t charge_count_error;
volatile uint8_t relay_clk;
volatile uint8_t first_boat_off;
volatile uint8_t machine_clk;
volatile uint8_t machine_clk_divider;
volatile uint8_t total_errors; // Contagem de ERROS
volatile uint16_t charge_count_error;
volatile uint8_t reset_clk;

volatile uint8_t led_clk_div;

/**
 * @brief
 */
void machine_init(void)
{
    // clr_bit(PRR0, PRTIM2);                          // Activates clock

    // MODE 2 -> CTC with TOP on OCR1
    TCCR2A = (1 << WGM21) | (0 << WGM20)      // mode 2
             | (0 << COM2B1) | (0 << COM2B0)  // do nothing
             | (0 << COM2A1) | (0 << COM2A0); // do nothing

    TCCR2B =
#if MACHINE_TIMER_PRESCALER == 1
        (0 << CS22) | (0 << CS21) | (1 << CS20) // Prescaler N=1
#elif MACHINE_TIMER_PRESCALER == 8
        (0 << CS22) | (1 << CS21) | (0 << CS20) // Prescaler N=8
#elif MACHINE_TIMER_PRESCALER == 32
        (0 << CS22) | (1 << CS21) | (1 << CS20) // Prescaler N=32
#elif MACHINE_TIMER_PRESCALER == 64
        (1 << CS22) | (0 << CS21) | (0 << CS20) // Prescaler N=64
#elif MACHINE_TIMER_PRESCALER == 128
        (1 << CS22) | (0 << CS21) | (1 << CS20) // Prescaler N=128
#elif MACHINE_TIMER_PRESCALER == 256
        (1 << CS22) | (1 << CS21) | (0 << CS20) // Prescaler N=256
#elif MACHINE_TIMER_PRESCALER == 1024
        (1 << CS22) | (1 << CS21) | (1 << CS20) // Prescaler N=1024
#else
        0
#endif
        | (0 << WGM22); // mode 2

    OCR2A = MACHINE_TIMER_TOP; // OCR2A = TOP = fcpu/(N*2*f) -1

    TIMSK2 |= (1 << OCIE2A); // Activates interruption

    set_machine_initial_state();
    set_state_initializing();
}

/**
 * @brief set machine initial state
 */
inline void set_machine_initial_state(void)
{
    error_flags.all = 0;
    machine_clk = machine_clk_divider = led_clk_div = 0;
}

/**
 * @brief set error state
 */
inline void set_state_error(void)
{
    VERBOSE_MSG_MACHINE(usart_send_string("\n>>>STATE ERROR\n"));
    state_machine = STATE_ERROR;
}

/**
 * @brief set initializing state
 */
inline void set_state_initializing(void)
{
    VERBOSE_MSG_MACHINE(usart_send_string("\n>>>INITIALIZING STATE\n"));
    state_machine = STATE_INITIALIZING;
}

/**
 * @brief set idle state
 */
inline void set_state_idle(void)
{
    VERBOSE_MSG_MACHINE(usart_send_string("\n>>>IDLE STATE\n"));
    state_machine = STATE_IDLE;
}

/**
 * @brief set running state
 */
inline void set_state_running(void)
{
    VERBOSE_MSG_MACHINE(usart_send_string("\n>>>RUNNING STATE\n"));
    state_machine = STATE_RUNNING;
}

/**
 * @brief set reset state
 */
inline void set_state_reset(void)
{
    VERBOSE_MSG_MACHINE(usart_send_string("\n>>>RESET STATE\n"));
    state_machine = STATE_RESET;
}

/**
 * @brief prints the system flags
 */
inline void print_system_flags(void)
{
}

inline void task_initializing(void)
{
#ifdef LED_ON
    set_led(LED1);
#endif

    set_machine_initial_state();

    VERBOSE_MSG_INIT(usart_send_string("System initialized without errors.\n"));
    set_state_idle();
}

/**
 * @brief waits for commands while checking the system
 */
inline void task_idle(void)
{
    set_state_running();
}



/**
 * @brief running task checks the system and apply the control action to pwm.
 */
inline void task_running(void)
{
    static uint16_t clk = 0;

    if (!(clk % 2))
        print_infos();

    SET_LED(CTRL_SWITCHES_PORT, BOAT_ON_SWITCH, system_flags.boat_switch_on);
    SET_LED(CTRL_SWITCHES_PORT, MOTOR_ON_SWITCH, system_flags.motor_switch_on);
    SET_LED(DMS_PORT, DMS, system_flags.dms_switch);
    SET_LED(REVERSE_SWITCH_PORT, REVERSE_SWITCH, system_flags.reverse_switch);
    SET_LED(POT_ZERO_PORT, POT_ZERO, system_flags.pot_zero);

    if (system_flags.boat_charging){
        if (!(clk % 10))
            cpl_bit(CTRL_SWITCHES_PORT, BOAT_ON_OK);
    }

    if (system_flags.motor_idle){
        if (!(clk % 30))
            cpl_bit(MOTOR_ON_OK_PORT, MOTOR_ON_OK);
    }else if (system_flags.motor_waiting_contactor){
        if (!(clk % 40))
            cpl_bit(MOTOR_ON_OK_PORT, MOTOR_ON_OK);
    }else if (system_flags.motor_running){
        if (!(clk % 50))
            cpl_bit(MOTOR_ON_OK_PORT, MOTOR_ON_OK);
    }

    if (system_flags.boat_on)
        set_bit(CTRL_SWITCHES_PORT, BOAT_ON_OK);




    clk++;
}

/**
 * @brief error task checks the system and tries to medicine it.
 */
inline void task_error(void)
{
#ifdef LED_ON
    if (led_clk_div++ >= 5)
    {
        set_led(LED1);
        led_clk_div = 0;
    }
#endif

    set_state_initializing();

    total_errors++; // incrementa a contagem de erros
    VERBOSE_MSG_ERROR(usart_send_string("The error code is: "));
    VERBOSE_MSG_ERROR(usart_send_uint16(error_flags.all));
    VERBOSE_MSG_ERROR(usart_send_char('\n'));

    if (!error_flags.all)
        VERBOSE_MSG_ERROR(usart_send_string("\t - Oh no, it was some unknown error.\n"));

    VERBOSE_MSG_ERROR(usart_send_string("The error level is: "));
    VERBOSE_MSG_ERROR(usart_send_uint16(total_errors));
    VERBOSE_MSG_ERROR(usart_send_char('\n'));

    if (total_errors < 2)
    {
        VERBOSE_MSG_ERROR(usart_send_string("I will reset the machine state.\n"));
    }
    if (total_errors >= 20)
    {
        VERBOSE_MSG_ERROR(usart_send_string("The watchdog will reset the whole system.\n"));
        set_state_reset();
    }

#ifdef LED_ON
    cpl_led(LED1);
#endif
}

/**
 * @brief reset error task just freezes the processor and waits for watchdog
 */
inline void task_reset(void)
{
#ifndef WATCHDOG_ON
    // wdt_init();
#endif

    cli(); // disable interrupts

    VERBOSE_MSG_ERROR(usart_send_string("WAITING FOR A RESET!\n"));
    for (;;)
    {
    };
}

void print_infos(void)
{

    VERBOSE_MSG_MACHINE(usart_send_string("\nMIC: "));
    VERBOSE_MSG_MACHINE(usart_send_string(" bo_sw: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.boat_switch_on + '0'));
    VERBOSE_MSG_MACHINE(usart_send_string(" mo_sw: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.motor_switch_on + '0'));
    VERBOSE_MSG_MACHINE(usart_send_string(" pot_0: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.pot_zero + '0'));
    VERBOSE_MSG_MACHINE(usart_send_string(" dms_sw: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.dms_switch + '0'));
    VERBOSE_MSG_MACHINE(usart_send_string(" re_sw: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.reverse_switch + '0'));
    VERBOSE_MSG_MACHINE(usart_send_string(" | MCS: "));
    VERBOSE_MSG_MACHINE(usart_send_string(" bo_on: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.boat_on + '0'));
    VERBOSE_MSG_MACHINE(usart_send_string(" bo_ch: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.boat_charging + '0'));
    VERBOSE_MSG_MACHINE(usart_send_string(" | MAM: "));
    VERBOSE_MSG_MACHINE(usart_send_string(" mo_running: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.motor_running + '0'));
    VERBOSE_MSG_MACHINE(usart_send_string(" mo_idle: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.motor_idle + '0'));
    VERBOSE_MSG_MACHINE(usart_send_string(" mo_wa_co: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.motor_waiting_contactor + '0'));
    VERBOSE_MSG_MACHINE(usart_send_string(" mo_error: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.motor_error + '0'));
    VERBOSE_MSG_MACHINE(usart_send_string(" | MCB: "));
    VERBOSE_MSG_MACHINE(usart_send_string(" mcbs_ok: "));
    VERBOSE_MSG_MACHINE(usart_send_char(system_flags.mcbs_ok + '0'));
}

/**
 * @brief this is the machine state itself.
 */
inline void machine_run(void)
{
    // print_infos();

    // print_system_flags();

    if (machine_clk)
    {
        machine_clk = 0;
#ifdef ADC_ON
        if (adc.ready)
        {
            adc.ready = 0;

            if (error_flags.all)
            {
                print_system_flags();
                print_infos();
                set_state_error();
            }

            switch (state_machine)
            {
            case STATE_INITIALIZING:
                task_initializing();

                break;
            case STATE_IDLE:
                task_idle();
#ifdef CAN_ON
                can_app_task();
#endif /* CAN_ON */
                break;
            case STATE_RUNNING:
                task_running();
#ifdef CAN_ON
                can_app_task();
#endif /* CAN_ON */

                break;
            case STATE_ERROR:
                task_error();

            case STATE_RESET:
            default:
                task_reset();
                break;
            }
        }
#endif /* ADC_ON */
    }
}

/**
 * @brief ISR para a????es de controle
 */
ISR(TIMER2_COMPA_vect)
{
    if (machine_clk_divider++ == MACHINE_CLK_DIVIDER_VALUE)
    {
        /*if(machine_clk){
            for(;;){
                pwm_reset();
                VERBOSE_MSG_ERROR(if(machine_clk) usart_send_string("\nERROR: CLOCK CONFLICT!!!\n"));
            }
        }*/
        machine_clk = 1;
        machine_clk_divider = 0;
    }
}
