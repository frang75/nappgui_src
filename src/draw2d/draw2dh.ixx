/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw2dh.ixx
 *
 */

/* Operating system 2D drawing support */
/* Public types, but hidden in distributions */
/* NO Documentation required */

#ifndef __OSDRAWH_IXX__
#define __OSDRAWH_IXX__

typedef enum _ffsize_t
{
    ekMINI = 1,
    ekSMALL,
    ekREGULAR
} fsize_t;

typedef enum _label_flag_t
{
    ekLBFLAG        = 0,
    ekLBSING        = 0,
    ekLBMULT        = 1,
    ekLBTYPE        = 1
} label_flag_t;

typedef enum _button_flag_t
{
    ekBTFLAG        = 0,
    ekBTPUSH        = 0,
    ekBTCHECK2      = 1,
    ekBTCHECK3      = 2,
    ekBTRADIO       = 3,
    ekBTFLAT        = 4,
    ekBTFLATGLE     = 5,
    ekBTHEADER      = 6,
    ekBTTYPE        = 7
} button_flag_t;

typedef enum _popup_flag_t
{
    ekPUFLAG = 0
} popup_flag_t;

typedef enum _edit_flag_t
{
    ekEDFLAG    = 0,
    ekEDSING    = 0,
    ekEDMULT    = 1,
    ekEDTYPE    = 1,
    ekEDAUTOSEL = 4
} edit_flag_t;

typedef enum _combo_flag_t
{
    ekCBFLAG    = 0
} combo_flag_t;

typedef enum _slider_flag_t
{
    ekSLFLAG    = 0,
    ekSLHORZ    = 0,
    ekSLVERT    = 1,
    ekSLTYPE    = 1
} slider_flag_t;

typedef enum _updown_flag_t
{
    ekUPFLAG    = 0
} updown_flag_t;

typedef enum _progress_flag_t
{
    ekPGFLAG    = 0,
    ekPGHORZ    = 0,
    ekPGVERT    = 1,
    ekPGTYPE    = 1
} progress_flag_t;

typedef enum _tview_flag_t
{
    ekTVFLAG    = 0
} tview_flag_t;

typedef enum _split_flag_t
{
    ekSPFLAG    = 0,
    ekSPVERT    = 0,
    ekSPHORZ    = 1,
    ekSPTYPE    = 1,
    ekSPPROP    = 2,
    ekSPLEFT    = 3,
    ekSPRIGHT   = 4
} split_flag_t;

typedef enum _menu_flag_t
{
    ekMNFLAG = 0,
    ekMNITEM,
    ekMNSEPARATOR
} menu_flag_t;

typedef enum _guiprop_t
{
    ekGUI_PROPERTY_RESIZE = 0,
    ekGUI_PROPERTY_CHILDREN = 1,
    ekGUI_TEXT_FAMILY,
    ekGUI_TEXT_UNITS,
    ekGUI_TEXT_SIZE,
    ekGUI_TEXT_STYLE,
    ekGUI_TEXT_COLOR,
    ekGUI_TEXT_BGCOLOR,
    ekGUI_TEXT_PGCOLOR,
    ekGUI_TEXT_PARALIGN,
    ekGUI_TEXT_LSPACING,
    ekGUI_TEXT_BFPARSPACE,
    ekGUI_TEXT_AFPARSPACE,
    ekGUI_TEXT_VSCROLL
} guiprop_t;

typedef enum _op_t
{
    ekOPADD = 1,
    ekOPDEL,
    ekOPINS,
    ekOPSET
} op_t;

typedef enum _syscolor_t
{
    ekSYS_DARKMODE = 0,
    ekSYS_LABEL,
    ekSYS_VIEW,
    ekSYS_LINE,
    ekSYS_LINK,
    ekSYS_BORDER
} syscolor_t;

typedef struct _gui_context_t GuiContext;

#endif
