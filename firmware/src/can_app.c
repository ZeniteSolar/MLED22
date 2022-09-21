#include "../lib/CAN_PARSER/can_parser.h"
#include "can_app.h"
#include <string.h>

/**
 * @brief Manages the canbus application protocol
 */
inline void can_app_task(void)
{
    check_can();
}

void can_parse_mic_motor(can_msg_t *msg)
{
    can_mic19_motor_msg_t *mic_motor = (can_mic19_motor_msg_t *)msg->raw;

    system_flags.motor_switch_on = mic_motor->motor.motor_on;
    system_flags.dms_switch = mic_motor->motor.dms_on;
    system_flags.reverse_switch = mic_motor->motor.reverse;
}

void can_parse_mic_mcs(can_msg_t *msg)
{
    can_mic19_mcs_msg_t *mic_mcs = (can_mic19_mcs_msg_t *)msg->raw;

    system_flags.boat_switch_on = mic_mcs->boat_on.boat_on;
}

void can_parse_mam_state(can_msg_t *msg)
{
    can_mam19_state_msg_t *mam_state = (can_mam19_state_msg_t *)msg->raw;

    switch (mam_state->state)
    {
    case MAM_STATE_INITIALIZING:
        break;
    case MAM_STATE_IDLE:
        system_flags.motor_idle = 1;
        system_flags.motor_running = 0;
        system_flags.motor_waiting_contactor = 0;
        system_flags.motor_error = 0;
        break;
    case MAM_STATE_RUNNING:
        system_flags.motor_idle = 0;
        system_flags.motor_running = 1;
        system_flags.motor_waiting_contactor = 0;
        system_flags.motor_error = 0;
        break;
    case MAM_STATE_CONTACTOR:
        system_flags.motor_idle = 0;
        system_flags.motor_running = 0;
        system_flags.motor_waiting_contactor = 1;
        system_flags.motor_error = 0;
        break;
    case MAM_STATE_ERROR:
        system_flags.motor_idle = 0;
        system_flags.motor_running = 0;
        system_flags.motor_waiting_contactor = 0;
        system_flags.motor_error = 1;
        break;
        
    }
}

void can_parse_mam_motor(can_msg_t *msg)
{
    can_mam19_motor_msg_t *mam_motor = (can_mam19_motor_msg_t *)msg->raw;

    system_flags.pot_zero = (mam_motor->d < 6) ? 1 : 0;
}

void can_parse_mam_contactor(can_msg_t *msg)
{
    can_mam19_contactor_msg_t *mam_contactor = (can_mam19_contactor_msg_t *)msg->raw;

    // switch (mam_contactor->request)
    // {
    // case CONTACTOR_REQUEST_TURN_OFF:
    // case CONTACTOR_REQUEST_SET_FORWARD:
    // case CONTACTOR_REQUEST_SET_REVERSE:
    //     system_flags.motor_waiting_contactor = 1;
    //     break;
    // default:
    //     break;
    // }
}

void can_parse_mcs_start_stages(can_msg_t *msg)
{
    can_mcs19_start_stages_msg_t *mcs_start = (can_mcs19_start_stages_msg_t *)msg->raw;

    system_flags.boat_charging = mcs_start->charge_relay.charge_relay;
    system_flags.boat_on = mcs_start->main_relay.main_relay;
}

void can_handle_timeout(uint8_t signature)
{
}

/**
 * @brief Manages to receive and extract specific messages from canbus
 */
inline void check_can(void)
{
    CAN_REGISTER_TOPICS(mic,
                        {CAN_MSG_MIC19_MOTOR_ID, &can_parse_mic_motor},
                        {CAN_MSG_MIC19_MCS_ID, &can_parse_mic_mcs});

    CAN_REGISTER_TOPICS(mam,
                        {CAN_MSG_MAM19_STATE_ID, &can_parse_mam_state},
                        {CAN_MSG_MAM19_MOTOR_ID, &can_parse_mam_motor},
                        {CAN_MSG_MAM19_CONTACTOR_ID, &can_parse_mam_contactor});
    CAN_REGISTER_TOPICS(mcs, 
                        {CAN_MSG_MCS19_START_STAGES_ID, &can_parse_mcs_start_stages});  
                    

    CAN_REGISTER_MODULES(mled_rx, 100,
                         {CAN_SIGNATURE_MIC19, &CAN_TOPICS_NAME(mic), 0},
                         {CAN_SIGNATURE_MAM19, &CAN_TOPICS_NAME(mam), 0},
                         {CAN_SIGNATURE_MCS19, &CAN_TOPICS_NAME(mcs), 0});

    if (can_check_message())
    {
        can_t msg_temp;
        if (can_get_message(&msg_temp))
        {
            can_msg_t msg;
            memcpy(msg.raw, msg_temp.data, sizeof(msg.raw));
            msg.id = msg_temp.id;
            can_parser(&CAN_PARSER_NAME(mled_rx), &msg);
        }
    }
}
