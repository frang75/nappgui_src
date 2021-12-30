/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: gui.cpp
 *
 */

/* Graphics User Interface */

#include "gui.h"
#include "gui.inl"
#include "draw.inl"
#include "draw2d.inl"
#include "guicontexth.inl"
#include "gbind.inl"
#include "res_assert.h"
#include "menu.inl"
#include "respack.inl"
#include "respack.h"
#include "window.inl"

#include "arrpt.h"
#include "arrst.h"
#include "bfile.h"
#include "bmem.h"
#include "bproc.h"
#include "bstd.h"
#include "button.h"
#include "cassert.h"
#include "color.h"
#include "event.h"
#include "font.h"
#include "heap.h"
#include "hfile.h"
#include "hfile.inl"
#include "image.h"
#include "imageview.h"
#include "label.h"
#include "layout.h"
#include "log.h"
#include "draw2d.h"
#include "panel.h"
#include "ptr.h"
#include "strings.h"
#include "v2d.h"
#include "window.h"

typedef struct _transtion_t Transition;
typedef struct _curicon_t CurIcon;
typedef struct _coloralt_t ColorAlt;

typedef enum _icon_t
{
    ekICON_ASSERT,
    ekICON_CRASH,
    ekICON_SYSTEM
} icon_t;

struct _transtion_t
{
    Listener *listener;
    void *owner;
};

struct _curicon_t
{
    cursor_t type;
    const Image *image;
    const Cursor *cursor;
};

struct _coloralt_t
{
    color_t light;
    color_t dark;
};

DeclSt(Transition);
DeclSt(CurIcon);
DeclSt(ColorAlt);
DeclPt(Window);
DeclPt(Menu);

#define kFIRST_COLOR_ALT 16
static bool_t i_kDARK_MODE = FALSE;

/*---------------------------------------------------------------------------*/

class Gui
{
public:
    static uint32_t NUM_USERS;

    Font *default_font;
    String *current_lang;
    ArrSt(ColorAlt) *altcolors;
    ArrSt(Transition) *transitions;
    ArrSt(CurIcon) *cursors;
    ArrPt(ResPack) *packs;
    ArrSt(FPtr_respack) *func_packs;
    ArrPt(Window) *windows;
    ArrPt(Menu) *menus;
    String *app_name;
    Window *assert_window;
    bool_t show_assert_window;
    bool_t write_assert_log;
    bool_t in_assert;
    Listener *OnTheme;
    Listener *OnTranslate;
    Listener *OnMenubar;
    Listener *OnFinish;
    FPtr_destroy func_destroy_cursor;

    ~Gui()
    {
        if (NUM_USERS != 0)
        {
            log_printf("Error! gui is not properly closed (%d)\n", NUM_USERS);
        }
    }
};

static Gui i_GUI;
uint32_t Gui::NUM_USERS = 0;
static void i_assert_handler(void*, const uint32_t, const char_t*, const char_t*, const char_t*, const uint32_t);

/*---------------------------------------------------------------------------*/

static void i_remove_transition(Transition *transition)
{
    cassert_no_null(transition);
    listener_destroy(&transition->listener);
}

/*---------------------------------------------------------------------------*/

static void i_remove_curicon(CurIcon *cursor)
{
    cassert_no_null(cursor);
    i_GUI.func_destroy_cursor((void**)&cursor->cursor);
}

/*---------------------------------------------------------------------------*/

void gui_update_transitions(const real64_t prtime, const real64_t crtime)
{
    if (arrst_size(i_GUI.transitions, Transition) > 0)
    {
        EvTransition params;
        params.prtime = prtime;
        params.crtime = crtime;
        params.cframe = UINT32_MAX;
        arrst_foreach(transition, i_GUI.transitions, Transition)
            listener_event(transition->listener, ekGUI_EVENT_ANIMATION, NULL, &params, NULL, void, EvTransition, void);
        arrst_end()
    }
}

/*---------------------------------------------------------------------------*/

static void i_logfile(void)
{
    String *logfile = hfile_appdata("log.txt");
    log_file(tc(logfile));
    log_printf("Starting log for '%s'", tc(i_GUI.app_name));
    str_destroy(&logfile);
}

/*---------------------------------------------------------------------------*/

void gui_start(void)
{
    if (i_GUI.NUM_USERS == 0)
    {
        draw2d_start();
        i_GUI.default_font = NULL;
        i_GUI.current_lang = str_c("");
        i_GUI.altcolors = arrst_create(ColorAlt);
        i_GUI.transitions = arrst_create(Transition);
        i_GUI.cursors = arrst_create(CurIcon);
        i_GUI.packs = arrpt_create(ResPack);
        i_GUI.func_packs = arrst_create(FPtr_respack);
        i_GUI.windows = arrpt_create(Window);
        i_GUI.menus = arrpt_create(Menu);
        i_GUI.show_assert_window = TRUE;
        i_GUI.write_assert_log = TRUE;
        i_GUI.in_assert = FALSE;
        i_GUI.assert_window = NULL;
        i_GUI.app_name = hfile_exename();
        i_GUI.OnTheme = NULL;
        i_GUI.OnTranslate = NULL;
        i_GUI.OnMenubar = NULL;
        i_GUI.OnFinish = NULL;
        i_GUI.func_destroy_cursor = NULL;
        cassert_set_func(NULL, i_assert_handler);
        i_logfile();
    }

    i_GUI.NUM_USERS += 1;
}

/*---------------------------------------------------------------------------*/

void gui_finish(void)
{
    cassert(i_GUI.NUM_USERS > 0);
    i_GUI.NUM_USERS -= 1;
    if (i_GUI.NUM_USERS == 0)
    {
        ptr_destopt(window_destroy, &i_GUI.assert_window, Window);
        ptr_destopt(font_destroy, &i_GUI.default_font, Font);
        str_destroy(&i_GUI.app_name);
        str_destroy(&i_GUI.current_lang);
        cassert(arrst_size(i_GUI.transitions, Transition) == 0);
        arrst_destroy(&i_GUI.altcolors, NULL, ColorAlt);
        arrst_destroy(&i_GUI.transitions, NULL, Transition);
        arrst_destroy(&i_GUI.cursors, i_remove_curicon, CurIcon);
        arrpt_destroy(&i_GUI.packs, respack_destroy, ResPack);
        arrst_destroy(&i_GUI.func_packs, NULL, FPtr_respack);
        arrpt_destroy(&i_GUI.windows, NULL, Window);
        arrpt_destroy(&i_GUI.menus, NULL, Menu);
        listener_destroy(&i_GUI.OnTheme);
        listener_destroy(&i_GUI.OnTranslate);
        listener_destroy(&i_GUI.OnMenubar);
        listener_destroy(&i_GUI.OnFinish);
        draw2d_finish();
    }
}

/*---------------------------------------------------------------------------*/

void gui_respack(FPtr_respack func_respack)
{
    cassert_no_nullf(func_respack);
    arrst_append(i_GUI.func_packs, func_respack, FPtr_respack);
}

/*---------------------------------------------------------------------------*/

void gui_language(const char_t *lang)
{
    str_upd(&i_GUI.current_lang, lang);

    arrpt_clear(i_GUI.packs, respack_destroy, ResPack);
    arrst_foreach(func_pack, i_GUI.func_packs, FPtr_respack)
        ResPack *pack = (*func_pack)(lang);
        arrpt_append(i_GUI.packs, pack, ResPack);
    arrpt_end();

    arrpt_foreach(menu, i_GUI.menus, Menu)
        _menu_locale(menu);
    arrpt_end();

    arrpt_foreach(window, i_GUI.windows, Window)
        _window_locale(window);
    arrpt_end();

    if (i_GUI.OnTranslate != NULL)
        listener_event(i_GUI.OnTranslate, 0, NULL, lang, NULL, void, char_t, void);
}

/*---------------------------------------------------------------------------*/

const char_t *gui_text(const ResId id)
{
    bool_t is_resid;
    const char_t *text = respack_atext(i_GUI.packs, id, &is_resid);
    cassert_unref(is_resid == TRUE, is_resid);
    return text;
}

/*---------------------------------------------------------------------------*/

const Image *gui_image(const ResId id)
{
    bool_t is_resid;
    Image *image = respack_aobj(i_GUI.packs, id, image_from_data, image_destroy, &is_resid, Image);
    cassert_unref(is_resid == TRUE, is_resid);
    return image;
}

/*---------------------------------------------------------------------------*/

const byte_t *gui_file(const ResId id, uint32_t *size)
{
    bool_t is_resid;
    const byte_t *data = respack_afile(i_GUI.packs, id, size, &is_resid);
    cassert_unref(is_resid == TRUE, is_resid);
    return data;
}

/*---------------------------------------------------------------------------*/

bool_t gui_dark_mode(void)
{
    return i_kDARK_MODE;
}

/*---------------------------------------------------------------------------*/

color_t gui_alt_color(const color_t light_color, const color_t dark_color)
{
    uint32_t n = arrst_size(i_GUI.altcolors, ColorAlt);
    ColorAlt *alt = arrst_new(i_GUI.altcolors, ColorAlt);
    alt->light = light_color;
    alt->dark = dark_color;
    draw2d_set_named_color(kFIRST_COLOR_ALT + n, i_kDARK_MODE ? alt->dark : alt->light);
    return (color_t)(kFIRST_COLOR_ALT + n);
}

/*---------------------------------------------------------------------------*/

color_t gui_label_color(void)
{
    return (color_t)ekSYS_LABEL;
}

/*---------------------------------------------------------------------------*/

color_t gui_view_color(void)
{
    return (color_t)ekSYS_VIEW;
}

/*---------------------------------------------------------------------------*/

color_t gui_line_color(void)
{
    return (color_t)ekSYS_LINE;
}

/*---------------------------------------------------------------------------*/

color_t gui_link_color(void)
{
    return (color_t)ekSYS_LINK;
}

/*---------------------------------------------------------------------------*/

color_t gui_border_color(void)
{
    return (color_t)ekSYS_BORDER;
}

/*---------------------------------------------------------------------------*/

//device_t gui_device(void);
//device_t gui_device(void)
//{
//    const GuiContext *context = gui_context_get_current();
//    cassert_no_null(context);
//    cassert_no_nullf(context->func_globals_device);
//    return (device_t)context->func_globals_device(NULL);
//}

/*---------------------------------------------------------------------------*/

S2Df gui_resolution(void)
{
    S2Df res = {0,0};
    const GuiContext *context = gui_context_get_current();
    cassert_no_null(context);
    cassert_no_nullf(context->func_globals_resolution);
    context->func_globals_resolution(NULL, &res.width, &res.height);
    return res;
}

/*---------------------------------------------------------------------------*/

V2Df gui_mouse_pos(void)
{
    V2Df pos = {0,0};
    const GuiContext *context = gui_context_get_current();
    cassert_no_null(context);
    cassert_no_nullf(context->func_globals_mouse_position);
    context->func_globals_mouse_position(NULL, &pos.x, &pos.y);
    return pos;
}

/*---------------------------------------------------------------------------*/

void gui_OnThemeChanged(Listener *listener)
{
    listener_update(&i_GUI.OnTheme, listener);
}

/*---------------------------------------------------------------------------*/

void *evbind_object_imp(Event *e, const char_t *type)
{
	const EvBind *p = event_params(e, EvBind);
	cassert_unref(str_equ_c(p->objtype_notif, type) == TRUE, type);
	return p->obj_notify;
}

/*---------------------------------------------------------------------------*/

bool_t evbind_modify_imp(Event *e, const char_t *type, const uint16_t size, const char_t *mname, const char_t *mtype, const uint16_t moffset, const uint16_t msize)
{
	const EvBind *p = event_params(e, EvBind);
	if (p->obj_notify != NULL)
		return gbind_modify_data(p->obj_notify, type, size, mname, mtype, moffset, msize, p);

	return FALSE;
}

/*---------------------------------------------------------------------------*/

Window *_gui_main_window(void)
{
    arrpt_foreach(window, i_GUI.windows, Window)
        if (_window_role(window) == ekGUI_ROLE_MAIN)
            return window;
    arrpt_end();
    return NULL;
}

/*---------------------------------------------------------------------------*/

Font *_gui_create_default_font(void)
{
    if (i_GUI.default_font == NULL)
        i_GUI.default_font = font_system(font_regular_size(), 0);
    return font_copy(i_GUI.default_font);
}

/*---------------------------------------------------------------------------*/

bool_t _gui_effective_alt_font(const Font *font, const Font *alt_font)
{
    cassert_no_null(font);
    if (alt_font == NULL)
        return FALSE;
    if (font_equals(font, alt_font) == TRUE)
        return FALSE;
    else
        return TRUE;
}

/*---------------------------------------------------------------------------*/

void _gui_update_font(Font **font, Font **alt_font, const Font *new_font)
{
    cassert_no_null(font);
    font_destroy(font);
    if (new_font != NULL)
        *font = font_copy(new_font);
    else
        *font = _gui_create_default_font();

    if (alt_font != NULL)
    {
        if (*alt_font != NULL)
        {
            uint32_t style = font_style(*alt_font);
            font_destroy(alt_font);
            *alt_font = font_with_style(*font, style);
        }
    }
}

/*---------------------------------------------------------------------------*/

void _gui_add_transition_imp(void *owner, Listener *listener)
{
    arrst_foreach(transition, i_GUI.transitions, Transition)
        if (transition->owner == owner)
        {
            cassert(FALSE);
            return;
        }
    arrst_end()

    {
        Transition *transition = arrst_new(i_GUI.transitions, Transition);
        transition->owner = owner;
        transition->listener = listener;
    }
}

/*---------------------------------------------------------------------------*/

void _gui_delete_transition_imp(void *owner)
{
    arrst_foreach(transition, i_GUI.transitions, Transition)
        if (transition->owner == owner)
        {
            arrst_delete(i_GUI.transitions, transition_i, i_remove_transition, Transition);
            return;
        }
    arrst_end()
}

/*---------------------------------------------------------------------------*/

void _gui_add_window(Window *window)
{
    cassert_no_null(window);
    arrpt_append(i_GUI.windows, window, Window);
}

/*---------------------------------------------------------------------------*/

void _gui_delete_window(Window *window)
{
    uint32_t i;
    cassert_no_null(window);
    i = arrpt_find(i_GUI.windows, window, Window);
    arrpt_delete(i_GUI.windows, i, NULL, Window);

    if (i_GUI.OnMenubar != NULL)
    {
        void *ptrs[2];
        ptrs[0] = NULL;
        ptrs[1] = window;
        listener_event(i_GUI.OnMenubar, 0, NULL, ptrs, NULL, void, void, void);
    }
}

/*---------------------------------------------------------------------------*/

void _gui_add_menu(Menu *menu)
{
    cassert_no_null(menu);
    arrpt_append(i_GUI.menus, menu, Menu);
}

/*---------------------------------------------------------------------------*/

void _gui_delete_menu(Menu *menu)
{
    uint32_t i;
    cassert_no_null(menu);
    i = arrpt_find(i_GUI.menus, menu, Menu);
    arrpt_delete(i_GUI.menus, i, NULL, Menu);

    if (i_GUI.OnMenubar != NULL)
    {
        void *ptrs[2];
        ptrs[0] = menu;
        ptrs[1] = NULL;
        listener_event(i_GUI.OnMenubar, 0, NULL, ptrs, NULL, void, void, void);
    }
}

/*---------------------------------------------------------------------------*/

const char_t *_gui_respack_text(const ResId id, ResId *store_id)
{
    bool_t is_resid;
    const char_t *restext = respack_atext(i_GUI.packs, id, &is_resid);
    if (restext != NULL)
    {
        cassert(is_resid == TRUE);
        ptr_assign(store_id, id);
        return restext;
    }
    else if (is_resid == TRUE)
    {
        ptr_assign(store_id, id);
        return "";
    }

    return (const char_t*)id;
}

/*---------------------------------------------------------------------------*/

const Image *_gui_respack_image(const ResId id, ResId *store_id)
{
    if (id != NULL)
    {
        bool_t is_resid;
        Image *resimage = respack_aobj(i_GUI.packs, id, image_from_data, image_destroy, &is_resid, Image);
        if (resimage != NULL)
        {
            cassert(is_resid == TRUE);
            ptr_assign(store_id, id);
            return resimage;
        }
        else if (is_resid == TRUE)
        {
            ptr_assign(store_id, id);
            return NULL;
        }

        return (const Image*)id;
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

const Cursor *_gui_cursor(const cursor_t cursor, const Image *image, const real32_t hot_x, const real32_t hot_y)
{
    arrst_foreach(curs, i_GUI.cursors, CurIcon)
        if (cursor == ekCUSER && curs->image == image)
            return curs->cursor;
        if (cursor != ekCUSER && cursor == curs->type)
            return curs->cursor;
    arrst_end();

    {
        CurIcon *newcur = arrst_new(i_GUI.cursors, CurIcon);
        const GuiContext *context = gui_context_get_current();
        i_GUI.func_destroy_cursor = context->func_globals_cursor_destroy;
        newcur->type = cursor;
        newcur->image = image;
        newcur->cursor = context->func_globals_cursor((enum_t)cursor, image, hot_x, hot_y);
        return newcur->cursor;
    }
}

/*---------------------------------------------------------------------------*/

static void i_precompute_system_colors(void)
{
    syscolor_t scol = ekSYS_DARKMODE;
    const GuiContext *context = gui_context_get_current();

    cassert_no_null(context);
    cassert_no_nullf(context->func_globals_color);

    i_kDARK_MODE = (bool_t)context->func_globals_color(&scol);

    scol = ekSYS_LABEL;
    draw2d_set_named_color(scol, context->func_globals_color(&scol));

    scol = ekSYS_VIEW;
    draw2d_set_named_color(scol, context->func_globals_color(&scol));

    scol = ekSYS_LINE;
    draw2d_set_named_color(scol, context->func_globals_color(&scol));

    scol = ekSYS_LINK;
    draw2d_set_named_color(scol, context->func_globals_color(&scol));

    scol = ekSYS_BORDER;
    draw2d_set_named_color(scol, context->func_globals_color(&scol));

    arrst_foreach(alt, i_GUI.altcolors, ColorAlt)
        draw2d_set_named_color(kFIRST_COLOR_ALT + alt_i, i_kDARK_MODE ? alt->dark : alt->light);
    arrst_end();
}

/*---------------------------------------------------------------------------*/

void _gui_OnThemeChanged(void)
{    
    arrpt_foreach(window, i_GUI.windows, Window)
        _window_update(window);
    arrpt_end();

    if (i_GUI.OnTheme != NULL)
        listener_event(i_GUI.OnTheme, 0, NULL, NULL, NULL, void, void, void);

    i_precompute_system_colors();
}

/*---------------------------------------------------------------------------*/

void _gui_OnTranslate(Listener *listener)
{
    listener_update(&i_GUI.OnTranslate, listener);
}

/*---------------------------------------------------------------------------*/

void _gui_OnMenubar(Listener *listener)
{
    listener_update(&i_GUI.OnMenubar, listener);
}

/*---------------------------------------------------------------------------*/

void _gui_OnFinish(Listener *listener)
{
    listener_update(&i_GUI.OnFinish, listener);
}

/*---------------------------------------------------------------------------*/

static Layout *i_info_layout(ResPack *pack, const char_t *caption, const char_t *detail, const char_t *file, const uint32_t line)
{
    Layout *layout = layout_create(1, 4);
    Label *label1 = label_create();
    Label *label2 = label_multiline();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Font *font = font_system(font_regular_size() + 2.f, ekFBOLD);
    String *sline = str_printf("%s: %d", respack_text(pack, LINE_TEXT), line);
    String *filename = NULL;
    String *sfile = NULL;
    str_split_pathname(file, NULL, &filename);
    sfile = str_printf("%s: %s", respack_text(pack, FILE_TEXT), tc(filename));
    label_font(label1, font);
    label_text(label1, caption);
    label_text(label2, detail);
    label_text(label3, tc(sfile));
    label_text(label4, tc(sline));
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 0, 1);
    layout_label(layout, label3, 0, 2);
    layout_label(layout, label4, 0, 3);
    layout_bgcolor(layout, gui_alt_color(color_rgb(255,255,254), color_rgb(10,10,10)));
    layout_skcolor(layout, gui_alt_color(color_bgr(0x4681Cf), color_bgr(0x1569E6)));
    layout_margin(layout, 5.f);
    font_destroy(&font);
    str_destroy(&filename);
    str_destroy(&sfile);
    str_destroy(&sline);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_icons_layout(const ResPack *pack, const icon_t icon)
{
    Layout *layout = layout_create(1, 2);
    ImageView *img1 = imageview_create();
    ImageView *img2 = imageview_create();
    const Image *icon_image = NULL;
    const Image *logo_image = NULL;
    bool_t dark = gui_dark_mode();

    switch (icon) {
    case ekICON_ASSERT:
        icon_image = image_from_resource(pack, dark ? CONFUSED_DARK_PNG : CONFUSED_PNG);
        break;
    case ekICON_CRASH:
        icon_image = image_from_resource(pack, dark ? BOMB_DARK_PNG : BOMB_PNG);
        break;
    case ekICON_SYSTEM:
        cassert_default();
    }

    logo_image = image_from_resource(pack, dark ? LOGO_DARK_PNG : LOGO_PNG);

    imageview_image(img1, icon_image);
    imageview_image(img2, logo_image);
    layout_imageview(layout, img1, 0, 0);
    layout_imageview(layout, img2, 0, 1);
    layout_vmargin(layout, 0, 10.f);
    layout_margin4(layout, 0.f, 15.f, 0.f, 15.f);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnShowAssert(void *empty, Event *e)
{
    const EvButton *params = event_params(e, EvButton);
    unref(empty);
    i_GUI.show_assert_window = (bool_t)(params->state == ekON);
}

/*---------------------------------------------------------------------------*/

static void i_OnWriteLog(void *empty, Event *e)
{
    const EvButton *params = event_params(e, EvButton);
    unref(empty);
    i_GUI.write_assert_log = (bool_t)(params->state == ekON);
}

/*---------------------------------------------------------------------------*/

static Layout *i_checks_layout(const ResPack *pack)
{
    const char_t *log_file = log_get_file();
    Layout *layout = layout_create(1, (uint32_t)(log_file != NULL ? 3 : 2));
    Button *button1 = button_check();
    Button *button2 = button_check();
    button_text(button1, respack_text(pack, SHOW_ASSERT));
    button_text(button2, respack_text(pack, ASSERT_LOG));
    button_state(button1, i_GUI.show_assert_window ? ekON : ekOFF);
    button_state(button2, i_GUI.write_assert_log ? ekON : ekOFF);
    button_OnClick(button1, listener(NULL, i_OnShowAssert, void));
    button_OnClick(button2, listener(NULL, i_OnWriteLog, void));
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 0, 1);

    if (log_file != NULL)
    {
        Label *label = label_create();
        Font *font = font_system(font_small_size(), 0);
        label_text(label, log_file);
        label_font(label, font);
        font_destroy(&font);
        layout_label(layout, label, 0, 2);
    }

    layout_vmargin(layout, 0, 5.f);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_assert_layout(const ResPack *pack, const char_t *message, const icon_t icon, Layout *info_layout)
{
    Layout *layout = layout_create(2, 1);
    Layout *layout1 = layout_create(1, (uint32_t)(icon == ekICON_CRASH ? 2 : 3));
    Layout *layout2 = i_icons_layout(pack, icon);
    Label *label = label_multiline();
    String *smessage = str_printf("%s %s %s", message, respack_text(pack, CONTACT_INFO), "support@nappgui.com");
    label_text(label, tc(smessage));
    layout_layout(layout, layout2, 0, 0);
    layout_valign(layout, 0, 0, ekTOP);
    layout_hmargin(layout, 0, 5.f);
    layout_label(layout1, label, 0, 0);
    layout_layout(layout1, info_layout, 0, 1);

    if (icon != ekICON_CRASH)
    {
        Layout *layout3 = i_checks_layout(pack);
        layout_layout(layout1, layout3, 0, 2);
        layout_vmargin(layout1, 1, 5.f);
    }

    layout_hsize(layout1, 0, 300.f);
    layout_hexpand(layout, 1);
    layout_vmargin(layout1, 0, 5.f);
    layout_layout(layout, layout1, 1, 0);
    str_destroy(&smessage);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnContinue(void *empty, Event *event)
{
    unref(empty);
    unref(event);
    window_stop_modal(i_GUI.assert_window, 0);
}

/*---------------------------------------------------------------------------*/

static void i_OnDebug(void *empty, Event *event)
{
    unref(empty);
    unref(event);
    window_stop_modal(i_GUI.assert_window, 1);
}

/*---------------------------------------------------------------------------*/

static void i_OnExit(void *empty, Event *event)
{
    unref(empty);
    unref(event);
    window_stop_modal(i_GUI.assert_window, 2);
}

/*---------------------------------------------------------------------------*/

static Layout *i_buttons_layout(const ResPack *pack, const icon_t icon, Button **defbutton)
{
    Layout *layout = layout_create(3, 1);
    Button *button1 = button_push();
    Button *button3 = button_push();
    button_text(button1, respack_text(pack, DEBUG_TEXT));
    button_text(button3, respack_text(pack, EXIT_TEXT));
    button_OnClick(button1, listener(NULL, i_OnDebug, void));
    button_OnClick(button3, listener(NULL, i_OnExit, void));
    layout_button(layout, button1, (uint32_t)(icon != ekICON_CRASH ? 0 : 1), 0);
    layout_button(layout, button3, 2, 0);

    if (icon != ekICON_CRASH)
    {
        Button *button2 = button_push();
        button_text(button2, respack_text(pack, CONTINUE_TEXT));
        button_OnClick(button2, listener(NULL, i_OnContinue, void));
        layout_button(layout, button2, 1, 0);
        *defbutton = button2;
    }
    else
    {
        *defbutton = button1;
    }

    layout_hmargin(layout, 0, 30.f);
    layout_hmargin(layout, 1, 10.f);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Window *i_assert_window(ResPack *pack, const char_t *message, const icon_t icon, Layout *info_layout)
{
    Button *defbutton = NULL;
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 2);
    Layout *layout1 = i_assert_layout(pack, message, icon, info_layout);
    Layout *layout2 = i_buttons_layout(pack, icon, &defbutton);
    String *title = str_printf(respack_text(pack, ASSERT_TITLE), tc(i_GUI.app_name));
    Window *window = window_create(ekWNTITLE);
    layout_layout(layout, layout1, 0, 0);
    layout_layout(layout, layout2, 0, 1);
    layout_vmargin(layout, 0, 20.f);
    layout_margin(layout, 15.f);
    panel_layout(panel, layout);
    window_panel(window, panel);
    window_title(window, tc(title));
    window_defbutton(window, defbutton);
    str_destroy(&title);
    return window;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_launch_assert(void)
{
    Window *main_window = _gui_main_window();
    S2Df size1;
    S2Df size2;
    V2Df origin;
    uint32_t ret;

    if (main_window != NULL)
    {
        origin = window_get_origin(main_window);
        size1 = window_get_size(main_window);
    }
    else
    {
        origin = kV2D_ZEROf;
        size1 = gui_resolution();
    }

    size2 = window_get_size(i_GUI.assert_window);
    origin.x += .5f * (size1.width - size2.width);
    origin.y += 50.f;
    window_origin(i_GUI.assert_window, origin);
    ret = window_modal(i_GUI.assert_window, main_window);
    window_destroy(&i_GUI.assert_window);
    return ret;
}

/*---------------------------------------------------------------------------*/

static void i_assert_handler(void *item, const uint32_t group, const char_t *caption, const char_t *detail, const char_t *file, const uint32_t line)
{
    bool_t finish = FALSE;
    unref(item);

    if (i_GUI.in_assert == TRUE)
        return;

    i_GUI.in_assert = TRUE;
    /*if (app->OnAssert_listener != NULL)
    {
        EvAssert params;
        params.group = group;
        params.caption = caption;
        params.detail = detail;
        params.file = file;
        params.line = line;
        listener_event(app->OnAssert_listener, ekEVENT_ASSERT, NULL, &params, &app->show_assert_window, void, EvAssert, bool_t);
    }*/


    if (i_GUI.show_assert_window == TRUE)
    {
        ResPack *pack = res_assert_respack("");
        const char_t *message = respack_text(pack, group == 0 ? ASSERT_CRASH : ASSERT_INFO);
        icon_t icon = group == 0 ? ekICON_CRASH : ekICON_ASSERT;
        Layout *info_layout = i_info_layout(pack, caption, detail, file, line);
        uint32_t assert_ret;
        i_GUI.assert_window = i_assert_window(pack, message, icon, info_layout);

        assert_ret = i_launch_assert();
        switch (assert_ret) {
        case 0:
            break;
        case 1:
            bstd_debug_break();
            break;
        case 2:
            bstd_abort();
            break;
            //if (i_GUI.OnFinish != NULL)
            //    listener_event(i_GUI.OnFinish, 0, NULL, NULL, NULL, void, void, void);
            //finish = TRUE;
            //break;
        cassert_default();
        }

        respack_destroy(&pack);
    }

    if (i_GUI.write_assert_log == TRUE)
    {
        if (group == 0)
        {
            log_printf("FATAL ASSERT: %s (%s:%d): %s", caption, file, line, detail);
            bproc_exit(1000);
        }
        else
        {
            log_printf("ASSERT: %s (%s:%d): %s", caption, file, line, detail);
        }
    }

    i_GUI.in_assert = finish;
}

