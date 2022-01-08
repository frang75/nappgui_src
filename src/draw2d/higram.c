/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: higram.c
 *
 */

/* Histograms */

#include "higram.inl"
#include "pixbuf.h"
#include "bmem.h"
#include "cassert.h"
#include "heap.h"

struct _higram_t
{
    uint32_t nvalues;
    uint32_t nchannels;
    uint32_t max;
};

/*---------------------------------------------------------------------------*/

#define i_DATA(higram) ((uint32_t*)((byte_t*)(higram) + sizeof(Higram)))

/*---------------------------------------------------------------------------*/

static __INLINE uint32_t i_nvalues(const pixformat_t format)
{
    switch (format) {
    case ekINDEX1:
        return 2;
    case ekINDEX2:
        return 4;
    case ekINDEX4:
        return 16;
    case ekINDEX8:
    case ekGRAY8:
    case ekRGB24:
    case ekRGBA32:
        return 256;
    case ekFIMAGE:
    cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static __INLINE uint32_t i_channels(const pixformat_t format)
{
    switch (format) {
    case ekINDEX1:
    case ekINDEX2:
    case ekINDEX4:
    case ekINDEX8:
    case ekGRAY8:
        return 1;
    case ekRGB24:
        return 3;
    case ekRGBA32:
        return 4;
    case ekFIMAGE:
    cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static __INLINE void i_compute_256(uint32_t *higram, const byte_t *pixdata, const uint32_t offset, const uint32_t n, uint32_t *max)
{
    register uint32_t i, lmax = *max;
    bmem_zero_n(higram, 256, uint32_t);
    for (i = 0; i < n; ++i, pixdata += offset)
    {
        higram[*pixdata] += 1;
        if (higram[*pixdata] > lmax)
            lmax = higram[*pixdata];
    }

    *max = lmax;
}

/*---------------------------------------------------------------------------*/

Higram *higram_create(const Pixbuf *pixbuf)
{
    pixformat_t format = pixbuf_format(pixbuf);
    uint32_t nvalues = i_nvalues(format);
    uint32_t channels = i_channels(format);
    uint32_t memsize = (uint32_t)(sizeof(Higram) + nvalues * channels * sizeof(uint32_t));
    const byte_t *pixdata = pixbuf_cdata(pixbuf);
    uint32_t n = pixbuf_size(pixbuf);
    Higram *higram = (Higram*)heap_malloc(memsize, "Higram");
    higram->nvalues = nvalues;
    higram->nchannels = channels;
    higram->max = 0;

    switch(format) {
    case ekRGB24:
        cassert(nvalues == 256);
        cassert(channels == 3);
        i_compute_256(i_DATA(higram), pixdata, 3, n, &higram->max);
        i_compute_256(i_DATA(higram) + 256, pixdata + 1, 3, n, &higram->max);
        i_compute_256(i_DATA(higram) + 512, pixdata + 2, 3, n, &higram->max);
        break;

    case ekRGBA32:
        cassert(nvalues == 256);
        cassert(channels == 4);
        i_compute_256(i_DATA(higram) + 0,   pixdata + 0, 4, n, &higram->max);
        i_compute_256(i_DATA(higram) + 256, pixdata + 1, 4, n, &higram->max);
        i_compute_256(i_DATA(higram) + 512, pixdata + 2, 4, n, &higram->max);
        i_compute_256(i_DATA(higram) + 768, pixdata + 3, 4, n, &higram->max);
        break;

    case ekGRAY8:
    case ekINDEX8:
        cassert(nvalues == 256);
        cassert(channels == 1);
        i_compute_256(i_DATA(higram), pixdata, 1, n, &higram->max);
        break;

    case ekINDEX4:
    case ekINDEX2:
    case ekINDEX1:
        // TODO

    case ekFIMAGE:
    cassert_default();
    }

    return higram;
}

/*---------------------------------------------------------------------------*/

Higram *higram_from_gray(const uint32_t *gray_256)
{
    uint32_t nvalues = i_nvalues(ekGRAY8);
    uint32_t channels = i_channels(ekGRAY8);
    uint32_t memsize = (uint32_t)(sizeof(Higram) + nvalues * channels * sizeof(uint32_t));
    Higram *higram = (Higram*)heap_malloc(memsize, "Higram");
    uint32_t i;

    higram->nvalues = nvalues;
    higram->nchannels = channels;
    higram->max = 0;
    bmem_copy_n(i_DATA(higram), gray_256, 256, uint32_t);

    for (i = 0; i < 256; ++i)
    {
        if (gray_256[i] > higram->max)
            higram->max = gray_256[i];
    }

    return higram;
}

/*---------------------------------------------------------------------------*/

void higram_destroy(Higram **higram)
{
    uint32_t memsize = 0;
    cassert_no_null(higram);
    cassert_no_null(*higram);
    memsize = (uint32_t)(sizeof(Higram) + (*higram)->nvalues * (*higram)->nchannels * sizeof(uint32_t));
    heap_free((byte_t**)higram, memsize, "Higram");
}

/*---------------------------------------------------------------------------*/

uint32_t higram_values(const Higram *higram)
{
    cassert_no_null(higram);
    return higram->nvalues;
}

/*---------------------------------------------------------------------------*/

uint32_t higram_channels(const Higram *higram)
{
    cassert_no_null(higram);
    return higram->nchannels;
}

/*---------------------------------------------------------------------------*/

uint32_t higram_max(const Higram *higram)
{
    cassert_no_null(higram);
    return higram->max;
}

/*---------------------------------------------------------------------------*/

uint8_t higram_mean(const Higram *higram, const uint32_t channel)
{
    const uint32_t *v = NULL;
    uint64_t accum = 0, total = 0, mean = 0;
    uint64_t i;
    cassert_no_null(higram);
    cassert(channel < higram->nchannels);
    v = i_DATA(higram) + channel * 256;

    for (i = 0; i < 256; ++i)
    {
        accum += i * v[i];
        total += v[i];
    }

    cassert(total > 0);
    mean = accum / total;
    cassert(mean < 256);
    return (uint8_t)mean;
}

/*---------------------------------------------------------------------------*/

const uint32_t *higram_red(const Higram *higram)
{
    cassert_no_null(higram);
    cassert(higram->nchannels == 3 || higram->nchannels == 4);
    cassert(higram->nvalues == 256);
    return i_DATA(higram);
}

/*---------------------------------------------------------------------------*/

const uint32_t *higram_green(const Higram *higram)
{
    cassert_no_null(higram);
    cassert(higram->nchannels == 3 || higram->nchannels == 4);
    cassert(higram->nvalues == 256);
    return i_DATA(higram) + 256;
}

/*---------------------------------------------------------------------------*/

const uint32_t *higram_blue(const Higram *higram)
{
    cassert_no_null(higram);
    cassert(higram->nchannels == 3 || higram->nchannels == 4);
    cassert(higram->nvalues == 256);
    return i_DATA(higram) + 512;
}

/*---------------------------------------------------------------------------*/

const uint32_t *higram_alpha(const Higram *higram)
{
    cassert_no_null(higram);
    cassert(higram->nchannels == 4);
    cassert(higram->nvalues == 256);
    return i_DATA(higram) + 768;
}

/*---------------------------------------------------------------------------*/

const uint32_t *higram_gray(const Higram *higram)
{
    cassert_no_null(higram);
    cassert(higram->nchannels == 1);
    cassert(higram->nvalues == 256);
    return i_DATA(higram);
}

/*---------------------------------------------------------------------------*/

const uint32_t *higram_index(const Higram *higram)
{
    cassert_no_null(higram);
    cassert(higram->nchannels == 1);
    return i_DATA(higram);
}

/*---------------------------------------------------------------------------*/
