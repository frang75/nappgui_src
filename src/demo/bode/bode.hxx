/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bode.hxx
 *
 */

/* Bode Types */

#ifndef __TYPES_HXX__
#define __TYPES_HXX__

#include "gui.hxx"

typedef struct _params_t Params;
typedef struct _model_t Model;
typedef struct _plot_t Plot;
typedef struct _ctrl_t Ctrl;

struct _params_t
{
	real32_t P[5];
	real32_t Q[9];
	real32_t K[3];
	real32_t T;
	real32_t R;
	real32_t KRg[6];
};

struct _model_t
{
	V2Df wpos;
	S2Df wsize;
	Params cparams;
	Params sparams;
};

#endif
