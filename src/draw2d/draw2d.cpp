/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw2d.cpp
 *
 */

/* Operating system 2D drawing support */

#include "draw2d.h"
#include "draw2d.inl"
#include "dbind.inl"
#include "draw.inl"
#include "font.inl"
#include "image.inl"
#include "respack.inl"
#include "image.h"

#include "arrpt.h"
#include "arrst.h"
#include "bmem.h"
#include "cassert.h"
#include "core.h"
#include "font.h"
#include "heap.h"
#include "log.h"
#include "strings.h"

#define i_WORD_TYPE_END         0
#define i_WORD_TYPE_NEW_LINE    1
#define i_WORD_TYPE_BLANCKS     2
#define i_WORD_TYPE_TEXT        3

/*---------------------------------------------------------------------------*/

class Draw2D
{
public:
    static uint32_t NUM_USERS;
    ArrPt(String) *font_families;
    ArrSt(color_t) *named_colors;

    ~Draw2D()
    {
        if (NUM_USERS != 0)
        {
            log_printf("Error! osdraw is not properly closed (%d)\n", NUM_USERS);
        }
    }
};

static Draw2D i_DRAW2D;
uint32_t Draw2D::NUM_USERS = 0;

/*---------------------------------------------------------------------------*/

void draw2d_start(void)
{
    if (i_DRAW2D.NUM_USERS == 0)
    {
        core_start();
        osimage_alloc_globals();
        osfont_alloc_globals();
        drawimp_alloc_globals();

        i_DRAW2D.font_families = arrpt_create(String);
        
        {
            String *str = str_c("__SYSTEM__");
            arrpt_append(i_DRAW2D.font_families, str, String);
        }

        {
            String *str = str_c("__MONOSPACE__");
            arrpt_append(i_DRAW2D.font_families, str, String);
        }

        i_DRAW2D.named_colors = arrst_create(color_t);
        dbind_opaque(Image, image_from_data, NULL, image_copy, NULL, image_write, image_destroy);
    }

    i_DRAW2D.NUM_USERS += 1;
}

/*---------------------------------------------------------------------------*/

void draw2d_finish(void)
{
    cassert(i_DRAW2D.NUM_USERS > 0);
    i_DRAW2D.NUM_USERS -= 1;
    if (i_DRAW2D.NUM_USERS == 0)
    {
        _dbind_finish(); // Destroy possible images
        arrpt_destroy(&i_DRAW2D.font_families, str_destroy, String);
        arrst_destroy(&i_DRAW2D.named_colors, NULL, color_t);
        osfont_dealloc_globals();
        osimage_dealloc_globals();
        drawimp_dealloc_globals();
        core_finish();
    }
}

/*---------------------------------------------------------------------------*/

uint32_t draw2d_register_font(const char_t *font_family)
{
    arrpt_foreach(family, i_DRAW2D.font_families, String)
        if (str_cmp(family, font_family) == 0)
            return family_i;
    arrpt_end();

    if (font_exists_family(font_family) == TRUE)
    {
        uint32_t num_elems = 0;
        String *family = NULL;
        num_elems = arrpt_size(i_DRAW2D.font_families, String);
        family = str_c(font_family);
        arrpt_append(i_DRAW2D.font_families, family, String);
        return num_elems;
    }
    else
    {
        return (uint32_t)ekFONT_FAMILY_SYSTEM;
    }
}

/*---------------------------------------------------------------------------*/

const char_t *draw2d_font_family(const uint32_t family)
{
    const String *font_family = arrpt_get(i_DRAW2D.font_families, family, String);
    return osfont_family(tc(font_family));
}

/*---------------------------------------------------------------------------*/

void draw2d_set_named_color(const uint32_t index, const color_t color)
{
    uint32_t n = arrst_size(i_DRAW2D.named_colors, color_t);
    color_t *col = NULL;
    if (index >= n)
    {
        uint32_t i;
        for (i = n; i <= index; ++i)
            arrst_append(i_DRAW2D.named_colors, 0, color_t);
    }

    col = arrst_get(i_DRAW2D.named_colors, index, color_t);
    *col = color;
}

/*---------------------------------------------------------------------------*/

color_t draw2d_get_named_color(const uint32_t index)
{
    // Named color should be initialized
    color_t *col = arrst_get(i_DRAW2D.named_colors, index, color_t);
    cassert(index == 0 || *col != 0);
    return *col;
}

/*---------------------------------------------------------------------------*/

static __INLINE real32_t i_ceil(const real32_t n) 
{
    int32_t in = (int32_t)n;
    if (n == (real32_t)in) 
    {
        return (real32_t)in;
    }

    return (real32_t)(in + 1);
}

/*---------------------------------------------------------------------------*/

static const char_t *i_jump_blanks(const char_t *str)
{
    cassert_no_null(str);
    for (;*str != '\0';)
    {
        if (*str == ' ' || *str == '\t' || *str == '\r')
        {
            str += 1;
        }
        else
        {
            return str;
        }
    }

    return str;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_jump_not_blanks(const char_t *str)
{
    cassert_no_null(str);
    for (;*str != '\0';)
    {
        if (*str != ' ' && *str != '\t' && *str != '\r' && *str != '\0' && *str != '\n')
        {
            str += 1;
        }
        else
        {
            return str;
        }
    }

    return str;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_next_word(const char_t *str, int *word_type)
{
    cassert_no_null(str);
    cassert_no_null(word_type);
    if (*str == '\0')
    {
        *word_type = i_WORD_TYPE_END;
        return NULL;
    }
    else if (*str == '\n')
    {
        *word_type = i_WORD_TYPE_NEW_LINE;
        return str + 1;
    }

    {
        register const char_t *end = i_jump_blanks(str);
        if (end != str)
        {
            *word_type = i_WORD_TYPE_BLANCKS;
            return end;
        }
    }

    {
        register const char_t *end = i_jump_not_blanks(str);
        cassert(end != str);
        *word_type = i_WORD_TYPE_TEXT;
        return end;
    }
}

/*---------------------------------------------------------------------------*/

static void i_new_line(void *data, FPtr_word_extents func_word_extents, real32_t *current_width, real32_t *current_height, real32_t *current_width_without_spaces, uint32_t *num_lines, real32_t *width, real32_t *height)
{
    cassert_no_null(current_width);
    cassert_no_null(current_height);
    cassert_no_null(current_width_without_spaces);
    cassert_no_null(num_lines);
    cassert_no_null(width);
    cassert_no_null(height);

    if (*current_width_without_spaces == 0)
    {
        real32_t word_width = 0, word_height = 0;
        func_word_extents(data, "A", &word_width, &word_height);
        *current_width_without_spaces = word_width;
        *current_height = word_height;
    }
    else
    {
        cassert(*current_height > 0);
    }

    if (*current_width_without_spaces > *width)
        *width = *current_width_without_spaces;
    *height += *current_height;

    *current_width = 0;
    *current_width_without_spaces = 0;
    *current_height = 0;
    *num_lines += 1;
}

/*---------------------------------------------------------------------------*/

void draw2d_extents_imp(void *data, FPtr_word_extents func_word_extents, const bool_t newlines, const char_t *str, const real32_t refwidth, real32_t *width, real32_t *height)
{
    uint32_t num_lines = 0;
    real32_t ref_width = refwidth > 0 ? refwidth : 1e8f;
    real32_t current_width = 0, current_height = 0, current_width_without_spaces = 0;
    register const char_t *ctext = str;

    cassert_no_nullf(func_word_extents);
    cassert_no_null(width);
    cassert_no_null(height);
    *width = 0;
    *height = 0;

    while (ctext != NULL)
    {
        const char_t *next_text = NULL;
        int word_type = 0;

        next_text = i_next_word(ctext, &word_type);

        switch (word_type) {

        case i_WORD_TYPE_END:
            if (current_width > .01f || num_lines == 0)
                i_new_line(data, func_word_extents, &current_width, &current_height, &current_width_without_spaces, &num_lines, width, height);
            break;

        case i_WORD_TYPE_NEW_LINE:
            if (newlines == TRUE)
                i_new_line(data, func_word_extents, &current_width, &current_height, &current_width_without_spaces, &num_lines, width, height);
            break;

        case i_WORD_TYPE_BLANCKS:
            if (current_width_without_spaces > 0)
            {
                char_t word[256];
                real32_t word_width = 0, word_height = 0;
                register uint32_t size = (uint32_t)(next_text - ctext);
                cassert(next_text > ctext);
                str_copy_cn(word, sizeof(word), ctext, size);
                word[size] = '\0';
                func_word_extents(data, word, &word_width, &word_height);                
                if (current_width + word_width <= ref_width)
                {
                    current_width += word_width;
                    if (word_height > current_height)
                        current_height = word_height;
                }
                else
                {
                    i_new_line(data, func_word_extents, &current_width, &current_height, &current_width_without_spaces, &num_lines, width, height);
                }
            }
            break;

        case i_WORD_TYPE_TEXT:
        {
            char_t word[256];
            real32_t word_width = 0.f, word_height = 0.f;
            register uint32_t size = (uint32_t)(next_text - ctext);
            cassert(next_text > ctext);
            cassert(word_type == i_WORD_TYPE_TEXT);
            str_copy_cn(word, sizeof(word), ctext, size);
            word[size] = '\0';
            func_word_extents(data, word, &word_width, &word_height);                

            if (current_width + word_width <= ref_width)
            {
                current_width += word_width;
                if (word_height > current_height)
                    current_height = word_height;
            }
            else
            {
                i_new_line(data, func_word_extents, &current_width, &current_height, &current_width_without_spaces, &num_lines, width, height);
                current_width = word_width;
                current_height = word_height;
            }

            current_width_without_spaces = current_width;
            break;
        }

        cassert_default();
        }

        ctext = next_text;
    }

    *width = i_ceil(*width);
    *height = i_ceil(*height);
}
