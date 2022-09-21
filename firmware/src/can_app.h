/**
 * @file can_app.h
 *
 * @defgroup CANAPP    Application Module for canbus
 *
 * @brief The application layer module for canbus
 *
 */

#ifndef CAN_APP_H
#define CAN_APP_H

#include "conf.h"
#include "dbg_vrb.h"
#include "../lib/bit_utils.h"
#include "can.h"
#include "can_ids.h"
#include "machine.h"
#include "usart.h"

void can_app_task(void);

void check_can(void);

#ifdef CAN_ON
#define CAN_APP_SEND_STATE_CLK_DIV CAN_APP_SEND_STATE_FREQ
#define CAN_APP_SEND_MOTOR_CLK_DIV CAN_APP_SEND_MOTOR_FREQ
#define CAN_APP_SEND_BOAT_CLK_DIV CAN_APP_SEND_BOAT_FREQ
#define CAN_APP_SEND_PUMPS_CLK_DIV CAN_APP_SEND_PUMPS_FREQ
#define CAN_APP_SEND_ADC_CLK_DIV CAN_APP_SEND_ADC_FREQ
#else
#define CAN_APP_SEND_STATE_CLK_DIV 1
#define CAN_APP_SEND_ADC_CLK_DIV 1
#endif

typedef enum contactor_request
{
    CONTACTOR_REQUEST_TURN_OFF,
    CONTACTOR_REQUEST_SET_FORWARD,
    CONTACTOR_REQUEST_SET_REVERSE,
    CONTACTOR_REQUEST_UNKNOWN = 0xFF,
} mam_contactor_request_t;

typedef enum mam_state_machine{
    MAM_STATE_INITIALIZING,
    MAM_STATE_CONTACTOR,
    MAM_STATE_IDLE,
    MAM_STATE_RUNNING,
    MAM_STATE_ERROR,
} mam_state_machine_t;

extern uint32_t can_app_send_state_clk_div;
extern uint32_t can_app_send_motor_clk_div;
extern uint32_t can_app_send_boat_clk_div;
extern uint32_t can_app_send_pumps_clk_div;

#endif /* ifndef CAN_APP_H */
