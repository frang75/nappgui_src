/* Products Types */

#ifndef __TYPES_HXX__
#define __TYPES_HXX__

#include <gui/gui.hxx>

typedef enum _wserv_t
{
    ekWS_CONNECT = 1,
    ekWS_JSON,
    ekWS_ACCESS,
    ekWS_OK
} wserv_t;

typedef struct _model_t Model;
typedef struct _product_t Product;
typedef struct _ctrl_t Ctrl;

__EXTERN_C

extern color_t kHOLDER;
extern color_t kEDITBG;
extern color_t kSTATBG;
extern color_t kSTATSK;
extern color_t kTXTRED;

__END_C

#endif
