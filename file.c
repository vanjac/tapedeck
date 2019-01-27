#include "file.h"
#include <stdio.h>
#include <string.h>

int read_file(FILE * file, Tape * tape);
void read_fourcc(FILE * file, char * fourcc);
// return length
unsigned int read_chunk_head(FILE * file, char * id);
// little endian
unsigned short read_uint16(FILE * file);
unsigned int read_uint32(FILE * file);
void read_chunk_fmt(FILE * file);

int write_file(FILE * file, Tape * tape);
void write_fourcc(FILE * file, char * fourcc);
void write_uint16(FILE * file, unsigned short value);
void write_uint32(FILE * file, unsigned int value);


/* LOADING */


int load_tape(char * path, Tape * tape) {
    tape_reset(tape);

    FILE * file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Couldn't open file %s\n", path);
        return 1;
    }

    printf("Reading file %s\n", path);
    int error = read_file(file, tape);

    if (fclose(file)) {
        fprintf(stderr, "Error closing file\n");
    }
    return error;
}

int read_file(FILE * file, Tape * tape) {
    char chunk_id[5] = {0,0,0,0,0};

    unsigned int riff_size = read_chunk_head(file, chunk_id);
    if (strcmp(chunk_id, "RIFF")) {
        fprintf(stderr, "Invalid WAV file\n");
        return 1;
    }
    printf("Size: %d\n", riff_size);
    long riff_start = ftell(file);

    read_fourcc(file, chunk_id);
    if (strcmp(chunk_id, "WAVE")) {
        fprintf(stderr, "Invalid WAV file\n");
        return 1;
    }

    long pos = riff_start + 4;

    while (pos < riff_start + riff_size) {
        fseek(file, pos, SEEK_SET);
        unsigned int chunk_size = read_chunk_head(file, chunk_id);
        printf("%s %d\n", chunk_id, chunk_size);

        if (!strcmp(chunk_id, "fmt "))
            read_chunk_fmt(file);
        else if (!strcmp(chunk_id, "data")) {
            unsigned int read_size = chunk_size;
            if (read_size > TAPE_MAX(tape) - tape->pt_start) {
                fprintf(stderr, "File is too large!\n");
                read_size = TAPE_MAX(tape) - tape->pt_start;
            }
            fread(tape->pt_start, 1, read_size, file);
            tape->pt_end = tape->pt_start + read_size;
        }

        pos += 8 + chunk_size;
    }

    return 0;
}


void read_fourcc(FILE * file, char * fourcc) {
    fread(fourcc, 1, 4, file);
}

unsigned int read_chunk_head(FILE * file, char * id) {
    read_fourcc(file, id);
    return read_uint32(file);
}


unsigned short read_uint16(FILE * file) {
    unsigned short v = 0;
    for (int i = 0; i < 2; i++)
        v += fgetc(file) << (i * 8);
    return v;
}

unsigned int read_uint32(FILE * file) {
    unsigned int v = 0;
    for (int i = 0; i < 4; i++)
        v += fgetc(file) << (i * 8);
    return v;
}


void read_chunk_fmt(FILE * file) {
    unsigned short format = read_uint16(file);
    unsigned short n_channels = read_uint16(file);
    unsigned short sample_rate = read_uint32(file);
    unsigned short data_rate = read_uint32(file);
    unsigned short block_size = read_uint16(file);
    unsigned short bits_per_sample = read_uint16(file);
    printf("Format: %d\nNum channels: %d\nSample rate: %d\nData rate: %d\nBlock size: %d\nBits per sample: %d\n", format, n_channels, sample_rate, data_rate, block_size, bits_per_sample);
}


/* SAVING */


int save_tape(char * path, Tape * tape) {
    FILE * file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "Couldn't write file %s\n", path);
        return 1;
    }

    printf("Writing file %s\n", path);
    write_file(file, tape);

    if (fclose(file)) {
        fprintf(stderr, "Error closing file\n");
    }
    return 0;
}

int write_file(FILE * file, Tape * tape) {
    write_fourcc(file, "RIFF");

    int data_chunk_size = tape->pt_end - tape->pt_start;
    // "WAVE" + fmt chunk + data chunk
    int riff_size = 4 + (8 + 16) + (8 + data_chunk_size);
    write_uint32(file, riff_size);

    write_fourcc(file, "WAVE");

    write_fourcc(file, "fmt ");
    write_uint32(file, 16); // size of format chunk
    write_uint16(file, 1); // format code
    write_uint16(file, 2); // num channels
    write_uint32(file, 44100); // sample rate
    write_uint32(file, 45328); // data rate
    write_uint16(file, 4); // block size
    write_uint16(file, 16); // bits per sample

    write_fourcc(file, "data");
    write_uint32(file, data_chunk_size);
    fwrite(tape->pt_start, 1, data_chunk_size, file);
}


void write_fourcc(FILE * file, char * fourcc) {
    fwrite(fourcc, 1, 4, file);
}


void write_uint16(FILE * file, unsigned short value) {
    for (int i = 0; i < 2; i++)
        fputc((value >> (i * 8)) & 0xFF, file);
}

void write_uint32(FILE * file, unsigned int value) {
    for (int i = 0; i < 4; i++)
        fputc((value >> (i * 8)) & 0xFF, file);
}

