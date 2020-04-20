//
// Created by dschana on 4/19/20.
//

#include <linux/can.h>
#include <stdint.h>

uint64_t getDataAsUint64FromCanFrame(struct can_frame cf) {
    uint64_t data = cf.data[0];

    for (int i = 1; i < cf.can_dlc; i++) {
        data = (data << 8) + cf.data[i];
    }

    return data;
}