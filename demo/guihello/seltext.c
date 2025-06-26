/* Text selection and clipboard demo */

#include "seltext.h"
#include "res_guihello.h"
#include <gui/guiall.h>

/*---------------------------------------------------------------------------*/

typedef struct _sel_data_t SelData;

struct _sel_data_t
{
    Window *window;
    Edit *edit_range;
    Button *caps;
    TextView *text;
    TextView *info_text;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_data(SelData **data)
{
    heap_delete(data, SelData);
}

/*---------------------------------------------------------------------------*/

static void i_OnCopy(SelData *data, Event *e)
{
    GuiControl *control = NULL;
    cassert_no_null(data);
    unref(e);
    control = window_get_focus(data->window);
    if (guicontrol_edit(control) != NULL)
        edit_copy(guicontrol_edit(control));
    else if (guicontrol_textview(control) != NULL)
        textview_copy(guicontrol_textview(control));
}

/*---------------------------------------------------------------------------*/

static void i_OnPaste(SelData *data, Event *e)
{
    GuiControl *control = NULL;
    cassert_no_null(data);
    unref(e);
    control = window_get_focus(data->window);
    if (guicontrol_edit(control) != NULL)
        edit_paste(guicontrol_edit(control));
    else if (guicontrol_textview(control) != NULL)
        textview_paste(guicontrol_textview(control));
}

/*---------------------------------------------------------------------------*/

static void i_OnCut(SelData *data, Event *e)
{
    GuiControl *control = NULL;
    cassert_no_null(data);
    unref(e);
    control = window_get_focus(data->window);
    if (guicontrol_edit(control) != NULL)
        edit_cut(guicontrol_edit(control));
    else if (guicontrol_textview(control) != NULL)
        textview_cut(guicontrol_textview(control));
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect(SelData *data, Event *e)
{
    GuiControl *control = NULL;
    const char_t *range = NULL;
    String *left = NULL, *right = NULL;
    int32_t start = 0, end = 0;
    cassert_no_null(data);
    unref(e);
    control = window_get_focus(data->window);
    range = edit_get_text(data->edit_range);
    str_split_trim(range, ",", &left, &right);
    start = str_to_i32(tc(left), 10, NULL);
    end = str_to_i32(tc(right), 10, NULL);

    if (guicontrol_edit(control) != NULL)
        edit_select(guicontrol_edit(control), start, end);
    else if (guicontrol_textview(control) != NULL)
        textview_select(guicontrol_textview(control), start, end);

    str_destroy(&left);
    str_destroy(&right);
}

/*---------------------------------------------------------------------------*/

static void i_OnWrap(SelData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    cassert_no_null(data);
    textview_wrap(data->text, p->state == ekGUI_ON ? TRUE : FALSE);
}

/*---------------------------------------------------------------------------*/

static void i_filter_event(SelData *data, Event *e, const bool_t from_editbox)
{
    const EvText *p = event_params(e, EvText);
    cassert_no_null(data);

    /*
     * Convert the inserted text in caps.
     * p->text the control current text (const).
     * r->text the new filterd text.
     * p->cpos current caret position.
     * r->apply = TRUE means the editbox text has to be updated.
     * p->len number of chars inserted at left of caret (to caps).
     * We are working with UTF8-Strings. Sequential access.
     */
    if (button_get_state(data->caps) == ekGUI_ON && p->len > 0)
    {
        EvTextFilter *r = event_result(e, EvTextFilter);
        const char_t *src = p->text;
        char_t *dest = r->text;
        uint32_t cp = unicode_to_u32(src, ekUTF8);
        uint32_t pos = 0;
        while (cp != 0)
        {
            uint32_t offset = 0;
            if ((pos >= p->cpos - p->len && pos < p->cpos) || !from_editbox)
            {
                if (cp >= 'a' && cp <= 'z')
                    cp -= 32;
            }
            offset = unicode_to_char(cp, dest, ekUTF8);
            dest += offset;
            src = unicode_next(src, ekUTF8);
            cp = unicode_to_u32(src, ekUTF8);
            pos += 1;
        }

        *dest = 0;
        r->cpos = p->cpos;
        r->apply = TRUE;
    }

    textview_printf(data->info_text, "%s OnFilter: Pos %d Len %d\n", from_editbox ? "Edit" : "TextView", p->cpos, p->len);
    textview_scroll_caret(data->info_text);
}

/*---------------------------------------------------------------------------*/

static void i_OnEditFilter(SelData *data, Event *e)
{
    i_filter_event(data, e, TRUE);
}

/*---------------------------------------------------------------------------*/

static void i_OnEditChange(SelData *data, Event *e)
{
    const EvText *p = event_params(e, EvText);
    cassert_no_null(data);
    textview_printf(data->info_text, "Edit OnChange: Pos: %d Len %d\n", p->cpos, p->len);
    textview_scroll_caret(data->info_text);
}

/*---------------------------------------------------------------------------*/

static void i_OnTextViewFilter(SelData *data, Event *e)
{
    i_filter_event(data, e, FALSE);
}

/*---------------------------------------------------------------------------*/

static Layout *i_text_controls(SelData *data)
{
    Layout *layout = layout_create(7, 1);
    Button *button1 = button_flat();
    Button *button2 = button_flat();
    Button *button3 = button_flat();
    Button *button4 = button_push();
    Button *button5 = button_check();
    Button *button6 = button_check();
    Edit *edit = edit_create();
    button_image(button1, gui_image(COPY_PNG));
    button_image(button2, gui_image(PASTE_PNG));
    button_image(button3, gui_image(CUT_PNG));
    edit_text(edit, "2,6");
    button_OnClick(button1, listener(data, i_OnCopy, SelData));
    button_OnClick(button2, listener(data, i_OnPaste, SelData));
    button_OnClick(button3, listener(data, i_OnCut, SelData));
    button_tooltip(button1, "Copy");
    button_tooltip(button2, "Paste");
    button_tooltip(button3, "Cut");
    button_text(button4, "Select Text");
    button_text(button5, "Caps");
    button_text(button6, "Wrap");
    button_OnClick(button4, listener(data, i_OnSelect, SelData));
    button_OnClick(button6, listener(data, i_OnWrap, SelData));
    button_state(button6, ekGUI_ON);
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 1, 0);
    layout_button(layout, button3, 2, 0);
    layout_edit(layout, edit, 3, 0);
    layout_button(layout, button4, 4, 0);
    layout_button(layout, button5, 5, 0);
    layout_button(layout, button6, 6, 0);
    layout_hsize(layout, 3, 40);
    layout_hmargin(layout, 0, 5);
    layout_hmargin(layout, 1, 5);
    layout_hmargin(layout, 2, 5);
    layout_hmargin(layout, 3, 5);
    layout_hmargin(layout, 4, 5);
    layout_hmargin(layout, 5, 5);
    data->edit_range = edit;
    data->caps = button5;
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnAddText(SelData *data, Event *e)
{
    cassert_no_null(data);
    textview_printf(data->text, "%s", "add");
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnInsertText(SelData *data, Event *e)
{
    cassert_no_null(data);
    textview_cpos_printf(data->text, "%s", "ins");
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnDeleteText(SelData *data, Event *e)
{
    cassert_no_null(data);
    textview_del_select(data->text);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static Layout *i_textview_controls(SelData *data)
{
    Layout *layout = layout_create(5, 1);
    Label *label = label_create();
    Button *button1 = button_flat();
    Button *button2 = button_flat();
    Button *button3 = button_flat();
    label_text(label, "TextView");
    button_image(button1, gui_image(EDIT16_PNG));
    button_image(button2, gui_image(CURSOR16_PNG));
    button_image(button3, gui_image(ERROR16_PNG));
    button_OnClick(button1, listener(data, i_OnAddText, SelData));
    button_OnClick(button2, listener(data, i_OnInsertText, SelData));
    button_OnClick(button3, listener(data, i_OnDeleteText, SelData));
    button_tooltip(button1, "Add text at the end of TextView");
    button_tooltip(button2, "Insert text at cursor position in TextView");
    button_tooltip(button3, "Delete the selected text, without copy into clipboard");
    layout_label(layout, label, 0, 0);
    layout_button(layout, button1, 2, 0);
    layout_button(layout, button2, 3, 0);
    layout_button(layout, button3, 4, 0);
    layout_hexpand(layout, 1);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(SelData *data)
{
    Layout *layout1 = layout_create(1, 11);
    Layout *layout2 = i_textview_controls(data);
    Layout *layout3 = i_text_controls(data);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label5 = label_create();
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_multiline();
    Edit *edit3 = edit_multiline();
    TextView *text1 = textview_create();
    TextView *text2 = textview_create();
    cassert_no_null(data);
    label_text(label1, "EditBox");
    label_text(label2, "EditBox (multiline)");
    label_text(label3, "EditBox (multiline with user-height)");
    label_text(label5, "Info");
    edit_text(edit1, "This is a text in the EditBox control");
    edit_text(edit2, "This is a text in the multiline EditBox control");
    edit_text(edit3, "This is a text in the multiline EditBox control");
    edit_min_height(edit3, 100);
    edit_OnFilter(edit1, listener(data, i_OnEditFilter, SelData));
    edit_OnFilter(edit2, listener(data, i_OnEditFilter, SelData));
    edit_OnFilter(edit3, listener(data, i_OnEditFilter, SelData));
    edit_OnChange(edit1, listener(data, i_OnEditChange, SelData));
    edit_OnChange(edit2, listener(data, i_OnEditChange, SelData));
    edit_OnChange(edit3, listener(data, i_OnEditChange, SelData));
    textview_writef(text1, "This is another text in the TextView control, wider than the control.");
    textview_editable(text1, TRUE);
    textview_OnFilter(text1, listener(data, i_OnTextViewFilter, SelData));
    layout_label(layout1, label1, 0, 0);
    layout_edit(layout1, edit1, 0, 1);
    layout_label(layout1, label2, 0, 2);
    layout_edit(layout1, edit2, 0, 3);
    layout_label(layout1, label3, 0, 4);
    layout_edit(layout1, edit3, 0, 5);
    layout_layout(layout1, layout2, 0, 6);
    layout_textview(layout1, text1, 0, 7);
    layout_layout(layout1, layout3, 0, 8);
    layout_label(layout1, label5, 0, 9);
    layout_textview(layout1, text2, 0, 10);
    layout_tabstop(layout1, 0, 6, FALSE);
    layout_tabstop(layout1, 0, 7, TRUE);
    layout_tabstop(layout1, 0, 8, FALSE);
    layout_halign(layout1, 0, 8, ekLEFT);
    layout_vmargin(layout1, 1, 5);
    layout_vmargin(layout1, 3, 5);
    layout_vmargin(layout1, 5, 5);
    layout_vmargin(layout1, 7, 5);
    data->text = text1;
    data->info_text = text2;
    return layout1;
}

/*---------------------------------------------------------------------------*/

static SelData *i_seldata(Window *window)
{
    SelData *data = heap_new0(SelData);
    data->window = window;
    return data;
}

/*---------------------------------------------------------------------------*/

Panel *seltext(Window *window)
{
    SelData *data = i_seldata(window);
    Layout *layout = i_layout(data);
    Panel *panel = panel_create();
    panel_data(panel, &data, i_destroy_data, SelData);
    panel_layout(panel, layout);
    return panel;
}
