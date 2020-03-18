#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/can.h>

#include "../logger.h"

#include "nodes.h"

/** nodes header */
#include "door.h"
#include "pump.h"
 
struct can_frame *current;

void can_send_signal(struct can_frame *frame)
{
  
}

void can_accept_signal(struct can_frame *frame)
{
  uint8_t i;
  Log("New frame recieved ...");

  if (frame->can_id == NODE_SIGNAL_DOOR_MODE) 
    node_door_mode(); 

  if (frame->can_id == NODE_SIGNAL_DOOR_UNLC)
    node_door_unlock();

  if (frame->can_id == NODE_SIGNAL_DOOR_LOCK)
    node_door_lock();

  if (frame->can_id == NODE_SIGNAL_DOOR_WNDW)
    node_door_mirror();

  if (frame->can_id == NODE_SIGNAL_DOOR_MIRR)
    node_door_mirror();

  if (frame->can_id == NODE_SIGNAL_EHPS_MODE)
    node_ehps_mode();

  printf("\n");
}

void can_accept_signal_rtr(struct can_frame *frame)
{
  Log("New RTR frame recieved ...");


}
