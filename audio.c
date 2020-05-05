#include "audio.h"
#include <stdio.h>
#include <pulse/error.h>
#include "main.h"

// pactl list | grep -A6 'Sink #'
#define AUX_SEND_DEVICE "alsa_output.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.analog-stereo"

pa_simple *play_stream;
pa_simple *aux_send_stream;
pa_simple *record_stream;

int audio_open(void) {
    static const pa_sample_spec sample_spec = {
        .format = PA_SAMPLE_FLOAT32,
        .rate = FRAME_RATE,
        .channels = OUT_CHANNELS
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

    play_stream = pa_simple_new(
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
    aux_send_stream = pa_simple_new(
        NULL,
        "tapedeck",
        PA_STREAM_PLAYBACK,
        AUX_SEND_DEVICE,
        "aux send",
        &sample_spec,
        NULL,
        &playback_buff_attr,
        &pa_error);
    if (!aux_send_stream) {
        fprintf(stderr, "Couldn't open aux-send stream: %s\n", pa_strerror(pa_error));
        return 1;
    }
    record_stream = pa_simple_new(
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
    return 0;
}

void audio_close(void) {
    pa_simple_free(play_stream);
    pa_simple_free(aux_send_stream);
    pa_simple_free(record_stream);
}

int audio_read(sample * buffer, int num_samples) {
    int pa_error;
    if (pa_simple_read(record_stream, buffer, num_samples * sizeof(sample), &pa_error) < 0) {
        fprintf(stderr, "Read failed: %s\n", pa_strerror(pa_error));
        return 1;
    }
    return 0;
}

int audio_write(sample * buffer, int num_samples) {
    int pa_error;
    if (pa_simple_write(play_stream, buffer, num_samples * sizeof(sample), &pa_error) < 0) {
        fprintf(stderr, "Write failed: %s\n", pa_strerror(pa_error));
        return 1;
    }
    return 0;
}

int audio_write_aux(sample * buffer, int num_samples) {
    int pa_error;
    if (pa_simple_write(aux_send_stream, buffer, num_samples * sizeof(sample), &pa_error) < 0) {
        fprintf(stderr, "Aux write failed: %s\n", pa_strerror(pa_error));
        return 1;
    }
    return 0;
}

int audio_flush(void) {
    int pa_error;
    if (pa_simple_flush(play_stream, &pa_error) < 0
        || pa_simple_flush(aux_send_stream, &pa_error) < 0
        || pa_simple_flush(record_stream, &pa_error) < 0) {
        fprintf(stderr, "Flush failed: %s\n", pa_strerror(pa_error));
    }
}
