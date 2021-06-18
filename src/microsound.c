#include "microsound.h"

lpgrain_t * grain_create(size_t length, lpbuffer_t * window) {
    lpgrain_t * g;

    g = (lpgrain_t *)LPMemoryPool.alloc(1, sizeof(lpgrain_t));
    g->speed = 1.f;
    g->length = length * (1.f / g->speed);
    g->offset = 0;
    g->startpos = 0;

    g->phase = 0.f;
    g->phaseinc = 1.f;
    g->pan = 0.5f;
    g->amp = 1.f;

    g->window = window;
    g->window_phase = 0.f;
    g->window_phaseinc = (lpfloat_t)window->length / g->length;

    return g;
}

void grain_process(lpgrain_t * g, lpbuffer_t * rb, lpbuffer_t * out) {
    lpfloat_t sample;
    int is_odd, channel;

    /* FIXME -- support mismatched channels between provider & output? */
    assert(rb->channels == out->channels);

    if(g->window_phase >= g->window->length) {
        if(g->spread > 0) {
            g->pan = LPRand.rand(0.f, 1.f);
        }
        g->window_phase = g->window_phase - g->window->length;
        g->window_phaseinc = (lpfloat_t)g->window->length / g->length;
        g->startpos = rb->pos - g->length + g->offset;
    }

    LPTapeOsc.process(g->osc);
    for(channel=0; channel < out->channels; channel++) {
        sample = g->osc->current_frame->data[channel] * g->window->data[(size_t)g->window_phase];

        is_odd = (2-(channel & 1));
        if(is_odd == 1) {
            sample *= (1.f - g->pan);
        } else {
            sample *= g->pan;
        }

        out->data[channel] += sample;
    }

    g->window_phase += g->window_phaseinc;
    g->phase += g->phaseinc;
}

void grain_destroy(lpgrain_t * g) {
    LPMemoryPool.free(g);
}

lpcloud_t * cloud_create(int numstreams, size_t maxgrainlength, size_t mingrainlength, size_t rblength, int channels, int samplerate) {
    lpcloud_t * cloud;
    int i;
    size_t grainlength;

    cloud = (lpcloud_t *)LPMemoryPool.alloc(1, sizeof(lpcloud_t));
    cloud->window = LPWavetable.create("hann", maxgrainlength);
    cloud->rb = LPRingBuffer.create(rblength, channels, samplerate);
    cloud->maxlength = maxgrainlength;
    cloud->minlength = mingrainlength;
    cloud->numgrains = numstreams * 2;
    cloud->grains = (lpgrain_t **)LPMemoryPool.alloc(cloud->numgrains, sizeof(lpgrain_t *));
    cloud->grainamp = (1.f / cloud->numgrains);
    cloud->current_frame = LPBuffer.create(1, channels, samplerate);

    grainlength = (size_t)LPRand.randint(cloud->minlength, cloud->maxlength);

    for(i=0; i < cloud->numgrains; i += 2) {
        cloud->grains[i] = grain_create(grainlength, cloud->window);
        cloud->grains[i]->amp = cloud->grainamp;
        /*cloud->grains[i]->offset = (size_t)LPRand.randint(0, cloud->rb->length - cloud->grains[i]->length - 1);*/

        cloud->grains[i+1] = grain_create(grainlength, cloud->window);
        cloud->grains[i+1]->amp = cloud->grainamp;
        /*cloud->grains[i+1]->phase = cloud->grains[i+1]->length/2.f;*/
        cloud->grains[i+1]->window_phase = cloud->window->length/2.f;
        /*cloud->grains[i+1]->offset = (size_t)LPRand.randint(0, cloud->rb->length - cloud->grains[i+1]->length - 1);*/
    }

    return cloud;
}

void cloud_process(lpcloud_t * c) {
    int i;

    for(i=0; i < c->current_frame->channels; i++) {
        c->current_frame->data[i] = 0.f;
    }

    for(i=0; i < c->numgrains; i++) {
        c->grains[i]->length = (size_t)LPRand.randint(c->minlength, c->maxlength) * (1.f / c->grains[i]->speed);
        c->grains[i]->offset = (size_t)LPRand.randint(0, c->rb->length - c->grains[i]->length - 1);

        grain_process(c->grains[i], c->rb, c->current_frame);
    }
}

void cloud_destroy(lpcloud_t * c) {
    int i;
    for(i=0; i < c->numgrains; i++) {
        LPMemoryPool.free(c->grains[i]);
    }
    LPMemoryPool.free(c->grains);
    LPBuffer.destroy(c->window);
    LPBuffer.destroy(c->rb);
    LPBuffer.destroy(c->current_frame);
    LPMemoryPool.free(c);
}

const lpgrain_factory_t LPGrain = { grain_create, grain_process, grain_destroy };
const lpcloud_factory_t LPCloud = { cloud_create, cloud_process, cloud_destroy };


