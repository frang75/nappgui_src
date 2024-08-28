/* Text editor demo */

#include "editor.h"
#include "res_guihello.h"
#include <gui/guiall.h>

/*---------------------------------------------------------------------------*/

typedef struct _edit_data_t EditData;

struct _edit_data_t
{
    ArrPt(String) *fonts;
    Window *window;
    TextView *text;
    PopUp *family_popup;
    PopUp *size_popup;
    PopUp *color_popup;
    PopUp *back_popup;
    Button *bold_check;
    Button *italic_check;
    Button *under_check;
    Button *strike_check;
};

/*---------------------------------------------------------------------------*/

static color_t i_COLORS[9];

/*---------------------------------------------------------------------------*/

static void i_destroy_data(EditData **data)
{
    cassert_no_null(data);
    cassert_no_null(*data);
    arrpt_destroy(&(*data)->fonts, str_destroy, String);
    heap_delete(data, EditData);
}

/*---------------------------------------------------------------------------*/

static void i_apply_params(EditData *data, const bool_t apply_all)
{
    const char_t *ffamily = NULL;
    const char_t *tsize = NULL;
    real32_t fsize = 0;
    uint32_t fstyle = 0;
    color_t color = 0;
    color_t back = 0;
    cassert_no_null(data);

    /* Get text attribs from GUI controls */
    ffamily = popup_get_text(data->family_popup, popup_get_selected(data->family_popup));
    tsize = popup_get_text(data->size_popup, popup_get_selected(data->size_popup));
    fsize = str_to_r32(tsize, NULL);

    if (button_get_state(data->bold_check) == ekGUI_ON)
        fstyle |= ekFBOLD;
    if (button_get_state(data->italic_check) == ekGUI_ON)
        fstyle |= ekFITALIC;
    if (button_get_state(data->under_check) == ekGUI_ON)
        fstyle |= ekFUNDERLINE;
    if (button_get_state(data->strike_check) == ekGUI_ON)
        fstyle |= ekFSTRIKEOUT;

    color = i_COLORS[popup_get_selected(data->color_popup)];
    back = i_COLORS[popup_get_selected(data->back_popup)];

    /* Set textview params */
    textview_family(data->text, ffamily);
    textview_fsize(data->text, fsize);
    textview_fstyle(data->text, fstyle);
    textview_color(data->text, color);
    textview_bgcolor(data->text, back);

    /* Apply the format */
    if (apply_all == TRUE)
        textview_apply_all(data->text);
    else
        textview_apply_sel(data->text);
}

/*---------------------------------------------------------------------------*/

static PopUp *i_font_popup(EditData *data)
{
    PopUp *popup = popup_create();
    Font *sfont = font_system(15, 0);
    const char_t *fname = font_family(sfont);
    uint32_t sel = UINT32_MAX;
    uint32_t arial = UINT32_MAX;
    cassert_no_null(data);
    data->fonts = font_installed_families();

    arrpt_foreach_const(font, data->fonts, String)
        popup_add_elem(popup, tc(font), NULL);
        if (str_equ(font, fname) == TRUE)
            sel = font_i;
        if (str_equ_nocase(tc(font), "Arial") == TRUE)
            arial = font_i;
    arrpt_end()

    if (sel != UINT32_MAX)
        popup_selected(popup, sel);
    else if (arial != UINT32_MAX)
        popup_selected(popup, arial);
    else
        popup_selected(popup, 0);

    popup_list_height(popup, 20);
    font_destroy(&sfont);
    return popup;
}

/*---------------------------------------------------------------------------*/

static PopUp *i_font_size(void)
{
    PopUp *popup = popup_create();
    uint32_t i = 0;
    for (i = 10; i <= 30; ++i)
    {
        char_t buf[32];
        bstd_sprintf(buf, sizeof(buf), "%d", i);
        popup_add_elem(popup, buf, NULL);
    }
    return popup;
}

/*---------------------------------------------------------------------------*/

static PopUp *i_font_color(void)
{
    PopUp *popup = popup_create();
    popup_add_elem(popup, "Default", NULL);
    popup_add_elem(popup, "Black", NULL);
    popup_add_elem(popup, "White", NULL);
    popup_add_elem(popup, "Red", NULL);
    popup_add_elem(popup, "Green", NULL);
    popup_add_elem(popup, "Blue", NULL);
    popup_add_elem(popup, "Yellow", NULL);
    popup_add_elem(popup, "Cyan", NULL);
    popup_add_elem(popup, "Magenta", NULL);
    return popup;
}

/*---------------------------------------------------------------------------*/

static Layout *i_font_style(EditData *data)
{
    Layout *layout = layout_create(1, 4);
    Button *button1 = button_check();
    Button *button2 = button_check();
    Button *button3 = button_check();
    Button *button4 = button_check();
    cassert_no_null(data);
    button_text(button1, "Bold");
    button_text(button2, "Italic");
    button_text(button3, "Underline");
    button_text(button4, "Strikeout");
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 0, 1);
    layout_button(layout, button3, 0, 2);
    layout_button(layout, button4, 0, 3);
    data->bold_check = button1;
    data->italic_check = button2;
    data->under_check = button3;
    data->strike_check = button4;
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_text_controls(EditData *data)
{
    Layout *layout1 = layout_create(5, 2);
    Layout *layout2 = i_font_style(data);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    PopUp *popup1 = i_font_popup(data);
    PopUp *popup2 = i_font_size();
    PopUp *popup3 = i_font_color();
    PopUp *popup4 = i_font_color();
    label_text(label1, "Font family");
    label_text(label2, "Font size");
    label_text(label3, "Font style");
    label_text(label4, "Font color");
    label_text(label5, "Back color");
    popup_selected(popup2, 5);
    layout_label(layout1, label1, 0, 0);
    layout_label(layout1, label2, 1, 0);
    layout_label(layout1, label3, 2, 0);
    layout_label(layout1, label4, 3, 0);
    layout_label(layout1, label5, 4, 0);
    layout_popup(layout1, popup1, 0, 1);
    layout_popup(layout1, popup2, 1, 1);
    layout_layout(layout1, layout2, 2, 1);
    layout_popup(layout1, popup3, 3, 1);
    layout_popup(layout1, popup4, 4, 1);
    layout_hmargin(layout1, 0, 5);
    layout_hmargin(layout1, 1, 5);
    layout_hmargin(layout1, 2, 5);
    layout_hmargin(layout1, 3, 5);
    layout_vmargin(layout1, 0, 5);
    layout_valign(layout1, 0, 1, ekTOP);
    layout_valign(layout1, 1, 1, ekTOP);
    layout_valign(layout1, 2, 1, ekTOP);
    layout_valign(layout1, 3, 1, ekTOP);
    layout_valign(layout1, 4, 1, ekTOP);
    data->family_popup = popup1;
    data->size_popup = popup2;
    data->color_popup = popup3;
    data->back_popup = popup4;
    return layout1;
}

/*---------------------------------------------------------------------------*/

static void i_OnApplyAll(EditData *data, Event *e)
{
    i_apply_params(data, TRUE);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnApplySel(EditData *data, Event *e)
{
    i_apply_params(data, FALSE);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static Layout *i_apply_buttons(EditData *data)
{
    Layout *layout = layout_create(3, 1);
    Label *label = label_create();
    Button *button1 = button_push();
    Button *button2 = button_push();
    label_text(label, "Apply format");
    button_text(button1, "All text");
    button_text(button2, "Selected text");
    button_OnClick(button1, listener(data, i_OnApplyAll, EditData));
    button_OnClick(button2, listener(data, i_OnApplySel, EditData));
    layout_label(layout, label, 0, 0);
    layout_button(layout, button1, 1, 0);
    layout_button(layout, button2, 2, 0);
    layout_hmargin(layout, 0, 20);
    layout_hmargin(layout, 1, 5);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(EditData *data)
{
    Layout *layout1 = layout_create(1, 3);
    Layout *layout2 = i_text_controls(data);
    Layout *layout3 = i_apply_buttons(data);
    TextView *text = textview_create();
    cassert_no_null(data);
    textview_editable(text, TRUE);
    textview_show_select(text, TRUE);
    layout_layout(layout1, layout2, 0, 0);
    layout_textview(layout1, text, 0, 1);
    layout_layout(layout1, layout3, 0, 2);
    layout_halign(layout1, 0, 2, ekRIGHT);
    layout_vmargin(layout1, 0, 10);
    layout_vmargin(layout1, 1, 10);
    layout_tabstop(layout1, 0, 1, TRUE);
    data->text = text;
    return layout1;
}

/*---------------------------------------------------------------------------*/

static EditData *i_edit_data(Window *window)
{
    EditData *data = heap_new0(EditData);
    data->window = window;
    return data;
}

/*---------------------------------------------------------------------------*/

static void i_colors(void)
{
    i_COLORS[0] = kCOLOR_DEFAULT;
    i_COLORS[1] = kCOLOR_BLACK;
    i_COLORS[2] = kCOLOR_WHITE;
    i_COLORS[3] = kCOLOR_RED;
    i_COLORS[4] = kCOLOR_GREEN;
    i_COLORS[5] = kCOLOR_BLUE;
    i_COLORS[6] = kCOLOR_YELLOW;
    i_COLORS[7] = kCOLOR_CYAN;
    i_COLORS[8] = kCOLOR_MAGENTA;
}

/*---------------------------------------------------------------------------*/

Panel *editor(Window *window)
{
    EditData *data = i_edit_data(window);
    Layout *layout = i_layout(data);
    Panel *panel = panel_create();
    i_colors();
    i_apply_params(data, TRUE);
    panel_data(panel, &data, i_destroy_data, EditData);
    panel_layout(panel, layout);
    return panel;
}
