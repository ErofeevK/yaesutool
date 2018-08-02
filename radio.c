/*
 * Clone Utility for Yaesu radios.
 *
 * Copyright (C) 2018 Serge Vakulenko, KK6ABQ
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. The name of the author may not be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include "radio.h"
#include "util.h"

int radio_port;                         // File descriptor of programming serial port
unsigned char radio_mem [0x10000];      // Radio: memory contents
int radio_progress;                     // Read/write progress counter

static radio_device_t *device;          // Device-dependent interface

//
// Close the serial port.
//
void radio_disconnect()
{
    fprintf(stderr, "Close device.\n");

    // Restore the port mode.
    serial_close(radio_port);

    // Radio needs a timeout to reset to a normal state.
    mdelay(2000);
}

//
// Print a generic information about the device.
//
void radio_print_version(FILE *out)
{
    fprintf(out, "Radio: %s\n", device->name);
    device->print_version(out);
}

//
// Connect to the radio and identify the type of device.
//
void radio_connect(const char *port_name, const char *radio_type)
{
    if (strcasecmp("ft60", radio_type) == 0) {          // Yaesu FT-60R
        device = &radio_ft60;
    } else if (strcasecmp("vx2", radio_type) == 0) {    // Yaesu VX-2R, VX-2E
        device = &radio_vx2;
    }

    fprintf(stderr, "Connect to %s at %d.\n", port_name, device->baud);
    radio_port = serial_open(port_name, device->baud);
    printf("Radio: %s\n", device->name);
}

//
// Read firmware image from the device.
//
void radio_download()
{
    radio_progress = 0;
    if (! verbose)
        fprintf(stderr, "Read device: ");

    device->download();

    if (! verbose)
        fprintf(stderr, " done.\n");
}

//
// Write firmware image to the device.
//
void radio_upload(int cont_flag)
{
    // Check for compatibility.
    if (! device->is_compatible()) {
        fprintf(stderr, "Incompatible image - cannot upload.\n");
        exit(-1);
    }
    radio_progress = 0;
    if (! verbose)
        fprintf(stderr, "Write device: ");

    serial_flush(radio_port);
    device->upload(cont_flag);

    if (! verbose)
        fprintf(stderr, " done.\n");
}

//
// Read firmware image from the binary file.
//
void radio_read_image(char *filename)
{
    FILE *img;
    struct stat st;

    fprintf(stderr, "Read image from file '%s'.\n", filename);

    // Guess device type by file size.
    if (stat(filename, &st) < 0) {
        perror(filename);
        exit(-1);
    }
    switch (st.st_size) {
    case 28616:
    case 31435:
        device = &radio_ft60;
        break;
    case 32595:
        device = &radio_vx2;
        break;
    default:
        fprintf(stderr, "%s: Unrecognized file size %u bytes.\n",
            filename, (int) st.st_size);
        exit(-1);
    }

    img = fopen(filename, "rb");
    if (! img) {
        perror(filename);
        exit(-1);
    }
    device->read_image(img);
    fclose(img);
}

//
// Save firmware image to the binary file.
//
void radio_save_image(char *filename)
{
    FILE *img;

    fprintf(stderr, "Write image to file '%s'.\n", filename);
    img = fopen(filename, "w");
    if (! img) {
        perror(filename);
        exit(-1);
    }
    device->save_image(img);
    fclose(img);
}

//
// Read the configuration from text file, and modify the firmware.
//
void radio_parse_config(char *filename)
{
    FILE *conf;
    char line [256], *p, *v;
    int table_id = 0, table_dirty = 0;

    fprintf(stderr, "Read configuration from file '%s'.\n", filename);
    conf = fopen(filename, "r");
    if (! conf) {
        perror(filename);
        exit(-1);
    }

    while (fgets(line, sizeof(line), conf)) {
        line[sizeof(line)-1] = 0;

        // Strip comments.
        v = strchr(line, '#');
        if (v)
            *v = 0;

        // Strip trailing spaces and newline.
        v = line + strlen(line) - 1;
        while (v >= line && (*v=='\n' || *v=='\r' || *v==' ' || *v=='\t'))
            *v-- = 0;

        // Ignore comments and empty lines.
        p = line;
        if (*p == 0)
            continue;

        if (*p != ' ') {
            // Table finished.
            table_id = 0;

            // Find the value.
            v = strchr(p, ':');
            if (! v) {
                // Table header: get table type.
                table_id = device->parse_header(p);
                if (! table_id) {
badline:            fprintf(stderr, "Invalid line: '%s'\n", line);
                    exit(-1);
                }
                table_dirty = 0;
                continue;
            }

            // Parameter.
            *v++ = 0;

            // Skip spaces.
            while (*v == ' ' || *v == '\t')
                v++;

            device->parse_parameter(p, v);

        } else {
            // Table row or comment.
            // Skip spaces.
            // Ignore comments and empty lines.
            while (*p == ' ' || *p == '\t')
                p++;
            if (*p == '#' || *p == 0)
                continue;
            if (! table_id)
                goto badline;

            if (! device->parse_row(table_id, ! table_dirty, p))
                goto badline;
            table_dirty = 1;
        }
    }
    fclose(conf);
}

//
// Print full information about the device configuration.
//
void radio_print_config(FILE *out, int verbose)
{
    if (verbose) {
        char buf [40];
        time_t t;
        struct tm *tmp;

        t = time(NULL);
        tmp = localtime(&t);
        if (! tmp || ! strftime(buf, sizeof(buf), "%Y/%m/%d ", tmp))
            buf[0] = 0;
        fprintf(out, "#\n");
        fprintf(out, "# This configuration was generated %sby YaesuTool,\n", buf);
        fprintf(out, "# Version %s, %s\n", version, copyright);
        fprintf(out, "#\n");
    }
    device->print_config(out, verbose);
}
