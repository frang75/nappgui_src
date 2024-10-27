/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscontrol.inl
 *
 */

/* Control common functions */

#include "osgui.ixx"

__EXTERN_C

gui_type_t _oscontrol_type(const OSControl *control);

const char_t *_oscontrol_type_str(const gui_type_t type);

OSControl *_oscontrol_parent(const OSControl *control);

void _oscontrol_frame(const OSControl *control, OSFrame *rect);

void _oscontrol_set_can_focus(OSControl *control, const bool_t can_focus);

OSWidget *_oscontrol_focus_widget(const OSControl *control);

bool_t _oscontrol_widget_visible(const OSWidget *widget);

bool_t _oscontrol_widget_enable(const OSWidget *widget);

void _oscontrol_detach_and_destroy(OSControl **control, OSPanel *panel);

__END_C
