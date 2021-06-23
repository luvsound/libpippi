#include "microsound.h"

lpgrain_t * grain_create(lpfloat_t length, lpbuffer_t * window, lpbuffer_t * source) {
    lpgrain_t * g;

    g = (lpgrain_t *)LPMemoryPool.alloc(1, sizeof(lpgrain_t));
    g->speed = 1.f;
    /*g->length = length * (1.f / g->speed);*/
    g->length = length;
    g->offset = 0.f;
    g->osc = LPTapeOsc.create(source, length);
    g->osc->offset = -length;

    g->pan = 0.5f;
    g->amp = 1.f;
    g->spread = 0.f;
    g->jitter = 0.f;

    g->window = window;
    g->window_phase = 0.f;
    g->window_phaseinc = window->length / g->length;

    return g;
}

void grain_process(lpgrain_t * g, lpbuffer_t * out) {
    lpfloat_t sample;
    int is_odd, c;

    if(g->window_phase >= g->window->length) {
        if(g->spread > 0) {
            g->pan = LPRand.rand(0.f, 1.f);
        }
        g->window_phase = g->window_phase - g->window->length;
        g->window_phaseinc = (lpfloat_t)g->window->length / g->length;
        g->osc->offset = -(g->length + g->offset);
        g->osc->range = g->length;
        g->osc->freq = 1.f / g->length;
        g->osc->speed = g->speed;
    }

    LPTapeOsc.process(g->osc);
    for(c=0; c < out->channels; c++) {
        /*sample = g->osc->current_frame->data[channel] * g->window->data[(size_t)g->window_phase];*/
        sample = g->osc->current_frame->data[c] * LPInterpolation.linear(g->window, g->window_phase);

        if(g->pan != 0.5f) {
            is_odd = (2-(c & 1));
            if(is_odd == 1) {
                sample *= (1.f - g->pan);
            } else {
                sample *= g->pan;
            }
        }

        out->data[c] += sample;
    }

    g->window_phase += g->window_phaseinc;
}

void grain_destroy(lpgrain_t * g) {
    LPMemoryPool.free(g);
}

lpcloud_t * cloud_create(int numstreams, size_t maxgrainlength, size_t mingrainlength, size_t rblength, int channels, int samplerate) {
    lpcloud_t * cloud;
    int i;
    lpfloat_t grainlength;

    cloud = (lpcloud_t *)LPMemoryPool.alloc(1, sizeof(lpcloud_t));
    cloud->window = LPWavetable.create("hann", 4096);
    cloud->rb = LPRingBuffer.create(rblength, channels, samplerate);
    cloud->maxlength = maxgrainlength;
    cloud->minlength = mingrainlength;
    cloud->numgrains = numstreams * 2;
    cloud->grains = (lpgrain_t **)LPMemoryPool.alloc(cloud->numgrains, sizeof(lpgrain_t *));
    cloud->grainamp = (1.f / cloud->numgrains);
    cloud->current_frame = LPBuffer.create(1, channels, samplerate);

    grainlength = LPRand.rand((lpfloat_t)cloud->minlength, (lpfloat_t)cloud->maxlength);

    for(i=0; i < cloud->numgrains; i += 2) {
        cloud->grains[i] = grain_create(grainlength, cloud->window, cloud->rb);
        cloud->grains[i]->amp = cloud->grainamp;

        cloud->grains[i+1] = grain_create(grainlength, cloud->window, cloud->rb);
        cloud->grains[i+1]->amp = cloud->grainamp;
        cloud->grains[i+1]->window_phase = cloud->window->length/2.f;
    }

    return cloud;
}

void cloud_process(lpcloud_t * c) {
    int i;

    for(i=0; i < c->current_frame->channels; i++) {
        c->current_frame->data[i] = 0.f;
    }

    for(i=0; i < c->numgrains; i++) {
        c->grains[i]->speed = LPRand.rand(1.f, 3.f);
        /*c->grains[i]->length = LPRand.rand(c->minlength, c->maxlength) * (1.f / c->grains[i]->speed);*/
        c->grains[i]->length = LPRand.rand(c->minlength, c->maxlength);
        c->grains[i]->offset = LPRand.rand(0.f, c->rb->length - c->grains[i]->length - 1.f);

        grain_process(c->grains[i], c->current_frame);
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


