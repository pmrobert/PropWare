/**
 * @file        pin.h
 *
 * @project     PropWare
 *
 * @author      David Zemon
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

#ifndef PROPWARE_PIN_H_
#define PROPWARE_PIN_H_

#include <PropWare/PropWare.h>
#include <PropWare/port.h>

namespace PropWare {

/**
 * @brief   Utility class to handle general purpose I/O pins
 */
class Pin: public PropWare::Port {
    public:
        /** Number of milliseconds to delay during debounce */
        static const uint8_t DEBOUNCE_DELAY = 3;

    public:
        /**
         * @brief   Public no-arg constructor - useful when you want a member
         *          variable in a class but don't want to require the pin be
         *          passed into the constructor
         */
        Pin () :
                Port() {
        }

        /**
         * @brief       Create a Pin variable
         *
         * @param[in]   mask    Bit-mask of pin; One of PropWare::Pin::Mask
         */
        Pin (const Pin::Mask mask) :
                PropWare::Port((uint32_t) mask) {
        }

        /**
         * @brief       Create a Pin variable
         *
         * @param[in]   mask        Bit-mask of pin; One of PropWare::Pin::Mask
         * @param[in]   direction   Direction to initialize pin; One of
         *                          PropWare::Pin::Dir
         */
        Pin (const Pin::Mask mask, const Pin::Dir direction) :
                PropWare::Port((uint32_t) mask, direction) {
        }

        /**
         * @brief       Create a Pin variable
         *
         * @param[in]   pinNum  0-indexed integer value representing pin-number
         */
        Pin (const uint8_t pinNum);

        /**
         * @brief       Create a Pin variable
         *
         * @param[in]   pinNum      0-indexed integer value representing
         *                          pin-number
         * @param[in]   direction   Direction to initialize pin; One of
         *                          PropWare::Pin::Dir
         */
        Pin (const uint8_t pinNum, const Pin::Dir direction);

        /**
         * @see PropWare::Port::set_mask()
         */
        void set_mask (const PropWare::Port::Mask mask);

        /**
         * @brief   Read the value from a single pin and return its state
         *
         * @return  True if the pin is high, False if the pin is low
         */
        bool read () const;

        /**
         * @brief   Allow easy switch-press detection of any pin; Includes
         *          de-bounce protection
         *
         * @return  Returns 1 or 0 depending on whether the switch was pressed
         */
        bool is_switch_low () const;

        /**
         * @brief       Allow easy switch-press detection of any pin; Includes
         *              de-bounce protection
         *
         * @param[in]   debounceDelayInMillis   Set the de-bounce delay in units
         *                                      of milliseconds
         *
         * @return      Returns 1 or 0 depending on whether the switch was
         *              pressed
         */
        bool is_switch_low (const uint16_t debounceDelayInMillis) const;

    public:
        /**
         * @brief   Copy one pin object into another; Only copies pin mask, not
         *          I/O direction
         */
        PropWare::Pin* operator= (const PropWare::Pin &rhs);

        /**
         * @Brief   Compare the pin mask of two pin objects. Does not compare
         *          I/O direction
         */
        bool operator== (const PropWare::Pin &rhs);

    private:
        /****************************************
         *** Nonsensical functions for a pin ***
         ****************************************/
        /**
         * Hide from user - should not be accessible within PropWare::Pin
         */
        void set_mask (const uint32_t mask) {
        }

        /**
         * Hide from user - should not be accessible within PropWare::Pin
         */
        void add_pins (const uint32_t mask) {
        }

        /**
         * Hide from user - should not be accessible within PropWare::Pin
         */
        uint32_t read_fast () const {
            return 0;
        }
};

}

#endif /* PROPWARE_PIN_H_ */