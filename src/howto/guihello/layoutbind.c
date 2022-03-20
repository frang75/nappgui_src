/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: layoutbind.c
 *
 */

/* GUI data binding */

#include "guiall.h"
#include "layoutbind.h"

typedef struct _vector_t Vector;
typedef struct _structtypes_t StructTypes;

struct _vector_t
{
    real32_t x;
    real32_t y;
    real32_t z;
};

struct _structtypes_t
{
    String *name;
    Vector vec1;
    Vector vec2;
    Vector vec3;
    Vector *pvec1;
    Vector *pvec2;
    Vector *pvec3;
    real32_t length1;
    real32_t length2;
    real32_t length3;
    real32_t length4;
    real32_t length5;
    real32_t length6;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_data(StructTypes **data)
{
    str_destroy(&(*data)->name);
    heap_delete(&(*data)->pvec1, Vector);
    heap_delete(&(*data)->pvec2, Vector);
    heap_delete(&(*data)->pvec3, Vector);
    heap_delete(data, StructTypes);
}

/*---------------------------------------------------------------------------*/

static Vector i_vec_init(const real32_t x, const real32_t y, const real32_t z)
{
    Vector v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

/*---------------------------------------------------------------------------*/

static real32_t i_vec_length(const Vector *vec)
{
    real32_t n = vec->x * vec->x + vec->y * vec->y + vec->z * vec->z;
    return bmath_sqrtf(n);
}

/*---------------------------------------------------------------------------*/

static void i_OnDataChange(void *non_used, Event *e)
{
    StructTypes *data = evbind_object(e, StructTypes);
    Layout *layout = event_sender(e, Layout);
    unref(non_used);

    if (evbind_modify(e, StructTypes, Vector, vec1) == TRUE)
    {
        data->length1 = i_vec_length(&data->vec1);
        layout_dbind_update(layout, StructTypes, real32_t, length1);
    }
    else if (evbind_modify(e, StructTypes, Vector, vec2) == TRUE)
    {
        data->length2 = i_vec_length(&data->vec2);
        layout_dbind_update(layout, StructTypes, real32_t, length2);
    }
    else if (evbind_modify(e, StructTypes, Vector, vec3) == TRUE)
    {
        data->length3 = i_vec_length(&data->vec3);
        layout_dbind_update(layout, StructTypes, real32_t, length3);
    }
    else if (evbind_modify(e, StructTypes, Vector*, pvec1) == TRUE)
    {
        data->length4 = i_vec_length(data->pvec1);
        layout_dbind_update(layout, StructTypes, real32_t, length4);
    }
    else if (evbind_modify(e, StructTypes, Vector*, pvec2) == TRUE)
    {
        data->length5 = i_vec_length(data->pvec2);
        layout_dbind_update(layout, StructTypes, real32_t, length5);
    }
    else if (evbind_modify(e, StructTypes, Vector*, pvec3) == TRUE)
    {
        data->length6 = i_vec_length(data->pvec3);
        layout_dbind_update(layout, StructTypes, real32_t, length6);
    }
}

/*---------------------------------------------------------------------------*/

static Layout *i_vector_layout(void)
{
    Layout *layout = layout_create(3, 3);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_create();
    Edit *edit3 = edit_create();
    UpDown *updown1 = updown_create();
    UpDown *updown2 = updown_create();
    UpDown *updown3 = updown_create();
    label_text(label1, "X:");
    label_text(label2, "Y:");
    label_text(label3, "Z:");
    edit_align(edit1, ekRIGHT);
    edit_align(edit2, ekRIGHT);
    edit_align(edit3, ekRIGHT);
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 0, 1);
    layout_label(layout, label3, 0, 2);
    layout_edit(layout, edit1, 1, 0);
    layout_edit(layout, edit2, 1, 1);
    layout_edit(layout, edit3, 1, 2);
    layout_updown(layout, updown1, 2, 0);
    layout_updown(layout, updown2, 2, 1);
    layout_updown(layout, updown3, 2, 2);
    layout_hmargin(layout, 0, 5);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    layout_hsize(layout, 1, 60);
    cell_dbind(layout_cell(layout, 1, 0), Vector, real32_t, x);
    cell_dbind(layout_cell(layout, 1, 1), Vector, real32_t, y);
    cell_dbind(layout_cell(layout, 1, 2), Vector, real32_t, z);
    cell_dbind(layout_cell(layout, 2, 0), Vector, real32_t, x);
    cell_dbind(layout_cell(layout, 2, 1), Vector, real32_t, y);
    cell_dbind(layout_cell(layout, 2, 2), Vector, real32_t, z);
    layout_dbind(layout, NULL, Vector);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_name_layout(void)
{
    Layout *layout = layout_create(2, 1);
    Label *label = label_create();
    Edit *edit = edit_create();
    label_text(label, "Object Name:");
    layout_hexpand(layout, 1);
    layout_label(layout, label, 0, 0);
    layout_edit(layout, edit, 1, 0);
    layout_hmargin(layout, 0, 10);
    cell_dbind(layout_cell(layout, 1, 0), StructTypes, String*, name);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_vectors_layout(void)
{
    Layout *layout1 = layout_create(3, 4);
    Layout *layout2 = i_vector_layout();
    Layout *layout3 = i_vector_layout();
    Layout *layout4 = i_vector_layout();
    Layout *layout5 = i_vector_layout();
    Layout *layout6 = i_vector_layout();
    Layout *layout7 = i_vector_layout();
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    Label *label6 = label_create();
    label_text(label1, "vec1");
    label_text(label2, "vec2");
    label_text(label3, "vec3");
    label_text(label4, "*pvec1");
    label_text(label5, "*pvec2");
    label_text(label6, "*pvec3");
    layout_label(layout1, label1, 0, 0);
    layout_label(layout1, label2, 1, 0);
    layout_label(layout1, label3, 2, 0);
    layout_label(layout1, label4, 0, 2);
    layout_label(layout1, label5, 1, 2);
    layout_label(layout1, label6, 2, 2);
    layout_layout(layout1, layout2, 0, 1);
    layout_layout(layout1, layout3, 1, 1);
    layout_layout(layout1, layout4, 2, 1);
    layout_layout(layout1, layout5, 0, 3);
    layout_layout(layout1, layout6, 1, 3);
    layout_layout(layout1, layout7, 2, 3);
    layout_halign(layout1, 0, 0, ekCENTER);
    layout_halign(layout1, 1, 0, ekCENTER);
    layout_halign(layout1, 2, 0, ekCENTER);
    layout_halign(layout1, 0, 2, ekCENTER);
    layout_halign(layout1, 1, 2, ekCENTER);
    layout_halign(layout1, 2, 2, ekCENTER);
    layout_hmargin(layout1, 0, 10);
    layout_hmargin(layout1, 1, 10);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 10);
    layout_vmargin(layout1, 2, 5);
    cell_dbind(layout_cell(layout1, 0, 1), StructTypes, Vector, vec1);
    cell_dbind(layout_cell(layout1, 1, 1), StructTypes, Vector, vec2);
    cell_dbind(layout_cell(layout1, 2, 1), StructTypes, Vector, vec3);
    cell_dbind(layout_cell(layout1, 0, 3), StructTypes, Vector*, pvec1);
    cell_dbind(layout_cell(layout1, 1, 3), StructTypes, Vector*, pvec2);
    cell_dbind(layout_cell(layout1, 2, 3), StructTypes, Vector*, pvec3);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static Layout *i_lengths_layout(void)
{
    Layout *layout = layout_create(2, 6);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    Label *label6 = label_create();
    Label *label7 = label_create();
    Label *label8 = label_create();
    Label *label9 = label_create();
    Label *label10 = label_create();
    Label *label11 = label_create();
    Label *label12 = label_create();
    label_text(label1, "length1:");
    label_text(label2, "length2:");
    label_text(label3, "length3:");
    label_text(label4, "length4:");
    label_text(label5, "length5:");
    label_text(label6, "length6:");
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 0, 1);
    layout_label(layout, label3, 0, 2);
    layout_label(layout, label4, 0, 3);
    layout_label(layout, label5, 0, 4);
    layout_label(layout, label6, 0, 5);
    layout_label(layout, label7, 1, 0);
    layout_label(layout, label8, 1, 1);
    layout_label(layout, label9, 1, 2);
    layout_label(layout, label10, 1, 3);
    layout_label(layout, label11, 1, 4);
    layout_label(layout, label12, 1, 5);
    label_align(label7, ekRIGHT);
    label_align(label8, ekRIGHT);
    label_align(label9, ekRIGHT);
    label_align(label10, ekRIGHT);
    label_align(label11, ekRIGHT);
    label_align(label12, ekRIGHT);
    layout_hsize(layout, 1, 40);
    layout_hmargin(layout, 0, 5);
    layout_halign(layout, 1, 0, ekJUSTIFY);
    layout_halign(layout, 1, 1, ekJUSTIFY);
    layout_halign(layout, 1, 2, ekJUSTIFY);
    layout_halign(layout, 1, 3, ekJUSTIFY);
    layout_halign(layout, 1, 4, ekJUSTIFY);
    layout_halign(layout, 1, 5, ekJUSTIFY);
    cell_dbind(layout_cell(layout, 1, 0), StructTypes, real32_t, length1);
    cell_dbind(layout_cell(layout, 1, 1), StructTypes, real32_t, length2);
    cell_dbind(layout_cell(layout, 1, 2), StructTypes, real32_t, length3);
    cell_dbind(layout_cell(layout, 1, 3), StructTypes, real32_t, length4);
    cell_dbind(layout_cell(layout, 1, 4), StructTypes, real32_t, length5);
    cell_dbind(layout_cell(layout, 1, 5), StructTypes, real32_t, length6);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(void)
{
    Layout *layout1 = layout_create(2, 2);
    Layout *layout2 = i_name_layout();
    Layout *layout3 = i_vectors_layout();
    Layout *layout4 = i_lengths_layout();
    layout_layout(layout1, layout2, 0, 0);
    layout_layout(layout1, layout3, 0, 1);
    layout_layout(layout1, layout4, 1, 1);
    layout_hmargin(layout1, 0, 10);
    layout_vmargin(layout1, 0, 10);
    return layout1;
}

/*---------------------------------------------------------------------------*/

Panel* layoutbind(void)
{
    Layout *layout = NULL;
    Panel *panel = NULL;
    StructTypes *data = heap_new(StructTypes);
    data->name = str_c("Generic Object");
    data->pvec1 = heap_new(Vector);
    data->pvec2 = heap_new(Vector);
    data->pvec3 = heap_new(Vector);
    data->vec1 = i_vec_init(1.2f, 2.1f, -3.4f);
    data->vec2 = i_vec_init(-0.2f, 1.8f, 2.3f);
    data->vec3 = i_vec_init(-3.2f, 4.9f, -4.7f);
    *data->pvec1 = i_vec_init(0.9f, 7.9f, -2.0f);
    *data->pvec2 = i_vec_init(-6.9f, 2.2f, 8.6f);
    *data->pvec3 = i_vec_init(3.9f, -5.5f, 0.3f);
    data->length1 = i_vec_length(&data->vec1);
    data->length2 = i_vec_length(&data->vec2);
    data->length3 = i_vec_length(&data->vec3);
    data->length4 = i_vec_length(data->pvec1);
    data->length5 = i_vec_length(data->pvec2);
    data->length6 = i_vec_length(data->pvec3);

    dbind(Vector, real32_t, x);
    dbind(Vector, real32_t, y);
    dbind(Vector, real32_t, z);
    dbind(StructTypes, String*, name);
    dbind(StructTypes, Vector, vec1);
    dbind(StructTypes, Vector, vec2);
    dbind(StructTypes, Vector, vec3);
    dbind(StructTypes, Vector*, pvec1);
    dbind(StructTypes, Vector*, pvec2);
    dbind(StructTypes, Vector*, pvec3);
    dbind(StructTypes, real32_t, length1);
    dbind(StructTypes, real32_t, length2);
    dbind(StructTypes, real32_t, length3);
    dbind(StructTypes, real32_t, length4);
    dbind(StructTypes, real32_t, length5);
    dbind(StructTypes, real32_t, length6);
    dbind_range(Vector, real32_t, x, -5, 5);
    dbind_range(Vector, real32_t, y, -5, 5);
    dbind_range(Vector, real32_t, z, -5, 5);
    dbind_increment(Vector, real32_t, x, .1f);
    dbind_increment(Vector, real32_t, y, .1f);
    dbind_increment(Vector, real32_t, z, .1f);

    layout = i_layout();
    panel = panel_create();
    layout_dbind(layout, listener(NULL, i_OnDataChange, void), StructTypes);
    layout_dbind_obj(layout, data, StructTypes);
    panel_data(panel, &data, i_destroy_data, StructTypes);
    panel_layout(panel, layout);
    return panel;
}
