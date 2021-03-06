/**
 * @file    Concurrency_Demo.cpp
 *
 * @author  David Zemon
 */

// Includes
#include "Concurrency_Demo.h"

volatile int lock = locknew();

// NOTE!!! The direction of a pin (or port) is not a member variable of the pin
// or port. Therefore, if you're going to declare a pin (or port) in one cog
// and use it in another, you MUST set the direction in the new port. Look at
// how g_pin2.set_dir() is called within blinkAnLEDSome() instead of the
// direction being set in the constructor
PropWare::Pin g_pin1(PropWare::Port::P16, PropWare::Port::OUT);
PropWare::Pin g_pin2(PropWare::Port::P17);
const int g_someStackSpace = 64;

// Main function
int main () {

    cog_run((void (*) (void *)) &blinkAnLEDSome, g_someStackSpace);

    waitcnt(20*MICROSECOND + CNT);

    while(lockset(lock));

    for (int i = 0; i < 40; ++i) {
        waitcnt(50 * MILLISECOND + CNT);
        g_pin1.toggle();
    }

    lockclr(lock);

    return 0;
}

void blinkAnLEDSome (void) {
    while(lockset(lock));

    g_pin2.set_dir(PropWare::Port::OUT);

    for (int i = 0; i < 40; ++i) {
        waitcnt(50 * MILLISECOND + CNT);
        g_pin2.toggle();
    }

    lockclr(lock);
}
