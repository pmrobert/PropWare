/**
 * @file    l3g.h
 *
 * @author  David Zemon
 * @author  Collin Winans
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

#ifndef PROPWARE_L3G_H_
#define PROPWARE_L3G_H_

#include <propeller.h>
#include <PropWare/PropWare.h>
#include <PropWare/spi.h>

namespace PropWare {

/**
 * @brief   L3G gyroscope driver using SPI communication for the Parallax
 *          Propeller
 */
class L3G {
    public:
        /**
         * Axes of the L3G device
         */
        typedef enum {
            /** X axis */X,
            /** Y axis */Y,
            /** Z axis */Z
        } Axis;

        /**
         * Sensitivity measured in degrees per second
         */
        typedef enum {
            /** 250 degrees per second */
            DPS_250 = 0x00,
            /** 500 degrees per second */
            DPS_500 = 0x10,
            /** 2000 degrees per second */
            DPS_2000 = 0x20
        } DPSMode;

    public:
        static const uint8_t WHO_AM_I = 0x0F;

        static const uint8_t CTRL_REG1 = 0x20;
        static const uint8_t CTRL_REG2 = 0x21;
        static const uint8_t CTRL_REG3 = 0x22;
        static const uint8_t CTRL_REG4 = 0x23;
        static const uint8_t CTRL_REG5 = 0x24;
        static const uint8_t REFERENCE = 0x25;
        static const uint8_t OUT_TEMP = 0x26;
        static const uint8_t STATUS_REG = 0x27;
        static const uint8_t OUT_X_L = 0x28;
        static const uint8_t OUT_X_H = 0x29;
        static const uint8_t OUT_Y_L = 0x2A;
        static const uint8_t OUT_Y_H = 0x2B;
        static const uint8_t OUT_Z_L = 0x2C;
        static const uint8_t OUT_Z_H = 0x2D;

        static const uint8_t FIFO_CTRL_REG = 0x2E;
        static const uint8_t FIFO_SRC_REG = 0x2F;

        static const uint8_t INT1_CFG = 0x30;
        static const uint8_t INT1_SRC = 0x31;
        static const uint8_t INT1_THS_XH = 0x32;
        static const uint8_t INT1_THS_XL = 0x33;
        static const uint8_t INT1_THS_YH = 0x34;
        static const uint8_t INT1_THS_YL = 0x35;
        static const uint8_t INT1_THS_ZH = 0x36;
        static const uint8_t INT1_THS_ZL = 0x37;
        static const uint8_t INT1_DURATION = 0x38;

    public:
        /**
         * @brief       Construction requires an instance of the SPI module;
         *              the SPI module does not need to be started
         *
         * @param[in]   *spi    Constructed SPI module
         */
        L3G (SPI *spi) {
            this->m_spi = spi;
            this->m_alwaysSetMode = false;
        }

        /**
         * @brief       Initialize an L3G module
         *
         * @param[in]   mosi        PinNum mask for MOSI
         * @param[in]   miso        PinNum mask for MISO
         * @param[in]   sclk        PinNum mask for SCLK
         * @param[in]   cs          PinNum mask for CS
         *
         * @return       Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode start (const PropWare::Port::Mask mosi,
                const PropWare::Port::Mask miso,
                const PropWare::Port::Mask sclk,
                const PropWare::Port::Mask cs) {
            PropWare::ErrorCode err;

            // Ensure SPI module started
            if (this->m_spi->is_running()) {
                check_errors(this->m_spi->set_mode(L3G::SPI_MODE));
                check_errors(this->m_spi->set_bit_mode(L3G::SPI_BITMODE));
            } else {
                check_errors(
                        this->m_spi->start(mosi, miso, sclk,
                                L3G::SPI_DEFAULT_FREQ, L3G::SPI_MODE,
                                L3G::SPI_BITMODE));
            }

            this->m_cs.set_mask(cs);
            this->m_cs.set_dir(PropWare::Pin::OUT);
            this->m_cs.set();

            // NOTE L3G has high- and low-pass filters. Should they be enabled?
            // (Page 31)
            check_errors(this->write8(L3G::CTRL_REG1, NIBBLE_0));
            check_errors(this->write8(L3G::CTRL_REG4, BIT_7));

            return 0;
        }

        /**
         * @brief       Choose whether to always set the SPI mode and bitmode
         *              before reading or writing to the L3G module; Useful when
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
         * @brief       Read a specific axis's data
         *
         * @param[in]   axis    Selects the axis to be read
         * @param[out]  *val    Address that data should be placed into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read (const L3G::Axis axis, int16_t *val) const {
            return this->read16(L3G::OUT_X_L + (axis << 1), val);
        }

        /**
         * @brief       Read data from the X axis
         *
         * @param[out]  *val    Address that data should be placed into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_x (int16_t *val) const {
            return this->read16(L3G::OUT_X_L, val);
        }

        /**
         * @brief       Read data from the Y axis
         *
         * @param[out]  *val    Address that data should be placed into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_y (int16_t *val) const {
            return this->read16(L3G::OUT_Y_L, val);
        }

        /**
         * @brief       Read data from the Z axis
         *
         * @param[out]  *val    Address that data should be placed into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_z (int16_t *val) const {
            return this->read16(L3G::OUT_Z_L, val);
        }

        /**
         * @brief       Read data from all three axes
         *
         * @param[out]  *val    Starting address for data to be placed; 6
         *                      contiguous bytes of space are required for the
         *                      read routine
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_all (int16_t *val) const {
            PropWare::ErrorCode err;
            uint8_t i;

            uint8_t addr = L3G::OUT_X_L;
            addr |= BIT_7;  // Set RW bit (
            addr |= BIT_6;  // Enable address auto-increment

            check_errors(this->maybe_set_spi_mode());

            this->m_cs.clear();
            check_errors(this->m_spi->shift_out(8, addr));
            check_errors(
                    this->m_spi->shift_in(16, &(val[L3G::X]),
                            sizeof(val[L3G::X])));
            check_errors(
                    this->m_spi->shift_in(16, &(val[L3G::Y]),
                            sizeof(val[L3G::Y])));
            check_errors(
                    this->m_spi->shift_in(16, &(val[L3G::Z]),
                            sizeof(val[L3G::Z])));
            this->m_cs.set();

            // err is useless at this point and will be used as a temporary
            // 8-bit variable
            for (i = 0; i < 3; ++i) {
                err = (ErrorCode) (val[i] >> 8);
                val[i] <<= 8;
                val[i] |= err;
            }

            return 0;
        }

        /**
         * @brief       Modify the scale of L3G in units of degrees per second
         *
         * @param[in]   dpsMode     Desired full-scale mode
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode set_dps (const PropWare::L3G::DPSMode dpsMode) {
            PropWare::ErrorCode err;
            uint8_t oldValue;

            this->m_dpsMode = dpsMode;
            check_errors(this->maybe_set_spi_mode());

            check_errors(this->read8(L3G::CTRL_REG4, (int8_t * ) &oldValue));
            oldValue &= ~(BIT_5 | BIT_4);
            oldValue |= dpsMode;
            check_errors(this->write8(L3G::CTRL_REG4, oldValue));

            return 0;
        }

        /**
         * @brief   Retrieve the current DPS setting
         *
         * @return  Returns what the L3G module is using for DPS mode
         */
        PropWare::L3G::DPSMode get_dps () const {
            return this->m_dpsMode;
        }

        /**
         * @brief       Convert the raw, integer value from the gyro into units
         *              of degrees-per-second
         *
         * @pre         Input value must have been read in when the DPS setting
         *              was set to the same value as it is during this function
         *              execution. If the input value was read with a different
         *              DPS setting, use
         *              PropWare::L3G::convert_to_dps(const int16_t rawValue,
         *              const PropWare::L3G::DPSMode dpsMode) to specify the
         *              correct DPS setting
         *
         * @param[in]   rawValue    Value from the gyroscope
         *
         * @return      Returns the rotational value in degrees-per-second
         */
        float convert_to_dps (const int16_t rawValue) const {
            return PropWare::L3G::convert_to_dps(rawValue, this->m_dpsMode);
        }

        /**
         * @brief       Convert the raw, integer value from the gyro into units
         *              of degrees-per-second
         *
         * @param[in]   rawValue    Value from the gyroscope
         * @param[in]   dpsMode     The DPS setting used at the time of reading
         *                          rawValue
         *
         * @return      Returns the rotational value in degrees-per-second
         */
        static float convert_to_dps (const int16_t rawValue,
                const PropWare::L3G::DPSMode dpsMode) {
            switch (dpsMode) {
                case PropWare::L3G::DPS_250:
                    return (float) (rawValue * 0.00875);
                case PropWare::L3G::DPS_500:
                    return (float) (rawValue * 0.01750);
                case PropWare::L3G::DPS_2000:
                    return (float) (rawValue * 0.07000);
            }
            return 0;
        }

    private:
        static const uint32_t SPI_DEFAULT_FREQ = 900000;
        static const SPI::Mode SPI_MODE = SPI::MODE_3;
        static const SPI::BitMode SPI_BITMODE = SPI::MSB_FIRST;

    private:
        /***********************************
         *** Private Method Declarations ***
         ***********************************/
        /**
         * @brief       Write one byte to the L3G module
         *
         * @param[in]   addr    Destination register address
         * @param[in]   dat     Data to be written to the destination register
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode write8 (uint8_t addr, const uint8_t dat) const {
            PropWare::ErrorCode err;
            uint16_t outputValue;

            addr &= ~BIT_7;  // Clear the RW bit (write mode)

            outputValue = ((uint16_t) addr) << 8;
            outputValue |= dat;

            check_errors(this->maybe_set_spi_mode());

            this->m_cs.clear();
            check_errors(this->m_spi->shift_out(16, outputValue));
            check_errors(this->m_spi->wait());
            this->m_cs.set();

            return err;
        }

        /**
         * @brief       Write one byte to the L3G module
         *
         * @param[in]   addr    Destination register address
         * @param[in]   dat     Data to be written to the destination register
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode write16 (uint8_t addr, const uint16_t dat) const {
            PropWare::ErrorCode err;
            uint16_t outputValue;

            addr &= ~BIT_7;  // Clear the RW bit (write mode)
            addr |= BIT_6;  // Enable address auto-increment

            outputValue = ((uint16_t) addr) << 16;
            outputValue |= ((uint16_t) ((uint8_t) dat)) << 8;
            outputValue |= (uint8_t) (dat >> 8);

            check_errors(this->maybe_set_spi_mode());

            this->m_cs.clear();
            check_errors(this->m_spi->shift_out(24, outputValue));
            check_errors(this->m_spi->wait());
            this->m_cs.set();

            return 0;
        }

        /**
         * @brief       Read one byte from the L3G module
         *
         * @param[in]   addr    Origin register address
         * @param[out]  *dat    Address where incoming data should be stored
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read8 (uint8_t addr, int8_t *dat) const {
            PropWare::ErrorCode err;

            addr |= BIT_7;  // Set RW bit (
            addr |= BIT_6;  // Enable address auto-increment

            check_errors(this->maybe_set_spi_mode());

            this->m_cs.clear();
            check_errors(this->m_spi->shift_out(8, addr));
            check_errors(this->m_spi->shift_in(8, dat, sizeof(*dat)));
            this->m_cs.set();

            return 0;
        }

        /**
         * @brief       Read two bytes from the L3G module
         *
         * @param[in]   addr    Origin register address
         * @param[out]  *dat    Address where incoming data should be stored
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read16 (uint8_t addr, int16_t *dat) const {
            PropWare::ErrorCode err;

            addr |= BIT_7;  // Set RW bit (
            addr |= BIT_6;  // Enable address auto-increment

            check_errors(this->maybe_set_spi_mode());

            this->m_cs.clear();
            check_errors(this->m_spi->shift_out(8, addr));
            check_errors(this->m_spi->shift_in(16, dat, sizeof(*dat)));
            this->m_cs.set();

            // err is useless at this point and will be used as a temporary
            // 8-bit variable
            err = (ErrorCode) (*dat >> 8);
            *dat <<= 8;
            *dat |= err;

            return 0;
        }

        /**
         * @brief   Set the SPI mode iff PropWare::L3G::m_alwaysSetMode is true
         *
         * @return  Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode maybe_set_spi_mode () const {
            PropWare::ErrorCode err;

            if (this->m_alwaysSetMode) {
                check_errors(this->m_spi->set_mode(L3G::SPI_MODE));
                check_errors(this->m_spi->set_bit_mode(L3G::SPI_BITMODE));
            }

            return 0;
        }

    private:
        SPI *m_spi;
        PropWare::Pin m_cs;
        DPSMode m_dpsMode;
        bool m_alwaysSetMode;
};

}

#endif /* PROPWARE_L3G_H_ */
