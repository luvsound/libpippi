#include "pippicore.h"
#include "microsound.h"

lpgrain_t * grain_create(size_t maxlength, size_t minlength, buffer_t * window) {
    lpgrain_t * g;

    g = (lpgrain_t *)MemoryPool.alloc(1, sizeof(lpgrain_t));
    g->maxlength = maxlength;
    g->minlength = minlength;
    g->offset = 0;
    g->startpos = 0;

    g->phase = 0.f;
    g->phaseinc = 1.f;
    g->pan = 0.5f;
    g->amp = 1.f;
    g->speed = 1.f;

    g->length = (size_t)Rand.randint(g->minlength, g->maxlength) * (1.f / g->speed);

    g->window = window;
    g->window_phase = 0;
    g->window_phaseinc = (lpfloat_t)window->length / g->length;

    return g;
}

void grain_process(lpgrain_t * g, ringbuffer_t * rb, buffer_t * out) {
    lpfloat_t sample, pos, f, a, b;
    int is_odd, idxa, idxb, channel;

    /* FIXME -- support mismatched channels between provider & output? */
    assert(rb->buf->channels == out->channels);

    if(g->phase >= g->length) {
        g->length = (size_t)Rand.randint(g->minlength, g->maxlength) * (1.f / g->speed);
        g->offset = (size_t)Rand.randint(0, rb->buf->length - g->length - 1);
        g->startpos = rb->pos - g->length + g->offset;

        g->window_phaseinc = (lpfloat_t)g->window->length / g->length;
        g->window_phase = 0.f;
        g->pan = Rand.rand(0.f, 1.f);

        g->phase = 0.f;
        g->phaseinc = g->speed;
    }

    pos = g->startpos + g->phase;

    idxa = lpfastmod((size_t)pos, rb->buf->length);
    idxb = lpfastmod((size_t)(pos+out->channels), rb->buf->length);
    f = g->phase - (int)g->phase;

    for(channel=0; channel < out->channels; channel++) {
        a = rb->buf->data[idxa * out->channels + channel];
        b = rb->buf->data[idxb * out->channels + channel];
        sample = (1.f - f) * a + (f * b);

        sample = sample * g->amp * g->window->data[(size_t)g->window_phase];

        is_odd = (2-(channel & 1));
        if(is_odd == 1) {
            sample *= (1.f - g->pan);
        } else {
            sample *= g->pan;
        }

        out->data[channel] += sample;
    }

    g->window_phase += g->window_phaseinc;
    if(g->window_phase >= g->window->length) {
        g->window_phase = g->window_phase - g->window->length;
    }

    g->phase += g->phaseinc;
}

void grain_destroy(lpgrain_t * g) {
    MemoryPool.free(g);
}

lpcloud_t * cloud_create(int numgrains, size_t maxgrainlength, size_t mingrainlength, size_t rblength, int channels, int samplerate) {
    lpcloud_t * cloud;
    int i;

    cloud = (lpcloud_t *)MemoryPool.alloc(1, sizeof(lpcloud_t));
    cloud->window = Wavetable.create("hann", maxgrainlength);
    cloud->rb = LPRingBuffer.create(rblength, channels, samplerate);
    cloud->numgrains = numgrains;
    cloud->grains = (lpgrain_t **)MemoryPool.alloc(numgrains, sizeof(lpgrain_t *));
    cloud->grainamp = (1.f / numgrains);
    cloud->current_frame = Buffer.create(1, channels, samplerate);

    for(i=0; i < numgrains; i++) {
        cloud->grains[i] = grain_create(maxgrainlength, mingrainlength, cloud->window);
        cloud->grains[i]->amp = cloud->grainamp;
    }

    return cloud;
}

void cloud_process(lpcloud_t * c) {
    int i;

    for(i=0; i < c->current_frame->channels; i++) {
        c->current_frame->data[i] = 0.f;
    }

    for(i=0; i < c->numgrains; i++) {
        grain_process(c->grains[i], c->rb, c->current_frame);
    }
}

void cloud_destroy(lpcloud_t * c) {
    MemoryPool.free(c);
}

const grain_factory_t Grain = { grain_create, grain_process, grain_destroy };
const cloud_factory_t Cloud = { cloud_create, cloud_process, cloud_destroy };


