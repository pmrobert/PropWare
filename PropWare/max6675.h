/**
 * @file    max6675.h
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

#ifndef PROPWARE_MAX6675_H_
#define PROPWARE_MAX6675_H_

#include <PropWare/PropWare.h>
#include <PropWare/spi.h>

namespace PropWare {

/**
 * @brief   K-type thermocouple amplifier driver using SPI communication for the
 *          Parallax Propeller
 */
class MAX6675 {
    public:
        /**
         * @brief       Construction requires an instance of the SPI module;
         *              the SPI module does not need to be started
         *
         * @param[in]   *spi    Constructed SPI module
         */
        MAX6675 (SPI *spi) {
            this->m_spi = spi;
            this->m_alwaysSetMode = 0;
        }

        /**
         * @brief       Initialize communication with an MAX6675 device
         *
         * @param[in]   mosi    PinNum mask for MOSI
         * @param[in]   miso    PinNum mask for MISO
         * @param[in]   sclk    PinNum mask for SCLK
         * @param[in]   cs      PinNum mask for CS
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode start (const PropWare::Port::Mask mosi,
                const PropWare::Port::Mask miso,
                const PropWare::Port::Mask sclk,
                const PropWare::Port::Mask cs) {
            PropWare::ErrorCode err;

            if (!this->m_spi->is_running()) {
                check_errors(
                        this->m_spi->start(mosi, miso, sclk,
                                PropWare::MAX6675::SPI_DEFAULT_FREQ,
                                PropWare::MAX6675::SPI_MODE,
                                PropWare::MAX6675::SPI_BITMODE));
            } else {
                check_errors(
                        this->m_spi->set_mode(PropWare::MAX6675::SPI_MODE));
                check_errors(
                        this->m_spi->set_bit_mode(
                                PropWare::MAX6675::SPI_BITMODE));
            }

            this->m_cs.set_mask(cs);
            this->m_cs.set_dir(PropWare::Pin::OUT);
            this->m_cs.set();

            return 0;
        }

        /**
         * @brief       Choose whether to always set the SPI mode and bitmode
         *              before reading or writing to the chip; Useful when
         *              multiple devices are connected to the SPI bus
         *
         * @param[in]   alwaysSetMode   For any non-zero value, the SPI modes
         *                              will always be set before a read or
         *                              write routine
         */
        void always_set_spi_mode (const bool alwaysSetMode) {
            this->m_alwaysSetMode = alwaysSetMode;
        }

        /**
         * @brief       Read data in fixed-point form
         *
         * 12-bit data is stored where lower 2 bits are fractional and upper
         * 10 bits are the whole number. Value presented in degrees Celsius
         *
         * @param[out]  *dat    Address where data should be stored
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read (uint16_t *dat) {
            PropWare::ErrorCode err;

            if (this->m_alwaysSetMode) {
                check_errors(
                        this->m_spi->set_mode(PropWare::MAX6675::SPI_MODE));
                check_errors(
                        this->m_spi->set_bit_mode(
                                PropWare::MAX6675::SPI_BITMODE));
            }

            *dat = 0;
            this->m_cs.clear();
            check_errors(
                    this->m_spi->shift_in(MAX6675::BIT_WIDTH, dat,
                            sizeof(*dat)));
            this->m_cs.set();

            return 0;
        }

        /**
         * @brief       Read data and return integer value
         *
         * @param[out]  *dat    Address where data should be stored
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_whole (uint16_t *dat) {
            PropWare::ErrorCode err;

            check_errors(this->read(dat));
            *dat >>= 2;

            return 0;
        }

        /**
         * @brief       Read data in floating point form
         *
         * @param[out]  *dat    Address where data should be stored
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_float (float *dat) {
            PropWare::ErrorCode err;
            uint16_t temp;

            check_errors(this->read(&temp));

            *dat = temp >> 2;
            *dat += ((float) (temp & (BIT_1 | BIT_0))) / 4;

            return 0;
        }

    private:
        static const uint32_t SPI_DEFAULT_FREQ = 1000000;
        static const SPI::Mode SPI_MODE = SPI::MODE_1;
        static const SPI::BitMode SPI_BITMODE = SPI::MSB_FIRST;
        static const uint8_t BIT_WIDTH = 12;

    private:
        SPI *m_spi;
        PropWare::Pin m_cs;
        bool m_alwaysSetMode;
};

}

#endif /* PROPWARE_MAX6675_H_ */
