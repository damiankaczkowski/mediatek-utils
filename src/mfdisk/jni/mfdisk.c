/* mfdisk.c -- Partition table manipulator for MT65xx boards
 *
 * Copyright (C) 2013 Rush Future
 *
 * This program is free software.  You can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation: either version 1 or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "common.h"
#include "mfdisk.h"

/* start_sect and nr_sects are stored little endian on all machines */
/* moreover, they are not aligned correctly */
static void
store4_little_endian(unsigned char *cp, unsigned int val) {
        cp[0] = (val & 0xff);
        cp[1] = ((val >> 8) & 0xff);
        cp[2] = ((val >> 16) & 0xff);
        cp[3] = ((val >> 24) & 0xff);
}

static unsigned int
read4_little_endian(const unsigned char *cp) {
        return (unsigned int)(cp[0]) + ((unsigned int)(cp[1]) << 8)
                + ((unsigned int)(cp[2]) << 16)
                + ((unsigned int)(cp[3]) << 24);
}

int read_blocks(void *buffer, const char *image, int start, int count) {
    int fd = open(image, 0);
    if (fd == -1) return -1;

    lseek(fd, start * SECTOR_SIZE, SEEK_SET);
    read(fd, buffer, count * SECTOR_SIZE);
    close(fd);

    return count;
}

int bootrecord_print(struct bootrecord *b) {

    printf("* %s:\n", b->filename);

    struct partition *p = b->partitions;

    for (int i = 0; i < 4; i++) {
        printf("%02x %08x %08x\n", p->sys_ind, read4_little_endian(p->start4) + b->offset, read4_little_endian(p->size4));
        p++;
    }

    return 0;
}

int bootrecord_read(struct bootrecord *b, const char *filename, int offset, int extended) {

    b->filename = filename;
    b->offset = offset;
    b->extended = extended;

    int fd = open(filename, 0);
    if (fd == -1) return -1;
    read(fd, (void*) b, SECTOR_SIZE);
    close(fd);

    return 0;
}

struct pmt_entry {
    char name[0x10];
    char data[15*4];
} PACKED;

struct pmt_table {
    char signature1[4];
    char name[12];
    char padding1[0x40];
    struct pmt_entry entries[25];
    char padding2[0x40];
    char signature2[4]; // must be equal to signature1
} PACKED;


void pmt_read(struct pmt_table *p) {
    read_blocks(p, "/dev/pmt", 0, 8);
}

void pmt_entry_print(struct pmt_entry *e) {
    printf("%-17s", e->name);
    const char *data = e->data;
    for (int i = 0; i < 15; i++) {
        printf(" %08x", read4_little_endian(data));
        data += 4;
    }
    printf("\n");
}

void pmt_table_print(struct pmt_table *t) {
    printf("* PMT1 %s:\n", t->name);
    struct pmt_entry *e = t->entries;
    for (int i=0; i < 26; i++) {
        if (e->name[0] == 0xff) break;
        pmt_entry_print(e++);
    }
}

int main(int argc, char *argv[]) {
    struct bootrecord br;
    struct pmt_table pmt[2];

    if ( sizeof(struct pmt_table) != 0x800) {
        printf("internal structures is corrupted, please recompile me (pmt structure size mismatch)\n");
        return -1;
    }

    bootrecord_read(&br, "/dev/mbr", 0, 0);
    bootrecord_print(&br);

    bootrecord_read(&br, "/dev/ebr1", 0x20, 1);
    bootrecord_print(&br);

    pmt_read(pmt);
    pmt_table_print(pmt);
}
