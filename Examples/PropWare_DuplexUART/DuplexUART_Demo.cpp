/**
 * @file    DuplexUART_Demo.cpp
 *
 * @author  David Zemon
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "DuplexUART_Demo.h"

// Create an easy-to-test number pattern - useful when testing with a logic
// analyzer
char g_numberPattern[] = {
        0x01,
        0x02,
        0x03,
        0x45,
        0xe5,
        0xaa,
        0xff,
        0x80,
        0x00 };

// Create the test string - useful when testing with a terminal
static char g_string[] = "Hello world! This is David Zemon. I'm here to rescue "
        "you! But I don't know if this will actually work :(";
static const uint32_t BAUD_RATE = 740750;
static const PropWare::Port::Mask TX_PIN = PropWare::Port::P16;
static const PropWare::UART::Parity PARITY = PropWare::UART::ODD_PARITY;
volatile uint8_t g_stringLength;

/**
 * @brief   Write "Hello world!\n" out via SPI protocol and receive an echo
 */
int main () {
    static uint32_t threadStack[STACK_SIZE];
    static _thread_state_t threadData;
    PropWare::SimplexUART uart(TX_PIN);
    uart.set_baud_rate(BAUD_RATE);
    uart.set_parity(PARITY);

    g_stringLength = (uint8_t) (strlen(g_string) + 1);

    uint8_t cog = (uint8_t) _start_cog_thread(threadStack + STACK_SIZE, receiveSilently,
                (void *) NULL, &threadData);

    while (1) {
        waitcnt(500 * MILLISECOND + CNT);
        uart.send_array(g_string, g_stringLength);
    }
}

void receiveSilently (void *arg) {
    char buffer[256];

    PropWare::HalfDuplexUART uart(PropWare::Port::P17);
    uart.set_baud_rate(BAUD_RATE);
    uart.set_parity(PARITY);

    printf("Ready to receive!!!\n");

    while (1) {
        uart.receive_array(buffer, g_stringLength);

        printf("Data: '%s'\n", buffer);
    }
}

void error (const PropWare::ErrorCode err) {
    PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Pin::OUT);

    printf("Unknown error %u\n", err);

    while (1) {
        debugLEDs.write((uint32_t) err);
        waitcnt(100*MILLISECOND);
        debugLEDs.write(0);
        waitcnt(100*MILLISECOND);
    }
}
