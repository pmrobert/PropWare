/**
 * @file    SD_Demo.cpp
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

#include "SD_Demo.h"

// Main function
int main () {
    PropWare::ErrorCode err;
    char c;

    PropWare::Pin statusLED(PropWare::Port::P16, PropWare::Pin::OUT);

    PropWare::SPI *spi = PropWare::SPI::getInstance();
    PropWare::SD sd(spi);
    PropWare::SD::File f, f2;

#ifndef LOW_RAM_MODE
    /* Option 1: Create at least one new SD::Buffer variable
     *
     * An extra 526 bytes of memory are required to create a new SD::Buffer
     * for the file variable, but speed will be increased if files are
     * being switched often. Using this option will allow the directory
     * contents to be kept in RAM while a file is loaded.
     *
     */
    PropWare::SD::Buffer fileBuf;  // If extra RAM is available
    PropWare::SD::Buffer fileBuf2;
    f.buf = &fileBuf;
    f2.buf = &fileBuf2;
#else
    /* Option 2: Use the generic buffer [i.e. SD.m_buf] as the buffer
     *
     * Good for low-RAM situations due to the re-use of g_sd_buf. Speed is
     * decreased when multiple files are used often.
     *
     */
    f.buf = sd.get_global_buffer();
    f2.buf = sd.get_global_buffer();
#endif

#ifdef DEBUG
    printf("Beginning SD card initialization...\n");
#endif

    // Start your engines!!!
    if ((err = sd.start(MOSI, MISO, SCLK, CS, -1)))
        error(err, &sd);

#ifdef DEBUG
    printf("SD routine started. Mounting now...\n");
#endif
    if ((err = sd.mount()))
        error(err, &sd);
#ifdef DEBUG
    printf("FAT partition mounted!\n");
#endif

#ifdef TEST_SHELL
    sd.shell(&f);
#elif (defined TEST_WRITE)
    // Create a blank file and copy the contents of STUFF.TXT into it
    sd.fopen(OLD_FILE, &f, PropWare::SD::FILE_MODE_R);
    sd.fopen(NEW_FILE, &f2, PropWare::SD::FILE_MODE_R_PLUS);

#ifdef DEBUG
    printf("Both files opened...\n");
#endif

    while (!sd.feof(&f)) {
        c = sd.fgetc(&f);
        sd.fputc(c, &f2);
#ifdef _STDIO_H
        putchar(SDfgetc(&f2));
#endif
    }

#ifdef DEBUG
    printf("\nFile printed...\n");

    printf("Now closing read-only file!\n");
#endif
    sd.fclose(&f);
#ifdef DEBUG
    printf("***Now closing the modified file!***\n");
#endif
    sd.fclose(&f2);

#ifdef DEBUG
    printf("Files closed...\n");

    sd.fopen(NEW_FILE, &f2, PropWare::SD::FILE_MODE_R);
    printf("File opened for a second time, now printing new contents...\n");
    while (!sd.feof(&f2))
    putchar(sd.fgetc(&f2));
    sd.fclose(&f2);
#endif

    sd.unmount();
#else
    sd.chdir("JAZZ");
    sd.fopen("DESKTOP.INI", &f, PropWare::SD::FILE_MODE_R);

    while (!sd.feof(&f))
#ifdef DEBUG
    putchar(sd.fgetc(&f));
#endif
#endif

#ifdef DEBUG
    printf("Execution complete!\n");
#endif

    while (1) {
        statusLED.toggle();
        waitcnt(CLKFREQ/2 + CNT);
    }

    return 0;
}

void error (const PropWare::ErrorCode err, const PropWare::SD *sd) {
    PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Pin::OUT);

    if (PropWare::SPI::BEG_ERROR <= err && err < PropWare::SPI::END_ERROR)
        PropWare::SPI::getInstance()->print_error_str(
                (PropWare::SPI::ErrorCode) err);
    else if (PropWare::SD::BEG_ERROR <= err && err < PropWare::SD::END_ERROR)
//        sd->print_error_str((PropWare::SD::ErrorCode) err);
        ;

    while (1) {
        debugLEDs.write(err);
        waitcnt(100*MILLISECOND);
        debugLEDs.write(0);
        waitcnt(100*MILLISECOND);
    }
}
