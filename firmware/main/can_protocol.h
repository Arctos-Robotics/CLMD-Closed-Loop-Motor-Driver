#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include "esp_err.h"
#include "driver/twai.h"

#define CAN_MODE_ENABLE         0xF0
#define CAN_MODE_DISABLE        0xF1
#define CAN_MODE_SET_ZERO       0xF2
#define CAN_MODE_QUERY_STATUS   0xF3
#define CAN_MODE_MOVE_RELATIVE  0xF4
#define CAN_MODE_MOVE_ABSOLUTE  0xF5
#define CAN_MODE_EMERGENCY_STOP 0xF6

esp_err_t can_init(void);
void can_handle_message(const twai_message_t *msg);
void can_send_status_response(void);
void can_task(void *pvParameters);

#endif
