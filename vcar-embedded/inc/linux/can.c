//
// Created by dschana on 4/19/20.
//

#include <linux/can.h>
#include <stdint.h>
#include <stdio.h>

uint64_t getDataAsUint64FromCanFrame(const struct can_frame * cf) {
    uint64_t data = cf->data[0];

    for (int i = 1; i < cf->can_dlc; i++) {
        data = (data << 8) + cf->data[i];
    }

    return data;
}

struct can_frame make_can_frame() {
    struct can_frame cf;
    cf.getDataAsUint64 = getDataAsUint64FromCanFrame;

    return cf;
}