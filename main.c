#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/soundcard.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include "tape.h"

#define BUFFER_SIZE 1024
// affects playback latency
#define PLAYBACK_BUFFER_SIZE 4096
// affects record latency
#define RECORD_BUFFER_SIZE 2048

// https://www.alsa-project.org/main/index.php/ALSA_Library_API
#define MIDI_DEVICE "/dev/snd/midiC1D0"

int main(int argc, char *argv[]) {
    int midi_fd = open(MIDI_DEVICE, O_RDONLY | O_NONBLOCK);
    if (midi_fd == -1) {
        fprintf(stderr, "Couldn't open midi device %s\n", MIDI_DEVICE);
        return 1;
    }

    static const pa_sample_spec sample_spec = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };

    static const pa_buffer_attr playback_buff_attr = {
        .tlength = PLAYBACK_BUFFER_SIZE,
        .maxlength = -1,
        .minreq = -1,
        .prebuf = -1
    };

    static const pa_buffer_attr record_buff_attr = {
        .fragsize = RECORD_BUFFER_SIZE,
        .maxlength = -1
    };

    int pa_error;

    pa_simple *play_stream = pa_simple_new(
        NULL,               // default server
        "tapedeck",         // name of app
        PA_STREAM_PLAYBACK,
        NULL,               // default device
        "playback",         // stream description
        &sample_spec,
        NULL,               // default channel map
        &playback_buff_attr,
        &pa_error);
    if (!play_stream) {
        fprintf(stderr, "Couldn't open playback stream: %s\n", pa_strerror(pa_error));
        return 1;
    }
    pa_simple *record_stream = pa_simple_new(
        NULL,               // default server
        "tapedeck",         // name of app
        PA_STREAM_RECORD,
        NULL,               // default device
        "record",           // stream description
        &sample_spec,
        NULL,               // default channel map
        &record_buff_attr,
        &pa_error);
    if (!record_stream) {
        fprintf(stderr, "Couldn't open record stream: %s\n", pa_strerror(pa_error));
        return 1;
    }

    uint8_t audio_buffer[BUFFER_SIZE];
    unsigned char midi_message[3];
    ssize_t midi_bytes_read;
    while(1) {
        pa_simple_read(record_stream, audio_buffer, sizeof(audio_buffer), &pa_error);
        pa_simple_write(play_stream, audio_buffer, sizeof(audio_buffer), &pa_error);

        if (pa_error) {
            fprintf(stderr, "PulseAudio error! %s\n", pa_strerror(pa_error));
            return 1;
        }

        while(1) {
            midi_bytes_read = read(midi_fd, &midi_message, sizeof(midi_message));
            if (midi_bytes_read < 0) { // read error
                if (errno == EWOULDBLOCK)
                    break; // nothing to read
                else {
                fprintf(stderr, "MIDI read error! %d\n", errno);
                return 1;
                }
            }
            else if (midi_bytes_read > 0) {
                printf("MIDI message %d %d %d\n", midi_message[0], midi_message[1], midi_message[2], midi_message[3]);
            }
        }
    }

    pa_simple_free(play_stream);
    pa_simple_free(record_stream);
    close(midi_fd);
    return 0;
}

