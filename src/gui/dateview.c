/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: dateview.c
 *
 */

/* Date view */

#include "dateview.h"
#include "cell.inl"
#include "drawctrl.inl"
#include "gui.h"
#include "view.h"
#include "vctrl.inl"
#include <draw2d/color.h>
#include <draw2d/dctx.h>
#include <draw2d/draw.h>
#include <draw2d/font.h>
#include <core/date.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <osbs/btime.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/unicode.h>

typedef enum _hit_t
{
    ekHIT_NONE,
    ekHIT_PREV_YEAR,
    ekHIT_PREV_MONTH,
    ekHIT_NEXT_MONTH,
    ekHIT_NEXT_YEAR,
    ekHIT_DAY
} hit_t;

typedef struct _dlayout_t DLayout;
typedef struct _ddata_t DData;

struct _dlayout_t
{
    bool_t header;
    bool_t wdays;
    uint32_t rows;
    int32_t x0;
    int32_t y0;
    int32_t ygrid;
    uint32_t total;
};

struct _ddata_t
{
    Font *font;
    Font *bfont;
    String *months[12];
    String *wdays[7];
    week_day_t first_wday;
    int32_t today;
    int32_t selected;
    int32_t focus;
    int32_t top;
    int32_t head_year;
    uint32_t head_month;
    bool_t has_selected;
    bool_t focused;
    bool_t mouse_in;
    real32_t mouse_x;
    real32_t mouse_y;
    real32_t wheel;
    real32_t text_height;
    real32_t btext_height;
    real32_t day_width[31];
    real32_t bday_width[31];
    real32_t wday_width[7];
    uint32_t cell_width;
    uint32_t cell_height;
    uint32_t header_height;
    uint32_t wdays_height;
    uint32_t button_width;
    uint32_t width;
    uint32_t height;
    Listener *OnSelect;
};

/*---------------------------------------------------------------------------*/

static const uint32_t i_MARGIN = 4;
static const int32_t i_MIN_DAY = -12687794;
static const int32_t i_MAX_DAY = 11248737;
static const int32_t i_MIN_YEAR = -32768;
static const int32_t i_MAX_YEAR = 32767;
static const char_t *i_DAYS[31] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"};

/*---------------------------------------------------------------------------*/

static void i_OnDraw(DData *data, Event *e);
static void i_OnSize(DData *data, Event *e);
static void i_OnExit(DData *data, Event *e);
static void i_OnMoved(DData *data, Event *e);
static void i_OnDown(DData *data, Event *e);
static void i_OnWheel(DData *data, Event *e);
static void i_OnKeyDown(DData *data, Event *e);
static void i_OnFocus(DData *data, Event *e);
static void i_destroy_data(DData **data);
static void i_natural(DateView *view, const uint32_t di, real32_t *dim0, real32_t *dim1);
static void i_set_empty(DateView *view);
static void i_set_int32(DateView *view, const int32_t value);

/*---------------------------------------------------------------------------*/

static VCtrlTbl i_DATEVIEW_TLB = {
    "DateView",
    (FPtr_event_handler)i_OnDraw,
    NULL, /* OnOverlay */
    (FPtr_event_handler)i_OnSize,
    NULL, /* OnEnter */
    (FPtr_event_handler)i_OnExit,
    (FPtr_event_handler)i_OnMoved,
    (FPtr_event_handler)i_OnDown,
    NULL, /* OnUp */
    NULL, /* OnClick */
    NULL, /* OnDrag */
    (FPtr_event_handler)i_OnWheel,
    (FPtr_event_handler)i_OnKeyDown,
    NULL, /* OnKeyUp */
    (FPtr_event_handler)i_OnFocus,
    NULL, /* OnResignFocus */
    NULL, /* OnAcceptFocus */
    NULL, /* OnScroll */
    (FPtr_destroy)i_destroy_data,
    NULL, /* func_locale */
    (FPtr_natural)i_natural,
    (FPtr_gctx_call)i_set_empty,
    NULL, /* func_uint32 */
    (FPtr_set_int32)i_set_int32,
    NULL /* func_image*/};

/*---------------------------------------------------------------------------*/

static int32_t i_day_number(const int32_t year, const uint32_t month, const uint32_t mday)
{
    Date date = kDATE_NULL;
    date.year = (int16_t)year;
    date.month = (uint8_t)month;
    date.mday = (uint8_t)mday;
    return date_to_days(&date);
}

/*---------------------------------------------------------------------------*/

static void i_civil(const int32_t day, int32_t *year, uint32_t *month, uint32_t *mday)
{
    Date date = date_from_days(day);
    ptr_assign(year, (int32_t)date.year);
    ptr_assign(month, (uint32_t)date.month);
    ptr_assign(mday, (uint32_t)date.mday);
}

/*---------------------------------------------------------------------------*/

static int32_t i_from_date(const Date *date)
{
    cassert_no_null(date);
    return date_to_days(date);
}

/*---------------------------------------------------------------------------*/

static Date i_to_date(const int32_t day)
{
    return date_from_days(day);
}

/*---------------------------------------------------------------------------*/

static bool_t i_in_range(const int32_t day)
{
    if (day >= i_MIN_DAY && day <= i_MAX_DAY)
        return TRUE;
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_wday(const int32_t day)
{
    int32_t r = day % 7;
    if (r < 0)
        r += 7;
    return (uint32_t)((r + 4) % 7);
}

/*---------------------------------------------------------------------------*/

static int32_t i_week_start(const DData *data, const int32_t day)
{
    uint32_t wday = i_wday(day);
    uint32_t offset = (wday + 7 - (uint32_t)data->first_wday) % 7;
    return day - (int32_t)offset;
}

/*---------------------------------------------------------------------------*/

static int32_t i_clamp_day(const int32_t day)
{
    if (day < i_MIN_DAY)
        return i_MIN_DAY;
    if (day > i_MAX_DAY)
        return i_MAX_DAY;
    return day;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_names(DData *data)
{
    uint32_t i;
    cassert_no_null(data);
    for (i = 0; i < 12; ++i)
        str_destopt(&data->months[i]);
    for (i = 0; i < 7; ++i)
        str_destopt(&data->wdays[i]);
}

/*---------------------------------------------------------------------------*/

static void i_destroy_data(DData **data)
{
    cassert_no_null(data);
    cassert_no_null(*data);
    i_destroy_names(*data);
    font_destroy(&(*data)->font);
    font_destroy(&(*data)->bfont);
    listener_destroy(&(*data)->OnSelect);
    heap_delete(data, DData);
}

/*---------------------------------------------------------------------------*/

static void i_locale_months(DData *data)
{
    uint32_t i;
    cassert_no_null(data);
    for (i = 0; i < 12; ++i)
    {
        char_t name[128];
        btime_month_name((month_t)(i + 1), name, sizeof(name));
        str_destopt(&data->months[i]);
        data->months[i] = str_c(name);
    }
}

/*---------------------------------------------------------------------------*/

static void i_locale_wdays(DData *data)
{
    uint32_t i;
    cassert_no_null(data);
    for (i = 0; i < 7; ++i)
    {
        char_t name[128];
        char_t initial[8];
        uint32_t codepoint = 0;
        uint32_t upper = 0;
        uint32_t nbytes = 0;
        btime_wday_name((week_day_t)i, name, sizeof(name));
        codepoint = unicode_to_u32(name, ekUTF8);
        upper = unicode_toupper(codepoint);
        nbytes = unicode_to_char(upper, initial, ekUTF8);
        initial[nbytes] = '\0';
        str_destopt(&data->wdays[i]);
        data->wdays[i] = str_c(initial);
    }
}

/*---------------------------------------------------------------------------*/

static real32_t i_text_width(const Font *font, const char_t *text)
{
    real32_t width = 0;
    real32_t height = 0;
    font_extents(font, text, -1, &width, &height);
    unref(height);
    return width;
}

/*---------------------------------------------------------------------------*/

static void i_metrics(DData *data)
{
    real32_t title_width = 0;
    real32_t digit_width = 0;
    real32_t cell_text = 0;
    uint32_t pad = 0;
    uint32_t header_width = 0;
    uint32_t i;
    cassert_no_null(data);
    data->text_height = font_height(data->font);
    data->btext_height = font_height(data->bfont);
    pad = (uint32_t)bmath_ceilf(data->text_height * .6f);

    for (i = 0; i < 31; ++i)
    {
        data->day_width[i] = i_text_width(data->font, i_DAYS[i]);
        data->bday_width[i] = i_text_width(data->bfont, i_DAYS[i]);
    }

    for (i = 0; i < 7; ++i)
        data->wday_width[i] = i_text_width(data->font, tc(data->wdays[i]));

    for (i = 0; i < 10; ++i)
    {
        char_t digit[2];
        real32_t width = 0;
        digit[0] = (char_t)('0' + i);
        digit[1] = '\0';
        width = i_text_width(data->bfont, digit);
        if (width > digit_width)
            digit_width = width;
    }

    for (i = 0; i < 12; ++i)
    {
        real32_t width = i_text_width(data->bfont, tc(data->months[i]));
        if (width > title_width)
            title_width = width;
    }

    title_width += i_text_width(data->bfont, " ") + 4 * digit_width;
    cell_text = i_text_width(data->bfont, "OO");

    if (data->btext_height > data->text_height)
        data->cell_height = (uint32_t)bmath_ceilf(data->btext_height) + pad;
    else
        data->cell_height = (uint32_t)bmath_ceilf(data->text_height) + pad;

    data->cell_width = (uint32_t)bmath_ceilf(cell_text) + pad;
    if (data->cell_width < data->cell_height)
        data->cell_width = data->cell_height;

    data->wdays_height = (uint32_t)bmath_ceilf(data->text_height) + pad / 2;
    data->header_height = (uint32_t)bmath_ceilf(data->btext_height) + pad;
    data->button_width = data->header_height;
    header_width = (uint32_t)bmath_ceilf(title_width) + 4 * data->button_width + 2 * pad;

    if (header_width > 7 * data->cell_width)
        data->cell_width = (header_width + 6) / 7;
}

/*---------------------------------------------------------------------------*/

static void i_layout(const DData *data, DLayout *layout)
{
    uint32_t avail = 0;
    cassert_no_null(data);
    cassert_no_null(layout);
    layout->header = FALSE;
    layout->wdays = FALSE;
    layout->total = 7 * data->cell_width;
    layout->y0 = (int32_t)i_MARGIN;

    if (data->height > 2 * i_MARGIN)
        avail = data->height - 2 * i_MARGIN;

    if (avail >= data->header_height + data->wdays_height + data->cell_height)
    {
        layout->header = TRUE;
        layout->wdays = TRUE;
        layout->rows = (avail - data->header_height - data->wdays_height) / data->cell_height;
    }
    else if (avail >= data->wdays_height + data->cell_height)
    {
        layout->wdays = TRUE;
        layout->rows = (avail - data->wdays_height) / data->cell_height;
    }
    else
    {
        layout->rows = avail / data->cell_height;
        if (layout->rows == 0)
            layout->rows = 1;
    }

    if (data->width > layout->total + 2 * i_MARGIN)
        layout->x0 = (int32_t)((data->width - layout->total) / 2);
    else
        layout->x0 = (int32_t)i_MARGIN;

    layout->ygrid = layout->y0;

    if (layout->header == TRUE)
        layout->ygrid += (int32_t)data->header_height;

    if (layout->wdays == TRUE)
        layout->ygrid += (int32_t)data->wdays_height;
}

/*---------------------------------------------------------------------------*/

static void i_update_header(DData *data, const uint32_t rows)
{
    int32_t years[3];
    uint32_t months[3];
    uint32_t counts[3];
    uint32_t nslots = 0;
    uint32_t ndays = 0;
    uint32_t best = 0;
    uint32_t i, j;
    cassert_no_null(data);

    if (rows < 6)
        ndays = rows * 7;
    else
        ndays = 42;

    for (i = 0; i < ndays; ++i)
    {
        int32_t year = 0;
        uint32_t month = 0;
        bool_t found = FALSE;
        i_civil(data->top + (int32_t)i, &year, &month, NULL);

        for (j = 0; j < nslots; ++j)
        {
            if (years[j] == year && months[j] == month)
            {
                counts[j] += 1;
                found = TRUE;
            }
        }

        if (found == FALSE)
        {
            cassert(nslots < 3);
            years[nslots] = year;
            months[nslots] = month;
            counts[nslots] = 1;
            nslots += 1;
        }
    }

    for (i = 1; i < nslots; ++i)
    {
        if (counts[i] > counts[best])
            best = i;
    }

    for (i = 0; i < nslots; ++i)
    {
        if (counts[i] == counts[best] && years[i] == data->head_year && months[i] == data->head_month)
            best = i;
    }

    data->head_year = years[best];
    data->head_month = months[best];
}

/*---------------------------------------------------------------------------*/

static void i_clamp_top(DData *data)
{
    int32_t min_top = i_week_start(data, i_MIN_DAY);
    int32_t max_top = i_week_start(data, i_MAX_DAY);
    if (data->top < min_top)
        data->top = min_top;
    if (data->top > max_top)
        data->top = max_top;
}

/*---------------------------------------------------------------------------*/

static void i_recolocate(DData *data)
{
    DLayout layout;
    int32_t year = 0;
    uint32_t month = 0;
    int32_t first = 0;
    int32_t ideal = 0;
    int32_t fweek = 0;
    cassert_no_null(data);
    i_layout(data, &layout);
    i_civil(data->focus, &year, &month, NULL);
    first = i_day_number(year, month, 1);
    ideal = i_week_start(data, first);
    fweek = i_week_start(data, data->focus);

    if ((uint32_t)((fweek - ideal) / 7) < layout.rows)
        data->top = ideal;
    else
        data->top = fweek - (int32_t)(layout.rows - 1) * 7;

    i_clamp_top(data);
    i_update_header(data, layout.rows);
}

/*---------------------------------------------------------------------------*/

static void i_ensure_visible(DData *data)
{
    DLayout layout;
    int32_t fweek = 0;
    cassert_no_null(data);
    i_layout(data, &layout);
    fweek = i_week_start(data, data->focus);

    if (fweek < data->top)
        data->top = fweek;
    else if (fweek >= data->top + (int32_t)layout.rows * 7)
        data->top = fweek - (int32_t)(layout.rows - 1) * 7;

    i_clamp_top(data);
    i_update_header(data, layout.rows);
}

/*---------------------------------------------------------------------------*/

static void i_scroll(DData *data, const int32_t weeks)
{
    DLayout layout;
    cassert_no_null(data);
    i_layout(data, &layout);
    data->top += weeks * 7;
    i_clamp_top(data);
    i_update_header(data, layout.rows);
}

/*---------------------------------------------------------------------------*/

static void i_target_month(const int32_t from_year, const uint32_t from_month, const int32_t nmonths, int32_t *year, uint32_t *month)
{
    int32_t total = from_year * 12 + (int32_t)from_month - 1 + nmonths;
    int32_t rem = total % 12;
    cassert_no_null(year);
    cassert_no_null(month);

    if (rem < 0)
        rem += 12;

    *year = (total - rem) / 12;
    *month = (uint32_t)rem + 1;

    if (*year < i_MIN_YEAR)
    {
        *year = i_MIN_YEAR;
        *month = 1;
    }

    if (*year > i_MAX_YEAR)
    {
        *year = i_MAX_YEAR;
        *month = 12;
    }
}

/*---------------------------------------------------------------------------*/

static void i_show_month(DData *data, const int32_t nmonths)
{
    DLayout layout;
    int32_t year = 0;
    uint32_t month = 0;
    int32_t first = 0;
    cassert_no_null(data);
    i_target_month(data->head_year, data->head_month, nmonths, &year, &month);
    first = i_day_number(year, month, 1);
    i_layout(data, &layout);
    data->top = i_week_start(data, first);
    data->head_year = year;
    data->head_month = month;
    i_clamp_top(data);
    i_update_header(data, layout.rows);
}

/*---------------------------------------------------------------------------*/

static void i_navigate(DData *data, const int32_t nmonths)
{
    int32_t fyear = 0;
    uint32_t fmonth = 0;
    int32_t year = 0;
    uint32_t month = 0;
    uint32_t mday = 0;
    uint32_t mdays = 0;
    cassert_no_null(data);
    i_civil(data->focus, &fyear, &fmonth, &mday);
    i_target_month(fyear, fmonth, nmonths, &year, &month);
    mdays = date_month_days((int16_t)year, (month_t)month);

    if (mday > mdays)
        mday = mdays;

    data->focus = i_day_number(year, month, mday);
    data->head_year = year;
    data->head_month = month;
    i_recolocate(data);
}

/*---------------------------------------------------------------------------*/

static int32_t i_button_x(const DData *data, const DLayout *layout, const uint32_t index)
{
    cassert_no_null(data);
    cassert_no_null(layout);
    switch (index)
    {
    case 0:
        return layout->x0;
    case 1:
        return layout->x0 + (int32_t)data->button_width;
    case 2:
        return layout->x0 + (int32_t)(layout->total - 2 * data->button_width);
    case 3:
        return layout->x0 + (int32_t)(layout->total - data->button_width);
    default:
        cassert_default(index);
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static hit_t i_hit(const DData *data, const DLayout *layout, const real32_t x, const real32_t y, int32_t *day)
{
    int32_t px = (int32_t)bmath_floorf(x);
    int32_t py = (int32_t)bmath_floorf(y);
    cassert_no_null(data);
    cassert_no_null(layout);

    if (layout->header == TRUE && py >= layout->y0 && py < layout->y0 + (int32_t)data->header_height)
    {
        uint32_t i;
        for (i = 0; i < 4; ++i)
        {
            int32_t bx = i_button_x(data, layout, i);
            if (px >= bx && px < bx + (int32_t)data->button_width)
                return (hit_t)(ekHIT_PREV_YEAR + i);
        }

        return ekHIT_NONE;
    }

    if (py >= layout->ygrid && px >= layout->x0 && px < layout->x0 + (int32_t)layout->total)
    {
        uint32_t row = (uint32_t)(py - layout->ygrid) / data->cell_height;
        uint32_t col = (uint32_t)(px - layout->x0) / data->cell_width;
        if (row < layout->rows && col < 7)
        {
            ptr_assign(day, data->top + (int32_t)(row * 7 + col));
            return ekHIT_DAY;
        }
    }

    return ekHIT_NONE;
}

/*---------------------------------------------------------------------------*/

static color_t i_dim_color(void)
{
    real32_t r1 = 0, g1 = 0, b1 = 0;
    real32_t r2 = 0, g2 = 0, b2 = 0;
    color_t label = gui_label_color();
    color_t back = gui_view_color();
    color_get_rgbf(label, &r1, &g1, &b1);
    color_get_rgbf(back, &r2, &g2, &b2);
    return color_rgbaf((r1 + r2) * .5f, (g1 + g2) * .5f, (b1 + b2) * .5f, 1);
}

/*---------------------------------------------------------------------------*/

static void i_draw_chevron(DCtx *ctx, const int32_t tx, const int32_t cy, const int32_t h, const uint32_t stroke, const bool_t left)
{
    V2Df points[3];
    int32_t dir = -1;
    uint32_t i;
    if (left == TRUE)
        dir = 1;

    for (i = 0; i < stroke; ++i)
    {
        int32_t x = tx + dir * (int32_t)i;
        points[0].x = (real32_t)(x + dir * h);
        points[0].y = (real32_t)(cy - h);
        points[1].x = (real32_t)x;
        points[1].y = (real32_t)cy;
        points[2].x = (real32_t)(x + dir * h);
        points[2].y = (real32_t)(cy + h);
        draw_polyline(ctx, FALSE, points, 3);
    }
}

/*---------------------------------------------------------------------------*/

static void i_draw_button(const DData *data, DCtx *ctx, const int32_t bx, const int32_t by, const uint32_t index)
{
    int32_t h = 0;
    int32_t d = 0;
    int32_t width = 0;
    int32_t x0 = 0;
    int32_t cy = 0;
    bool_t left = FALSE;
    cassert_no_null(data);
    h = (int32_t)bmath_roundf(data->text_height * .33f);
    if (h < 3)
        h = 3;

    if (index <= 1)
        left = TRUE;

    cy = by + (int32_t)data->header_height / 2;

    if (index == 1 || index == 2)
    {
        width = h + 1;
        x0 = bx + ((int32_t)data->button_width - width) / 2;
        if (left == TRUE)
            i_draw_chevron(ctx, x0, cy, h, 2, TRUE);
        else
            i_draw_chevron(ctx, x0 + width, cy, h, 2, FALSE);
    }
    else
    {
        d = h / 2 + 1;
        width = h + d;
        x0 = bx + ((int32_t)data->button_width - width) / 2;
        if (left == TRUE)
        {
            i_draw_chevron(ctx, x0, cy, h, 1, TRUE);
            i_draw_chevron(ctx, x0 + d, cy, h, 1, TRUE);
        }
        else
        {
            i_draw_chevron(ctx, x0 + width, cy, h, 1, FALSE);
            i_draw_chevron(ctx, x0 + width - d, cy, h, 1, FALSE);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_draw_header(const DData *data, DCtx *ctx, const DLayout *layout, const hit_t hit, const ctrl_state_t normal, const ctrl_state_t hot)
{
    String *title = NULL;
    real32_t twidth = 0;
    int32_t ty = 0;
    uint32_t i;
    cassert_no_null(data);
    cassert_no_null(layout);

    for (i = 0; i < 4; ++i)
    {
        if (hit == (hit_t)(ekHIT_PREV_YEAR + i))
        {
            int32_t bx = i_button_x(data, layout, i);
            drawctrl_fill(ctx, bx, layout->y0, data->button_width, data->header_height, hot);
        }
    }

    draw_line_color(ctx, gui_label_color());
    draw_line_width(ctx, 1);
    draw_antialias(ctx, FALSE);

    for (i = 0; i < 4; ++i)
    {
        int32_t bx = i_button_x(data, layout, i);
        i_draw_button(data, ctx, bx, layout->y0, i);
    }

    draw_antialias(ctx, TRUE);

    title = str_printf("%s %d", tc(data->months[data->head_month - 1]), (int)data->head_year);
    twidth = i_text_width(data->bfont, tc(title));
    draw_font(ctx, data->bfont);
    ty = layout->y0 + (int32_t)(((real32_t)data->header_height - data->btext_height) / 2);
    drawctrl_text(ctx, tc(title), layout->x0 + (int32_t)(((real32_t)layout->total - twidth) / 2), ty, normal);
    str_destroy(&title);
}

/*---------------------------------------------------------------------------*/

static void i_draw_wdays(const DData *data, DCtx *ctx, const DLayout *layout, const ctrl_state_t normal)
{
    int32_t y = layout->y0;
    uint32_t i;
    cassert_no_null(data);
    cassert_no_null(layout);

    if (layout->header == TRUE)
        y += (int32_t)data->header_height;

    draw_font(ctx, data->font);

    for (i = 0; i < 7; ++i)
    {
        uint32_t wday = ((uint32_t)data->first_wday + i) % 7;
        int32_t x = layout->x0 + (int32_t)(i * data->cell_width) + (int32_t)(((real32_t)data->cell_width - data->wday_width[wday]) / 2);
        drawctrl_text(ctx, tc(data->wdays[wday]), x, y, normal);
    }

    y = layout->ygrid - 1;
    draw_line_color(ctx, gui_line_color());
    drawctrl_line(ctx, layout->x0, y, layout->x0 + (int32_t)(7 * data->cell_width), y);
}

/*---------------------------------------------------------------------------*/

static void i_draw_days(const DData *data, DCtx *ctx, const DLayout *layout, const hit_t hit, const int32_t hit_day, const ctrl_state_t normal, const ctrl_state_t hot, const ctrl_state_t pressed)
{
    color_t dim = i_dim_color();
    const Font *current = NULL;
    uint32_t i, j;
    cassert_no_null(data);
    cassert_no_null(layout);

    for (i = 0; i < layout->rows; ++i)
    {
        for (j = 0; j < 7; ++j)
        {
            int32_t day = data->top + (int32_t)(i * 7 + j);
            int32_t cx = layout->x0 + (int32_t)(j * data->cell_width);
            int32_t cy = layout->ygrid + (int32_t)(i * data->cell_height);
            int32_t year = 0;
            uint32_t month = 0;
            uint32_t mday = 0;
            ctrl_state_t state = normal;
            const Font *font = data->font;
            real32_t twidth = 0;
            real32_t theight = data->text_height;
            int32_t tx = 0;
            int32_t ty = 0;

            if (i_in_range(day) == FALSE)
                continue;

            i_civil(day, &year, &month, &mday);

            if (data->has_selected == TRUE && day == data->selected)
            {
                state = pressed;
                drawctrl_fill(ctx, cx + 1, cy + 1, data->cell_width - 2, data->cell_height - 2, state);
            }
            else if (hit == ekHIT_DAY && day == hit_day)
            {
                state = hot;
                drawctrl_fill(ctx, cx + 1, cy + 1, data->cell_width - 2, data->cell_height - 2, state);
            }

            if (state != pressed && (year != data->head_year || month != data->head_month))
                draw_text_color(ctx, dim);
            else
                draw_text_color(ctx, kCOLOR_DEFAULT);

            if (day == data->today)
            {
                font = data->bfont;
                twidth = data->bday_width[mday - 1];
                theight = data->btext_height;
            }
            else
            {
                twidth = data->day_width[mday - 1];
            }

            if (font != current)
            {
                draw_font(ctx, font);
                current = font;
            }

            tx = cx + (int32_t)(((real32_t)data->cell_width - twidth) / 2);
            ty = cy + (int32_t)(((real32_t)data->cell_height - theight) / 2);
            drawctrl_text(ctx, i_DAYS[mday - 1], tx, ty, state);

            if (day == data->today)
            {
                int32_t x0 = cx + 1;
                int32_t y0 = cy + 1;
                int32_t x1 = cx + (int32_t)data->cell_width - 2;
                int32_t y1 = cy + (int32_t)data->cell_height - 2;
                draw_line_color(ctx, gui_accent_color());
                drawctrl_line(ctx, x0, y0, x1, y0);
                drawctrl_line(ctx, x1, y0, x1, y1);
                drawctrl_line(ctx, x1, y1, x0, y1);
                drawctrl_line(ctx, x0, y1, x0, y0);
            }

            if (data->focused == TRUE && day == data->focus)
                drawctrl_focus(ctx, cx + 2, cy + 2, data->cell_width - 4, data->cell_height - 4, state);
        }
    }

    draw_text_color(ctx, kCOLOR_DEFAULT);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(DData *data, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    DLayout layout;
    hit_t hit = ekHIT_NONE;
    int32_t hit_day = 0;
    ctrl_state_t normal = ekCTRL_STATE_BKNORMAL;
    ctrl_state_t hot = ekCTRL_STATE_BKHOT;
    ctrl_state_t pressed = ekCTRL_STATE_BKPRESSED;
    cassert_no_null(data);
    drawctrl_clear(p->ctx, 0, 0, data->width, data->height);
    i_layout(data, &layout);

    if (data->focused == TRUE)
    {
        normal = ekCTRL_STATE_NORMAL;
        hot = ekCTRL_STATE_HOT;
        pressed = ekCTRL_STATE_PRESSED;
    }

    if (data->mouse_in == TRUE)
        hit = i_hit(data, &layout, data->mouse_x, data->mouse_y, &hit_day);

    draw_text_color(p->ctx, kCOLOR_DEFAULT);

    if (layout.header == TRUE)
        i_draw_header(data, p->ctx, &layout, hit, normal, hot);

    if (layout.wdays == TRUE)
        i_draw_wdays(data, p->ctx, &layout, normal);

    i_draw_days(data, p->ctx, &layout, hit, hit_day, normal, hot, pressed);
}

/*---------------------------------------------------------------------------*/

static void i_OnSize(DData *data, Event *e)
{
    const EvSize *p = event_params(e, EvSize);
    cassert_no_null(data);
    data->width = (uint32_t)p->width;
    data->height = (uint32_t)p->height;
    i_recolocate(data);
}

/*---------------------------------------------------------------------------*/

static void i_OnExit(DData *data, Event *e)
{
    View *view = event_sender(e, View);
    cassert_no_null(data);
    data->mouse_in = FALSE;
    view_update(view);
}

/*---------------------------------------------------------------------------*/

static void i_OnMoved(DData *data, Event *e)
{
    const EvMouse *p = event_params(e, EvMouse);
    View *view = event_sender(e, View);
    cassert_no_null(data);
    data->mouse_in = TRUE;
    data->mouse_x = p->x;
    data->mouse_y = p->y;
    view_update(view);
}

/*---------------------------------------------------------------------------*/

static void i_select_day(DData *data, DateView *view, const int32_t day)
{
    cassert_no_null(data);
    data->selected = day;
    data->focus = day;
    data->has_selected = TRUE;
    _cell_update_i32(_vctrl_cell(cast(view, View)), day);

    if (data->OnSelect != NULL)
    {
        EvDate params;
        params.date = i_to_date(day);
        listener_event(data->OnSelect, ekGUI_EVENT_DATE, view, &params, NULL, DateView, EvDate, void);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnDown(DData *data, Event *e)
{
    const EvMouse *p = event_params(e, EvMouse);
    View *view = event_sender(e, View);
    cassert_no_null(data);

    if (p->button == ekGUI_MOUSE_LEFT)
    {
        DLayout layout;
        int32_t day = 0;
        hit_t hit = ekHIT_NONE;
        i_layout(data, &layout);
        hit = i_hit(data, &layout, p->x, p->y, &day);

        switch (hit)
        {
        case ekHIT_PREV_YEAR:
            i_show_month(data, -12);
            break;
        case ekHIT_PREV_MONTH:
            i_show_month(data, -1);
            break;
        case ekHIT_NEXT_MONTH:
            i_show_month(data, 1);
            break;
        case ekHIT_NEXT_YEAR:
            i_show_month(data, 12);
            break;
        case ekHIT_DAY:
            i_select_day(data, cast(view, DateView), i_clamp_day(day));
            break;
        case ekHIT_NONE:
            break;
        default:
            cassert_default(hit);
        }

        view_update(view);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnWheel(DData *data, Event *e)
{
    const EvWheel *p = event_params(e, EvWheel);
    View *view = event_sender(e, View);
    int32_t weeks = 0;
    cassert_no_null(data);
    data->mouse_in = TRUE;
    data->mouse_x = p->x;
    data->mouse_y = p->y;
    data->wheel += p->dy;

    while (data->wheel >= 1)
    {
        weeks -= 1;
        data->wheel -= 1;
    }

    while (data->wheel <= -1)
    {
        weeks += 1;
        data->wheel += 1;
    }

    if (weeks != 0)
        i_scroll(data, weeks);

    view_update(view);
}

/*---------------------------------------------------------------------------*/

static void i_move_focus(DData *data, const int32_t days)
{
    cassert_no_null(data);
    data->focus = i_clamp_day(data->focus + days);
    i_ensure_visible(data);
}

/*---------------------------------------------------------------------------*/

static void i_OnKeyDown(DData *data, Event *e)
{
    const EvKey *p = event_params(e, EvKey);
    View *view = event_sender(e, View);
    bool_t update = TRUE;
    cassert_no_null(data);

    if (p->key == ekKEY_LEFT)
    {
        i_move_focus(data, -1);
    }
    else if (p->key == ekKEY_RIGHT)
    {
        i_move_focus(data, 1);
    }
    else if (p->key == ekKEY_UP)
    {
        i_move_focus(data, -7);
    }
    else if (p->key == ekKEY_DOWN)
    {
        i_move_focus(data, 7);
    }
    else if (p->key == ekKEY_HOME)
    {
        int32_t start = i_week_start(data, data->focus);
        i_move_focus(data, start - data->focus);
    }
    else if (p->key == ekKEY_END)
    {
        int32_t start = i_week_start(data, data->focus);
        i_move_focus(data, start + 6 - data->focus);
    }
    else if (p->key == ekKEY_PAGEUP)
    {
        if (p->modifiers & ekMKEY_SHIFT)
            i_navigate(data, -12);
        else
            i_navigate(data, -1);
    }
    else if (p->key == ekKEY_PAGEDOWN)
    {
        if (p->modifiers & ekMKEY_SHIFT)
            i_navigate(data, 12);
        else
            i_navigate(data, 1);
    }
    else
    {
        update = FALSE;
    }

    if (update == TRUE)
    {
        if (data->has_selected == FALSE || data->focus != data->selected)
            i_select_day(data, cast(view, DateView), data->focus);
        view_update(view);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnFocus(DData *data, Event *e)
{
    const bool_t *p = event_params(e, bool_t);
    View *view = event_sender(e, View);
    cassert_no_null(data);
    data->focused = *p;

    if (data->focused == TRUE)
    {
        if (data->has_selected == TRUE)
            data->focus = data->selected;
        else
            data->focus = data->today;
    }

    view_update(view);
}

/*---------------------------------------------------------------------------*/

static void i_natural(DateView *view, const uint32_t di, real32_t *dim0, real32_t *dim1)
{
    DData *data = view_get_data(cast(view, View), DData);
    cassert_no_null(data);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (di == 0)
    {
        *dim0 = (real32_t)(7 * data->cell_width + 2 * i_MARGIN);
    }
    else
    {
        cassert(di == 1);
        *dim1 = (real32_t)(data->header_height + data->wdays_height + 6 * data->cell_height + 2 * i_MARGIN);
    }
}

/*---------------------------------------------------------------------------*/

static void i_fonts(DData *data, const Font *font)
{
    uint32_t style = font_style(font) & ~(uint32_t)(ekFBOLD | ekFITALIC | ekFSTRIKEOUT | ekFUNDERLINE | ekFSUBSCRIPT | ekFSUPSCRIPT);
    cassert_no_null(data);
    ptr_destopt(font_destroy, &data->font, Font);
    ptr_destopt(font_destroy, &data->bfont, Font);
    data->font = font_with_style(font, style);
    data->bfont = font_with_style(font, style | ekFBOLD);
}

/*---------------------------------------------------------------------------*/

static void i_set_empty(DateView *view)
{
    DData *data = view_get_data(cast(view, View), DData);
    cassert_no_null(data);
    data->has_selected = FALSE;
    view_update(cast(view, View));
}

/*---------------------------------------------------------------------------*/

static void i_set_int32(DateView *view, const int32_t value)
{
    DData *data = view_get_data(cast(view, View), DData);
    cassert_no_null(data);
    if (value == INT32_MIN)
    {
        data->has_selected = FALSE;
    }
    else
    {
        data->selected = i_clamp_day(value);
        data->focus = data->selected;
        data->has_selected = TRUE;
        i_recolocate(data);
    }

    view_update(cast(view, View));
}

/*---------------------------------------------------------------------------*/

static DData *i_create_data(void)
{
    DData *data = heap_new0(DData);
    Font *font = drawctrl_font(NULL);
    Date today = date_system();
    i_fonts(data, font);
    font_destroy(&font);
    i_locale_months(data);
    i_locale_wdays(data);
    data->first_wday = btime_first_wday();
    data->today = i_from_date(&today);
    data->focus = data->today;
    i_metrics(data);
    data->width = 7 * data->cell_width + 2 * i_MARGIN;
    data->height = data->header_height + data->wdays_height + 6 * data->cell_height + 2 * i_MARGIN;
    i_recolocate(data);
    return data;
}

/*---------------------------------------------------------------------------*/

DateView *dateview_create(void)
{
    DData *data = i_create_data();
    View *view = _vctrl_create(ekVIEW_BORDER | ekVIEW_CONTROL | ekVIEW_NOERASE, &i_DATEVIEW_TLB, data, DData);
    return cast(view, DateView);
}

/*---------------------------------------------------------------------------*/

void dateview_OnSelect(DateView *view, Listener *listener)
{
    DData *data = view_get_data(cast(view, View), DData);
    cassert_no_null(data);
    listener_update(&data->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void dateview_font(DateView *view, const Font *font)
{
    DData *data = view_get_data(cast(view, View), DData);
    cassert_no_null(data);
    i_fonts(data, font);
    i_metrics(data);
    i_recolocate(data);
    view_update(cast(view, View));
}

/*---------------------------------------------------------------------------*/

void dateview_first_wday(DateView *view, const week_day_t wday)
{
    DData *data = view_get_data(cast(view, View), DData);
    cassert_no_null(data);
    cassert((uint32_t)wday <= 6);
    data->first_wday = wday;
    i_recolocate(data);
    view_update(cast(view, View));
}

/*---------------------------------------------------------------------------*/

void dateview_month_names(DateView *view, const char_t **names)
{
    DData *data = view_get_data(cast(view, View), DData);
    cassert_no_null(data);
    if (names != NULL)
    {
        uint32_t i;
        for (i = 0; i < 12; ++i)
        {
            str_destopt(&data->months[i]);
            data->months[i] = str_c(names[i]);
        }
    }
    else
    {
        i_locale_months(data);
    }

    i_metrics(data);
    i_recolocate(data);
    view_update(cast(view, View));
}

/*---------------------------------------------------------------------------*/

void dateview_wday_names(DateView *view, const char_t **names)
{
    DData *data = view_get_data(cast(view, View), DData);
    cassert_no_null(data);
    if (names != NULL)
    {
        uint32_t i;
        for (i = 0; i < 7; ++i)
        {
            str_destopt(&data->wdays[i]);
            data->wdays[i] = str_c(names[i]);
        }
    }
    else
    {
        i_locale_wdays(data);
    }

    i_metrics(data);
    i_recolocate(data);
    view_update(cast(view, View));
}

/*---------------------------------------------------------------------------*/

void dateview_now(DateView *view, const Date *date)
{
    DData *data = view_get_data(cast(view, View), DData);
    cassert_no_null(data);
    data->today = i_clamp_day(i_from_date(date));

    if (data->has_selected == FALSE)
    {
        data->focus = data->today;
        i_recolocate(data);
    }

    view_update(cast(view, View));
}

/*---------------------------------------------------------------------------*/

void dateview_select(DateView *view, const Date *date)
{
    DData *data = view_get_data(cast(view, View), DData);
    cassert_no_null(data);
    if (date_is_null(date) == TRUE)
    {
        data->has_selected = FALSE;
    }
    else
    {
        data->selected = i_clamp_day(i_from_date(date));
        data->focus = data->selected;
        data->has_selected = TRUE;
        i_recolocate(data);
    }

    view_update(cast(view, View));
}

/*---------------------------------------------------------------------------*/

Date dateview_selected(const DateView *view)
{
    const DData *data = view_get_data(cast(view, View), DData);
    cassert_no_null(data);
    if (data->has_selected == TRUE)
        return i_to_date(data->selected);
    else
        return kDATE_NULL;
}
