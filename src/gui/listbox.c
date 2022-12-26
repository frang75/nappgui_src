/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: listbox.c
 *
 */

/* ListBox */

#include "listbox.h"
#include "listbox.inl"
#include "cell.inl"
#include "drawctrl.inl"
#include "view.h"
#include "view.inl"
#include "gui.inl"

#include "arrpt.h"
#include "arrst.h"
#include "bmath.h"
#include "cassert.h"
#include "color.h"
#include "draw.h"
#include "draw.inl"
#include "event.h"
#include "font.h"
#include "heap.h"
#include "image.h"
#include "ptr.h"
#include "s2d.h"
#include "strings.h"
#include "types.h"

typedef struct _pelem_t PElem;
typedef struct _ldata_t LData;

struct _pelem_t
{
    ResId resid;
    String *text;
    Image *image;
    uint32_t imgwidth;
    uint32_t imgheight;
    color_t color;
    bool_t check;
    bool_t select;
};

struct _ldata_t
{
    Font *font;
    ArrSt(PElem) *elems;
    uint32_t mouse_ypos;
    uint32_t control_width;
    uint32_t control_height;
    uint32_t content_width;
    uint32_t font_height;
    uint32_t cell_height;
    uint32_t row_height;
    uint32_t check_width;
    uint32_t check_height;
    uint32_t text_yoffset;
    uint32_t check_yoffset;
    Listener *OnSelect;
    uint32_t selected;
    ctrl_msel_t multisel_mode;
    bool_t mouse_incheck;
    bool_t check_pressed;
    bool_t checks;
    bool_t multisel;
    bool_t focused;
};

/*---------------------------------------------------------------------------*/

DeclSt(PElem);
static const uint32_t i_LEFT_PADDING = 4;
static const uint32_t i_RIGHT_PADDING = 4;
static const uint32_t i_BOTTOM_PADDING = 4;

/*---------------------------------------------------------------------------*/

static LData *i_create_data(void)
{
    LData *data = heap_new0(LData);
    data->font = drawctrl_font(NULL);
    data->elems = arrst_create(PElem);
    data->mouse_ypos = UINT32_MAX;
    data->selected = UINT32_MAX;
    data->font_height = (uint32_t)bmath_ceilf(font_size(data->font));
    data->cell_height = (uint32_t)bmath_ceilf(font_height(data->font));
    data->check_width = UINT32_MAX;
    data->check_height = UINT32_MAX;
    return data;
}

/*---------------------------------------------------------------------------*/

static void i_remove_elem(PElem *elem)
{
    cassert_no_null(elem);
    str_destroy(&elem->text);
    ptr_destopt(image_destroy, &elem->image, Image);
}

/*---------------------------------------------------------------------------*/

static void i_destroy_data(LData **data)
{
    cassert_no_null(data);
    cassert_no_null(*data);
    font_destroy(&(*data)->font);
    listener_destroy(&(*data)->OnSelect);
    arrst_destroy(&(*data)->elems, i_remove_elem, PElem);
    heap_delete(data, LData);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(ListBox *box, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    LData *data = view_get_data((View*)box, LData);
    uint32_t n = 0;
    cassert_no_null(data);

    n = arrst_size(data->elems, PElem);
    drawctrl_clear(p->ctx, 0, 0, data->control_width, data->control_height);

    if (n > 0)
    {
        uint32_t strow = (uint32_t)p->y / data->row_height;
        uint32_t edrow = min_u32(n, strow + ((uint32_t)p->height / data->row_height) + 2);
        uint32_t mouse_row = data->mouse_ypos != UINT32_MAX ? (data->mouse_ypos / data->row_height) : UINT32_MAX;
        PElem *elems = arrst_all(data->elems, PElem);
        uint32_t y = strow * data->row_height;
        uint32_t i;

        draw_font(p->ctx, data->font);
        for (i = strow; i < edrow; ++i)
        {
            ctrl_state_t state = data->focused == TRUE ? ekCTRL_STATE_NORMAL : ekCTRL_STATE_BKNORMAL;
            uint32_t tx = i_LEFT_PADDING;

            if (elems[i].select == TRUE)
            {
                state = data->focused == TRUE ? ekCTRL_STATE_PRESSED : ekCTRL_STATE_BKPRESSED;
                drawctrl_fill(p->ctx, 0, (int32_t)y, data->content_width, data->row_height, state);
            }
            else if (i == mouse_row)
            {
                state = data->focused == TRUE ? ekCTRL_STATE_HOT : ekCTRL_STATE_BKHOT;
                drawctrl_fill(p->ctx, 0, (int32_t)y, data->content_width, data->row_height, state);
            }

            if (data->checks == TRUE)
            {
                ctrl_state_t cstate = ekCTRL_STATE_NORMAL;
                if (i == mouse_row && data->mouse_incheck == TRUE)
                    cstate = data->check_pressed == TRUE ? ekCTRL_STATE_PRESSED : ekCTRL_STATE_HOT;

                if (elems[i].check == TRUE)
                    drawctrl_checkbox(p->ctx, (int32_t)tx, (int32_t)(y + data->check_yoffset), cstate);
                else
                    drawctrl_uncheckbox(p->ctx, (int32_t)tx, (int32_t)(y + data->check_yoffset), cstate);

                tx += data->check_width + i_LEFT_PADDING;
            }

            if (elems[i].image != NULL)
            {
                uint32_t yoffset = (data->row_height - elems[i].imgheight) / 2;
                drawctrl_image(p->ctx, elems[i].image, (int32_t)tx, (int32_t)(y + yoffset));
                tx += elems[i].imgwidth + i_LEFT_PADDING;
            }

            draw_text_color(p->ctx, elems[i].color);
            drawctrl_text(p->ctx, tc(elems[i].text), (int32_t)tx, (int32_t)(y + data->text_yoffset), state);

            if (i == data->selected && data->focused == TRUE)
                drawctrl_focus(p->ctx, 0, (int32_t)y, data->content_width, data->row_height, state);

            y += data->row_height;
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_document_size(ListBox *box, LData *data)
{
    uint32_t twidth = 0;
    uint32_t theight = 0;
    uint32_t n = arrst_size(data->elems, PElem);
    cassert_no_null(data);

    data->row_height = data->cell_height;

    if (data->checks == TRUE)
    {
        if (data->check_width == UINT32_MAX)
        {
            cassert(data->check_height == UINT32_MAX);
            data->check_width = drawctrl_check_width(NULL);
            data->check_height = drawctrl_check_height(NULL);
        }

        if (data->check_height > data->row_height)
            data->row_height = data->check_height;
    }

    arrst_foreach(elem, data->elems, PElem)
        uint32_t tw;

        {
            real32_t w, h;
            font_extents(data->font, tc(elem->text), -1, &w, &h);
            tw = (uint32_t)bmath_ceilf(w);
            unref(h);
        }

        if (elem->image != NULL)
        {
            cassert(elem->imgwidth > 0);
            cassert(elem->imgheight > 0);
            tw += elem->imgwidth + i_LEFT_PADDING;
            if (elem->imgheight > data->row_height)
                data->row_height = elem->imgheight;
        }

        if (tw > twidth)
            twidth = tw;

    arrst_end();

    if ((uint32_t)(data->row_height - data->font_height) % 2 == 0)
        data->row_height += 1;

    data->row_height += drawctrl_row_padding(NULL);
    data->text_yoffset = ((uint32_t)data->row_height - (uint32_t)data->cell_height) / 2;
    data->check_yoffset = ((uint32_t)data->row_height - (uint32_t)data->check_height) / 2;
    twidth += i_LEFT_PADDING + i_RIGHT_PADDING;

    /* GCC warning if not brackets */
    if (data->checks == TRUE)
    {
        twidth += i_LEFT_PADDING + data->check_width;
    }

	if (twidth < data->control_width)
	{
		twidth = data->control_width;
	}

    theight = data->row_height * n + i_BOTTOM_PADDING;
    if (theight < data->control_height)
        theight = data->control_height;

    data->content_width = twidth;
    view_content_size((View*)box, s2df((real32_t)twidth, (real32_t)theight), s2df(10, (real32_t)data->row_height));
}

/*---------------------------------------------------------------------------*/

static void i_OnSize(ListBox *box, Event *e)
{
    LData *data = view_get_data((View*)box, LData);
    const EvSize *p = event_params(e, EvSize);
    data->control_width = (uint32_t)p->width;
    data->control_height = (uint32_t)p->height;
    i_document_size(box, data);
}

/*---------------------------------------------------------------------------*/

static bool_t i_mouse_in_check(const LData *data, const uint32_t x, uint32_t y)
{
    if (data->checks == TRUE)
    {
        uint32_t mouse_row = data->mouse_ypos / data->row_height;
        uint32_t check_x0 = i_LEFT_PADDING;
        uint32_t check_x1 = check_x0 + data->check_width;
        uint32_t check_y0 = mouse_row * data->row_height + data->check_yoffset;
        uint32_t check_y1 = check_y0 + data->check_height;
        if (x >= check_x0 && x <= check_x1 && y >= check_y0 && y <= check_y1)
            return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_OnMove(ListBox *box, Event *e)
{
    LData *data = view_get_data((View*)box, LData);
    const EvMouse *p = event_params(e, EvMouse);
    uint32_t y = (uint32_t)p->y;
    data->mouse_ypos = y;
    data->mouse_incheck = FALSE;

    if (arrst_size(data->elems, PElem) > 0)
    {
        if (data->checks == TRUE)
            data->mouse_incheck = i_mouse_in_check(data, (uint32_t)p->x, y);

        view_update((View*)box);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnEnter(ListBox *box, Event *e)
{
    unref(box);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnExit(ListBox *box, Event *e)
{
    LData *data = view_get_data((View*)box, LData);
    data->mouse_ypos = UINT32_MAX;

    if (arrst_size(data->elems, PElem) > 0)
        view_update((View*)box);

    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnFocus(ListBox *box, Event *e)
{
    LData *data = view_get_data((View*)box, LData);
    const bool_t *p = event_params(e, bool_t);
    data->focused = *p;
    if (data->selected != UINT32_MAX)
        view_update((View*)box);
}

/*---------------------------------------------------------------------------*/

static void i_clean_select(ArrSt(PElem) *elems)
{
    arrst_foreach(elem, elems, PElem)
        elem->select = FALSE;
    arrst_end();
}

/*---------------------------------------------------------------------------*/

static void i_select(ListBox *box, LData *data, const bool_t bymouse)
{
    const char_t *text = NULL;

    if (data->selected != UINT32_MAX)
    {
        PElem *elem = arrst_get(data->elems, data->selected, PElem);

        if (bymouse == TRUE && data->multisel_mode == ekCTRL_MSEL_SINGLE)
            elem->select = !elem->select;
        else
            elem->select = TRUE;

        text = tc(elem->text);
    }

	_cell_upd_uint32(_view_cell((View*)box), data->selected);

    if (data->OnSelect != NULL)
    {
        EvButton params;
        params.state = ekGUI_ON;
        params.index = data->selected;
        params.text = text;
        listener_event(data->OnSelect, ekGUI_EVENT_LISTBOX, box, &params, NULL, ListBox, EvButton, void);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnDown(ListBox *box, Event *e)
{
    LData *data = view_get_data((View*)box, LData);
    const EvMouse *p = event_params(e, EvMouse);
    uint32_t n = arrst_size(data->elems, PElem);

    if (n > 0 && p->button == ekGUI_MOUSE_LEFT)
    {
        uint32_t y = (uint32_t)p->y;
        uint32_t sel = y / data->row_height;

        if (sel >= n)
            sel = UINT32_MAX;

        if (sel != UINT32_MAX && data->checks == TRUE)
        {
            if (data->mouse_incheck == TRUE)
            {
                PElem *elem = arrst_get(data->elems, sel, PElem);
                elem->check = !elem->check;
                data->check_pressed = TRUE;
                view_update((View*)box);
            }
        }

        if (sel != data->selected || data->multisel_mode != ekCTRL_MSEL_NO)
        {
            data->selected = sel;

            if (data->multisel == FALSE || data->multisel_mode == ekCTRL_MSEL_NO)
                i_clean_select(data->elems);

            i_select(box, data, TRUE);
            view_update((View*)box);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnUp(ListBox *box, Event *e)
{
    LData *data = view_get_data((View*)box, LData);
    const EvMouse *p = event_params(e, EvMouse);
    uint32_t n = arrst_size(data->elems, PElem);

    if (n > 0 && p->button == ekGUI_MOUSE_LEFT)
    {
        if (data->check_pressed == TRUE)
        {
            data->check_pressed = FALSE;
            view_update((View*)box);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_update_sel_top(ListBox *box, LData *data, const uint32_t scroll_y)
{
    if (data->multisel == FALSE || data->multisel_mode != ekCTRL_MSEL_BURST)
        i_clean_select(data->elems);

    i_select(box, data, FALSE);

    if (scroll_y > 0)
    {
        uint32_t ypos = data->selected * data->row_height;
        if (scroll_y > ypos)
            view_scroll_y((View*)box, (real32_t)ypos);
    }

    view_update((View*)box);
}

/*---------------------------------------------------------------------------*/

static void i_update_sel_bottom(ListBox *box, LData *data, const uint32_t scroll_y)
{
    uint32_t ypos = (data->selected + 1) * data->row_height + i_BOTTOM_PADDING;
    real32_t scroll_height = 0;

    if (data->multisel == FALSE || data->multisel_mode != ekCTRL_MSEL_BURST)
        i_clean_select(data->elems);

    i_select(box, data, FALSE);

    view_scroll_size((View*)box, NULL, &scroll_height);
    if (scroll_y + data->control_height - scroll_height < ypos)
        view_scroll_y((View*)box, (real32_t)ypos - (real32_t)data->control_height + scroll_height);

    view_update((View*)box);
}

/*---------------------------------------------------------------------------*/

static void i_OnKeyDown(ListBox *box, Event *e)
{
    LData *data = view_get_data((View*)box, LData);
    const EvKey *p = event_params(e, EvKey);
    uint32_t n = arrst_size(data->elems, PElem);

    if (n > 0)
    {
        uint32_t scroll_x, scroll_y;

        {
            V2Df pos;
            view_viewport((View*)box, &pos, NULL);
            scroll_x = (uint32_t)pos.x;
            scroll_y = (uint32_t)pos.y;
        }

        if (p->key == ekKEY_UP)
        {
            bool_t update = FALSE;
            if (data->selected == UINT32_MAX)
            {
                data->selected = 0;
                update = TRUE;
            }
            else if (data->selected > 0)
            {
                data->selected -= 1;
                update = TRUE;
            }

            if (update == TRUE)
                i_update_sel_top(box, data, scroll_y);
        }
        else if (p->key == ekKEY_DOWN)
        {
            bool_t update = FALSE;
            if (data->selected == UINT32_MAX)
            {
                data->selected = n - 1;
                update = TRUE;
            }
            else if (data->selected < n - 1)
            {
                data->selected += 1;
                update = TRUE;
            }

            if (update == TRUE)
                i_update_sel_bottom(box, data, scroll_y);
        }
        else if (p->key == ekKEY_HOME)
        {
            if (data->selected != 0 || scroll_y > 0)
            {
                data->selected = 0;
                i_update_sel_top(box, data, scroll_y);
            }
        }
        else if (p->key == ekKEY_END)
        {
            if (data->selected != n - 1)
            {
                data->selected = n - 1;
                i_update_sel_bottom(box, data, scroll_y);
            }
        }
        else if (p->key == ekKEY_PAGEUP)
        {
            if (data->selected != 0)
            {
                if (data->selected == UINT32_MAX)
                {
                    data->selected = 0;
                }
                else
                {
                    uint32_t psize = data->control_height / data->row_height;
                    if (data->selected > psize)
                        data->selected -= psize;
                    else
                        data->selected = 0;
                }

                i_update_sel_top(box, data, scroll_y);
            }
        }
        else if (p->key == ekKEY_PAGEDOWN)
        {
            if (data->selected != n - 1)
            {
                if (data->selected == UINT32_MAX)
                {
                    data->selected = n - 1;
                }
                else
                {
                    uint32_t psize = data->control_height / data->row_height;
                    if (data->selected + psize < n - 1)
                        data->selected += psize;
                    else
                        data->selected = n - 1;
                }

                i_update_sel_bottom(box, data, scroll_y);
            }
        }
        else if (p->key == ekKEY_LEFT)
        {
            if (data->content_width > data->control_width)
            {
                if (scroll_x > 0)
                {
                    view_scroll_x((View*)box, (real32_t)(scroll_x > 10 ? scroll_x - 10 : 0));
                    view_update((View*)box);
                }
            }
        }
        else if (p->key == ekKEY_RIGHT)
        {
            real32_t scroll_width = 0;
            view_scroll_size((View*)box, &scroll_width, NULL);
            if (data->content_width > data->control_width - scroll_width)
            {
                if (scroll_x < data->content_width - data->control_width + scroll_width)
                {
                    view_scroll_x((View*)box, (real32_t)(scroll_x + 10));
                    view_update((View*)box);
                }
            }
        }
        else if (p->key == ekKEY_SPACE)
        {
            if (data->checks == TRUE && data->selected != UINT32_MAX)
            {
                PElem *elem = arrst_get(data->elems, data->selected, PElem);
                elem->check = !elem->check;
                view_update((View*)box);
            }
        }
        else if (data->multisel == TRUE)
        {
            ctrl_msel_t msel = drawctrl_multisel(NULL, p->key);
            if (msel != ekCTRL_MSEL_NO)
                data->multisel_mode = msel;
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnKeyUp(ListBox *box, Event *e)
{
    LData *data = view_get_data((View*)box, LData);
    const EvKey *p = event_params(e, EvKey);
    uint32_t n = arrst_size(data->elems, PElem);

    if (n > 0)
    {
        if (data->multisel == TRUE)
        {
            ctrl_msel_t msel = drawctrl_multisel(NULL, p->key);
            if (msel != ekCTRL_MSEL_NO)
                data->multisel_mode = ekCTRL_MSEL_NO;
        }
    }
}

/*---------------------------------------------------------------------------*/


static void i_set_empty(ListBox *listbox)
{
	LData *data = view_get_data((View*)listbox, LData);
	i_clean_select(data->elems);
	view_update((View*)listbox);
}

/*---------------------------------------------------------------------------*/

static void i_set_uint32(ListBox *listbox, const uint32_t value)
{
	LData *data = view_get_data((View*)listbox, LData);
	i_clean_select(data->elems);

	if (value < arrst_size(data->elems, PElem))
	{
		PElem *elem = arrst_get(data->elems, value, PElem);
		elem->select = TRUE;
		data->selected = value;
	}
	else
	{
		data->selected = UINT32_MAX;
	}

	view_update((View*)listbox);
}

/*---------------------------------------------------------------------------*/

ListBox *listbox_create(void)
{
    View *view = _view_create(ekVIEW_HSCROLL | ekVIEW_VSCROLL | ekVIEW_BORDER | ekVIEW_CONTROL | ekVIEW_NOERASE);
    LData *data = i_create_data();
    view_data(view, &data, i_destroy_data, LData);
    view_OnDraw(view, listener((ListBox*)view, i_OnDraw, ListBox));
    view_OnSize(view, listener((ListBox*)view, i_OnSize, ListBox));
    view_OnMove(view, listener((ListBox*)view, i_OnMove, ListBox));
    view_OnEnter(view, listener((ListBox*)view, i_OnEnter, ListBox));
    view_OnExit(view, listener((ListBox*)view, i_OnExit, ListBox));
    view_OnFocus(view, listener((ListBox*)view, i_OnFocus, ListBox));
    view_OnDown(view, listener((ListBox*)view, i_OnDown, ListBox));
    view_OnUp(view, listener((ListBox*)view, i_OnUp, ListBox));
    view_OnKeyDown(view, listener((ListBox*)view, i_OnKeyDown, ListBox));
    view_OnKeyUp(view, listener((ListBox*)view, i_OnKeyUp, ListBox));
    _view_set_subtype(view, "ListBox");
	view_OnUInt32(view, (FPtr_gctx_set_uint32)i_set_uint32);
	view_OnEmpty(view, (FPtr_gctx_call)i_set_empty);
    i_document_size((ListBox*)view, view_get_data(view, LData));
    return (ListBox*)view;
}

/*---------------------------------------------------------------------------*/

void listbox_OnSelect(ListBox *box, Listener *listener)
{
    LData *data = view_get_data((View*)box, LData);
    cassert_no_null(data);
    listener_update(&data->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void listbox_size(ListBox *box, S2Df size)
{
    view_size((View*)box, size);
}

/*---------------------------------------------------------------------------*/

void listbox_checkbox(ListBox *box, const bool_t show)
{
    LData *data = view_get_data((View*)box, LData);
    cassert_no_null(data);
    if (data->checks != show)
    {
        data->checks = show;
        i_document_size(box, data);
        view_update((View*)box);
    }
}

/*---------------------------------------------------------------------------*/

void listbox_multisel(ListBox *box, const bool_t multisel)
{
    LData *data = view_get_data((View*)box, LData);
    cassert_no_null(data);
    if (data->multisel != multisel)
    {
        if (multisel == FALSE)
        {
            i_clean_select(data->elems);
            if (data->selected != UINT32_MAX)
            {
                PElem *elem = arrst_get(data->elems, data->selected, PElem);
                elem->select = TRUE;
            }
        }

        data->multisel = multisel;
        view_update((View*)box);
    }
}

/*---------------------------------------------------------------------------*/

void listbox_add_elem(ListBox *box, const char_t *text, const Image *image)
{
    LData *data = view_get_data((View*)box, LData);
    const char_t *ltext = NULL;
    const Image *limage = NULL;
    PElem *elem = NULL;
    cassert_no_null(data);
    elem = arrst_new0(data->elems, PElem);
    ltext = _gui_respack_text(text, &elem->resid);
    limage = _gui_respack_image((const ResId)image, NULL);
    elem->text = str_c(ltext);
    elem->color = kCOLOR_DEFAULT;

    if (limage != NULL)
    {
        elem->image = image_copy(limage);
        elem->imgwidth = image_width(limage);
        elem->imgheight = image_height(limage);
    }

    i_document_size(box, data);
    view_update((View*)box);
}

/*---------------------------------------------------------------------------*/

void listbox_set_elem(ListBox *box, const uint32_t index, const char_t *text, const Image *image)
{
    LData *data = view_get_data((View*)box, LData);
    PElem *elem = NULL;
    const char_t *ltext = NULL;
    cassert_no_null(data);
    elem = arrst_get(data->elems, index, PElem);
    ltext = _gui_respack_text(text, &elem->resid);
    str_upd(&elem->text, ltext);

    if (image != NULL)
    {
        const Image *limage = _gui_respack_image((const ResId)image, NULL);
        ptr_destopt(image_destroy, &elem->image, Image);
        elem->image = image_copy(limage);
        elem->imgwidth = image_width(limage);
        elem->imgheight = image_height(limage);
    }

    i_document_size(box, data);
    view_update((View*)box);
}

/*---------------------------------------------------------------------------*/

void listbox_clear(ListBox *box)
{
    LData *data = view_get_data((View*)box, LData);
    cassert_no_null(data);
    arrst_clear(data->elems, i_remove_elem, PElem);
    i_document_size(box, data);
    view_update((View*)box);
}

/*---------------------------------------------------------------------------*/

void listbox_color(ListBox *box, const uint32_t index, const color_t color)
{
    PElem *elem = NULL;
    LData *data = view_get_data((View*)box, LData);
    cassert_no_null(data);
    elem = arrst_get(data->elems, index, PElem);
    elem->color = color;
    view_update((View*)box);
}

/*---------------------------------------------------------------------------*/

void listbox_select(ListBox *box, const uint32_t index, const bool_t select)
{
    LData *data = view_get_data((View*)box, LData);
    cassert_no_null(data);
    cassert(index == UINT32_MAX || index < arrst_size(data->elems, PElem));

    if (select == TRUE)
    {
        if (data->multisel == FALSE)
            i_clean_select(data->elems);

        data->selected = index;
    }

    if (index != UINT32_MAX)
    {
        PElem *elem = arrst_get(data->elems, index, PElem);
        elem->select = select;
    }
}

/*---------------------------------------------------------------------------*/

void listbox_check(ListBox *box, const uint32_t index, const bool_t check)
{
    LData *data = view_get_data((View*)box, LData);
    PElem *elem = NULL;
    cassert_no_null(data);
    elem = arrst_get(data->elems, index, PElem);
    elem->check = check;
}

/*---------------------------------------------------------------------------*/

uint32_t listbox_count(const ListBox *box)
{
    LData *data = view_get_data((View*)box, LData);
    cassert_no_null(data);
    return arrst_size(data->elems, PElem);
}

/*---------------------------------------------------------------------------*/

const char_t *listbox_text(const ListBox *box, const uint32_t index)
{
    LData *data = view_get_data((View*)box, LData);
    const PElem *elem = NULL;
    cassert_no_null(data);
    elem = arrst_get(data->elems, index, PElem);
    return tc(elem->text);
}

/*---------------------------------------------------------------------------*/

bool_t listbox_selected(const ListBox *box, uint32_t index)
{
    LData *data = view_get_data((View*)box, LData);
    const PElem *elem = NULL;
    cassert_no_null(data);
    elem = arrst_get(data->elems, index, PElem);
    return elem->select;
}

/*---------------------------------------------------------------------------*/

bool_t listbox_checked(const ListBox *box, uint32_t index)
{
    LData *data = view_get_data((View*)box, LData);
    const PElem *elem = NULL;
    cassert_no_null(data);
    elem = arrst_get(data->elems, index, PElem);
    return elem->check;
}

/*---------------------------------------------------------------------------*/

uint32_t _listbox_count(const ListBox *listbox)
{
	return listbox_count(listbox);
}

/*---------------------------------------------------------------------------*/

void _listbox_add_enum_item(ListBox *listbox, const char_t *text)
{
	listbox_add_elem(listbox, text, NULL);
}

