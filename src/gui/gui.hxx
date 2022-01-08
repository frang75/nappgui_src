/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: gui.hxx
 * https://nappgui.com/en/gui/gui.html
 *
 */

/* Graphics User Interface */

#ifndef __GUI_HXX__
#define __GUI_HXX__

#include "draw2d.hxx"

typedef enum _orient_t
{
    ekHORIZONTAL = 1,
    ekVERTICAL
} orient_t;

typedef enum _state_t
{
    ekOFF = 0,
    ekON,
    ekMIXED
} state_t;

typedef enum _mouse_t
{
    ekMLEFT = 1,
    ekMRIGHT,
    ekMMIDDLE
} mouse_t;

typedef enum _cursor_t
{
    ekCARROW = 1,
    ekCHAND,
    ekCIBEAM,
    ekCCROSS,
    ekCSIZEWE,
    ekCSIZENS,
    ekCUSER
} cursor_t;

typedef enum _close_t
{
    ekCLESC = 1,
    ekCLINTRO,
    ekCLBUTTON,
    ekCLDEACT
} close_t;

typedef enum _scale_t
{
    ekAUTO = 1,
    ekSNONE,
    ekASPECT,
    ekASPECTDW
} scale_t;

typedef enum _table_flag_t
{
    ekTBTEXT        = 0x1,
    ekTBICON16      = 0x2,
    ekTBEDIT        = 0x4,
    ekTBCOMBO       = 0x8,
    ekTBPOPUP       = 0x10,
    ekTBRADIO       = 0x20,
    ekTBCHECK       = 0x40,
    ekTBTOOLTIP     = 0x80,
    ekTBBGCOLOR     = 0x100,
    ekTBROWALT      = 0x200,
    ekTBROWPRESEL   = 0x400
} table_flag_t;

typedef enum _window_flag_t
{
    ekWNFLAG            = 0,
    ekWNEDGE            = 1 << 0,
    ekWNTITLE           = 1 << 1,
    ekWNMAX             = 1 << 2,
    ekWNMIN             = 1 << 3,
    ekWNCLOSE           = 1 << 4,
    ekWNRES             = 1 << 5,
    ekWNRETURN          = 1 << 6,
    ekWNESC             = 1 << 7,
    ekWNSTD             = ekWNTITLE | ekWNMIN | ekWNCLOSE,
    ekWNSRES            = ekWNSTD | ekWNMAX | ekWNRES
} window_flag_t;

typedef enum _gevent_t
{
    ekEVLABEL = 0x400,
    ekEVBUTTON,
    ekEVPOPUP,
    ekEVLISTBOX,
    ekEVSLIDER,
    ekEVUPDOWN,
    ekEVTXTFILTER,
    ekEVTXTCHANGE,
    ekEVFOCUS,
    ekEVMENU,
    ekEVDRAW,
    ekEVRESIZE,
    ekEVENTER,
    ekEVEXIT,
    ekEVMOVED,  
    ekEVDOWN,
    ekEVUP,
    ekEVCLICK,
    ekEVDRAG,
    ekEVWHEEL,
    ekEVKEYDOWN,
    ekEVKEYUP,
    ekEVWNDMOVED,
    ekEVWNDSIZING,
    ekEVWNDSIZE,
    ekEVWNDCLOSE,
    ekEVCOLOR,
    ekEVTHEME,
    ekEVOBJCHANGE,

    ekEVTBLSIZE,
    ekEVTBLNROWS,
    ekEVTBLROW,
    ekEVTBLCELL,
    ekEVHEADSIZE,
    ekEVHEADCLICK
} event_t;

typedef struct _control_t Control;
typedef struct _label_t Label;
typedef struct _button_t Button;
typedef struct _popup_t PopUp;
typedef struct _edit_t Edit;
typedef struct _combo_t Combo;
typedef struct _listbox_t ListBox;
typedef struct _updown_t UpDown;
typedef struct _slider_t Slider;
typedef struct _progress_t Progress;
typedef struct _view_t View;
typedef struct _textview_t TextView;
typedef struct _imageview_t ImageView;
typedef struct _tableview_t TableView;
typedef struct _splitview_t SplitView;
typedef struct _layout_t Layout;
typedef struct _cell_t Cell;
typedef struct _panel_t Panel;
typedef struct _window_t Window;
typedef struct _menu_t Menu;
typedef struct _menu_item_t MenuItem;
typedef struct _evbutton_t EvButton;
typedef struct _evslider_t EvSlider;
typedef struct _evtext_t EvText;
typedef struct _evtextfilter_t EvTextFilter;
typedef struct _evdraw_t EvDraw;
typedef struct _evmouse_t EvMouse;
typedef struct _evwheel_t EvWheel;
typedef struct _evkey_t EvKey;
typedef struct _evpos_t EvPos;
typedef struct _evsize_t EvSize;
typedef struct _evwinclose_t EvWinClose;
typedef struct _evmenu_t EvMenu;

typedef struct _evheader_t EvHeader;
typedef struct _evtbpos_t EvTbPos;
typedef struct _evtbrow_t EvTbRow;
typedef struct _evtbcell_t EvTbCell;

typedef ResPack*(*FPtr_respack)(const char_t *locale);

struct _evbutton_t
{
    uint32_t index;
    state_t state;
    const char_t *text;
};

struct _evslider_t
{
    real32_t pos;
    real32_t incr;
    uint32_t step;
};

struct _evtext_t
{
    const char_t *text;
    uint32_t cpos;
};

#define kTEXTFILTER_SIZE    1024

struct _evtextfilter_t
{
    bool_t apply;
    char_t text[kTEXTFILTER_SIZE];
    uint32_t cpos;
};

struct _evdraw_t
{
    DCtx *ctx;
    real32_t x;
    real32_t y;
    real32_t width;
    real32_t height;
};

struct _evmouse_t
{
    real32_t x;
    real32_t y;
    mouse_t button;
    uint32_t count;
};

struct _evwheel_t
{
    real32_t x;
    real32_t y;
    real32_t dx;
    real32_t dy;
    real32_t dz;
};

struct _evkey_t
{
    vkey_t key;
};

struct _evpos_t
{
    real32_t x;
    real32_t y;
};

struct _evsize_t
{
    real32_t width;
    real32_t height;
};

struct _evwinclose_t
{
    close_t origin;
};

struct _evmenu_t
{
    uint32_t index;
    state_t state;
    const char_t *str;
};

struct _evheader_t
{
    uint32_t index;
    real32_t width;
};

struct _evtbpos_t
{
    uint32_t col;
    uint32_t row;
};

struct _evtbrow_t
{
    color_t bgcolor;
};

struct _evtbcell_t
{        
    uint32_t flags;
    const char_t *text;
    bool_t checked;
    color_t bgcolor;
};

DeclSt(FPtr_respack);

#endif
