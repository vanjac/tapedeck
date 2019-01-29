#include "file.h"
#include <stdio.h>
#include <string.h>

#define TAPE_PATH_SIZE 16

#define FMT_CHUNK_ID "fmt "
#define CUE_CHUNK_ID "cue "
#define DATA_CHUNK_ID "data"

#define FMT_CHUNK_SIZE 16
// 4 byte count + 2 cue points (24 bytes per cue)
#define CUE_CHUNK_SIZE 52

#define IN_CUE_ID 1
#define OUT_CUE_ID 2

// https://sites.google.com/site/musicgapi/technical-documents/wav-file-format


int read_file(FILE * file, Tape * tape);
void read_fourcc(FILE * file, char * fourcc);
// return length
unsigned int read_chunk_head(FILE * file, char * id);
// little endian
unsigned short read_uint16(FILE * file);
unsigned int read_uint32(FILE * file);
void read_chunk_fmt(FILE * file);
int read_chunk_cue(FILE * file, Tape * tape);
int read_chunk_data(FILE * file, unsigned int chunk_size, Tape * tape);

int write_file(FILE * file, Tape * tape);
void write_fourcc(FILE * file, char * fourcc);
void write_uint16(FILE * file, unsigned short value);
void write_uint32(FILE * file, unsigned int value);
void write_cue(FILE * file, unsigned int id, unsigned int position);


void path_for_tape(Tape * tape, char * path_out) {
    path_out[0] = "ESLC"[tape->tape_num / 4]; // effect sample loop cue
    path_out[1] = "1234"[tape->tape_num % 4];
    path_out[2] = '.'; // yes this is good code
    path_out[3] = 'w';
    path_out[4] = 'a';
    path_out[5] = 'v';
    path_out[6] = 0;
}


/* LOADING */


int load_tape(Tape * tape) {
    tape_reset(tape);

    char path[TAPE_PATH_SIZE];
    path_for_tape(tape, path);

    FILE * file = fopen(path, "rb");
    if (file == NULL) {
        return 0; // probably doesn't exist
    }

    int error = read_file(file, tape);

    if (fclose(file)) {
        fprintf(stderr, "Error closing file\n");
    }
    return error;
}

int read_file(FILE * file, Tape * tape) {
    int error = 0;
    char chunk_id[5] = {0,0,0,0,0};

    unsigned int riff_size = read_chunk_head(file, chunk_id);
    if (strcmp(chunk_id, "RIFF")) {
        fprintf(stderr, "Invalid WAV file\n");
        return 1;
    }
    //printf("Size: %d\n", riff_size);
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
        //printf("%s %d\n", chunk_id, chunk_size);

        if (chunk_size % 2 == 1)
            chunk_size ++; // chunks are always word-aligned

        if (!strcmp(chunk_id, FMT_CHUNK_ID)) {
            //read_chunk_fmt(file);
            // ignore fmt chunk
        } else if (!strcmp(chunk_id, CUE_CHUNK_ID)) {
            if (read_chunk_cue(file, tape))
                error = 1;
        } else if (!strcmp(chunk_id, DATA_CHUNK_ID)) {
            if (read_chunk_data(file, chunk_size, tape))
                error = 1;
        }

        pos += 8 + chunk_size;
    }

    return error;
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

int read_chunk_cue(FILE * file, Tape * tape) {
    int error = 0;
    unsigned int num_cues = read_uint32(file);
    for (int i = 0; i < num_cues; i++) {
        unsigned int cue_id = read_uint32(file);
        fseek(file, 12, SEEK_CUR);
        unsigned int cue_pos = read_uint32(file);
        fseek(file, 4, SEEK_CUR);

        if (cue_pos > TAPE_MAX(tape) - tape->pt_start) {
            fprintf(stderr, "Cue past end of tape!\n");
            error = 1;
            cue_pos = TAPE_MAX(tape) - tape->pt_start;
        }
        if (cue_id == IN_CUE_ID)
            tape->pt_in = cue_pos + tape->pt_start;
        else if (cue_id = OUT_CUE_ID)
            tape->pt_out = cue_pos + tape->pt_start;
    }
    return error;
}

int read_chunk_data(FILE * file, unsigned int chunk_size, Tape * tape) {
    int error = 0;
    if (chunk_size > TAPE_SIZE) {
        error = 1;
        fprintf(stderr, "File is too large!\n");
        move_all_tape_points(tape, tape->audio_data - tape->pt_start);
        chunk_size = TAPE_SIZE;
    } else if (chunk_size > TAPE_SIZE / 2) {
        move_all_tape_points(tape, -((long)chunk_size - TAPE_SIZE/2) / 2);
    }
    fread(tape->pt_start, 1, chunk_size, file);
    tape->pt_end = tape->pt_start + chunk_size;
    return error;
}


/* SAVING */


int save_tape(Tape * tape) {
    char path[TAPE_PATH_SIZE];
    path_for_tape(tape, path);

    FILE * file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "Couldn't write file %s\n", path);
        return 1;
    }

    write_file(file, tape);

    if (fclose(file)) {
        fprintf(stderr, "Error closing file\n");
    }
    return 0;
}

int write_file(FILE * file, Tape * tape) {
    write_fourcc(file, "RIFF");

    int data_chunk_size = tape->pt_end - tape->pt_start;
    int riff_size = 4 + (8 + FMT_CHUNK_SIZE) + (8 + data_chunk_size)
        + (8 + CUE_CHUNK_SIZE);
    write_uint32(file, riff_size);

    write_fourcc(file, "WAVE");

    write_fourcc(file, FMT_CHUNK_ID); // begin format chunk
    write_uint32(file, FMT_CHUNK_SIZE);
    write_uint16(file, 1); // format code
    write_uint16(file, 2); // num channels
    write_uint32(file, 44100); // sample rate
    write_uint32(file, 45328); // data rate
    write_uint16(file, 4); // block size
    write_uint16(file, 16); // bits per sample

    write_fourcc(file, CUE_CHUNK_ID); // begin cue chunk
    write_uint32(file, CUE_CHUNK_SIZE);
    write_uint32(file, 2); // num cue points
    write_cue(file, IN_CUE_ID, tape->pt_in - tape->pt_start);
    write_cue(file, OUT_CUE_ID, tape->pt_out - tape->pt_start);

    write_fourcc(file, DATA_CHUNK_ID); // begin data chunk
    write_uint32(file, data_chunk_size); // size of data chunk
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

void write_cue(FILE * file, unsigned int id, unsigned int position) {
    write_uint32(file, id);
    write_uint32(file, 0);
    write_fourcc(file, DATA_CHUNK_ID);
    write_uint32(file, 0);
    write_uint32(file, position);
    write_uint32(file, position);
}
