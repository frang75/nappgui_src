/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscontrol.c
 *
 */

/* Control common functions */

#include "oscontrol.inl"
#include "osbutton.inl"
#include "oscombo.inl"
#include "osedit.inl"
#include "oslabel.inl"
#include "ospanel.inl"
#include "ospopup.inl"
#include "osprogress.inl"
#include "osslider.inl"
#include "ossplit.inl"
#include "ostext.inl"
#include "osupdown.inl"
#include "osview.inl"
#include "osweb.inl"
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

const char_t *_oscontrol_type_str(const gui_type_t type)
{
    switch (type)
    {
    case ekGUI_TYPE_LABEL:
        return "OSLabel";
    case ekGUI_TYPE_BUTTON:
        return "OSButton";
    case ekGUI_TYPE_POPUP:
        return "OSPopUp";
    case ekGUI_TYPE_EDITBOX:
        return "OSEdit";
    case ekGUI_TYPE_COMBOBOX:
        return "OSComboBox";
    case ekGUI_TYPE_SLIDER:
        return "OSSlider";
    case ekGUI_TYPE_UPDOWN:
        return "OSUpDown";
    case ekGUI_TYPE_PROGRESS:
        return "OSProgress";
    case ekGUI_TYPE_TEXTVIEW:
        return "OSTextView";
    case ekGUI_TYPE_WEBVIEW:
        return "OSWebView";
    case ekGUI_TYPE_TREEVIEW:
        return "OSTreeView";
    case ekGUI_TYPE_BOXVIEW:
        return "OSBoxView";
    case ekGUI_TYPE_SPLITVIEW:
        return "OSSplitView";
    case ekGUI_TYPE_CUSTOMVIEW:
        return "OSView";
    case ekGUI_TYPE_PANEL:
        return "OSPanel";
    case ekGUI_TYPE_LINE:
        return "OSLine";
    case ekGUI_TYPE_HEADER:
        return "OSHeader";
    case ekGUI_TYPE_TOOLBAR:
        return "OSToolbar";
    case ekGUI_TYPE_WINDOW:
        return "OSWindow";
    default:
        cassert_default(type);
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

void _oscontrol_detach_and_destroy(OSControl **control, OSPanel *panel)
{
    gui_type_t type = ENUM_MAX(gui_type_t);
    cassert_no_null(control);
    type = _oscontrol_type(*control);
    switch (type)
    {
    case ekGUI_TYPE_LABEL:
        _oslabel_detach_and_destroy(dcast(control, OSLabel), panel);
        break;
    case ekGUI_TYPE_BUTTON:
        _osbutton_detach_and_destroy(dcast(control, OSButton), panel);
        break;
    case ekGUI_TYPE_POPUP:
        _ospopup_detach_and_destroy(dcast(control, OSPopUp), panel);
        break;
    case ekGUI_TYPE_EDITBOX:
        _osedit_detach_and_destroy(dcast(control, OSEdit), panel);
        break;
    case ekGUI_TYPE_COMBOBOX:
        _oscombo_detach_and_destroy(dcast(control, OSCombo), panel);
        break;
    case ekGUI_TYPE_SLIDER:
        _osslider_detach_and_destroy(dcast(control, OSSlider), panel);
        break;
    case ekGUI_TYPE_UPDOWN:
        _osupdown_detach_and_destroy(dcast(control, OSUpDown), panel);
        break;
    case ekGUI_TYPE_PROGRESS:
        _osprogress_detach_and_destroy(dcast(control, OSProgress), panel);
        break;
    case ekGUI_TYPE_TEXTVIEW:
        _ostext_detach_and_destroy(dcast(control, OSText), panel);
        break;
    case ekGUI_TYPE_CUSTOMVIEW:
        _osview_detach_and_destroy(dcast(control, OSView), panel);
        break;
    case ekGUI_TYPE_PANEL:
        _ospanel_detach_and_destroy(dcast(control, OSPanel), panel);
        break;
    case ekGUI_TYPE_SPLITVIEW:
        _ossplit_detach_and_destroy(dcast(control, OSSplit), panel);
        break;
    case ekGUI_TYPE_WEBVIEW:
        _osweb_detach_and_destroy(dcast(control, OSWeb), panel);
        break;
    case ekGUI_TYPE_TREEVIEW:
    case ekGUI_TYPE_BOXVIEW:
    case ekGUI_TYPE_LINE:
    case ekGUI_TYPE_HEADER:
    case ekGUI_TYPE_WINDOW:
    case ekGUI_TYPE_TOOLBAR:
    default:
        cassert_default(type);
    }
}
