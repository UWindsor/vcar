#include <linux/can.h>

struct can_frame *current;

void can_send_signal(struct can_frame *frame);

void can_accept_signal(struct can_frame *frame);

void can_accept_signal_rtr(struct can_frame *frame);
