/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: higram.inl
 *
 */

/* Histograms */

#include "draw2d.ixx"

__EXTERN_C

Higram *higram_create(const Pixbuf *pixbuf);

Higram *higram_from_gray(const uint32_t *gray_256);

void higram_destroy(Higram **higram);

uint32_t higram_values(const Higram *higram);

uint32_t higram_channels(const Higram *higram);

uint32_t higram_max(const Higram *higram);

uint8_t higram_mean(const Higram *higram, const uint32_t channel);

const uint32_t *higram_red(const Higram *higram);

const uint32_t *higram_green(const Higram *higram);

const uint32_t *higram_blue(const Higram *higram);

const uint32_t *higram_alpha(const Higram *higram);

const uint32_t *higram_gray(const Higram *higram);

const uint32_t *higram_index(const Higram *higram);

__END_C

