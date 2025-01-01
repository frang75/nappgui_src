/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osglobals.inl
 *
 */

/* Operating System globals */

void _osglobals_init(void);

void _osglobals_finish(void);

uint32_t _osglobals_check_width(void);

uint32_t _osglobals_check_height(void);

NSRect _osglobals_check_rect(void);

NSRect _osglobals_header_rect(void);

NSImage *_osglobals_checkbox_image(const bool_t pressed, const ctrl_state_t state);

NSImage *_osglobals_header_image(const bool_t pressed);

const CGFloat *_osglobals_text_color(void);

const CGFloat *_osglobals_seltx_color(void);

const CGFloat *_osglobals_hottx_color(void);

const CGFloat *_osglobals_textbackdrop_color(void);

const CGFloat *_osglobals_seltxbackdrop_color(void);

const CGFloat *_osglobals_hottxbackdrop_color(void);

const CGFloat *_osglobals_back_color(void);

const CGFloat *_osglobals_hotbg_color(void);

const CGFloat *_osglobals_selbg_color(void);

const CGFloat *_osglobals_backbackdrop_color(void);

const CGFloat *_osglobals_hotbgbackdrop_color(void);

const CGFloat *_osglobals_selbgbackdrop_color(void);

color_t _osglobals_focus_color(void);
