/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: prmodel.c
 *
 */

/* Products Model */

#include "prmodel.h"
#include "guiall.h"
#include "httpreq.h"
#include "json.h"
#include "res_products.h"

typedef struct _pjson_t PJson;

typedef enum _type_t
{
    ekCPU,
    ekGPU,
    ekHDD,
    ekSCD
} type_t;

struct _product_t
{
    type_t type;
    String *code;
    String *description;
    Image *image64;
    real32_t price;
};

struct _pjson_t
{
    int32_t code;
    uint32_t size;
    ArrPt(Product) *data;
};

struct _model_t
{
    ArrSt(uint32_t) *filter;
    ArrPt(Product) *products;
};

DeclPt(Product);

/*---------------------------------------------------------------------------*/

Model *model_create(void)
{
    Model *model = heap_new(Model);
    model->filter = arrst_create(uint32_t);
    model->products = arrpt_create(Product);
    return model;
}

/*---------------------------------------------------------------------------*/

void model_destroy(Model **model)
{
    arrst_destroy(&(*model)->filter, NULL, uint32_t);
    dbind_destroy(&(*model)->products, ArrPt(Product));
    heap_delete(model, Model);
}

/*---------------------------------------------------------------------------*/

static Stream *i_http_get(void)
{
    Http *http = http_create("serv.nappgui.com", 80);
    Stream *stm = NULL;

    if (http_get(http, "/dproducts.php", NULL, 0, NULL) == TRUE)
    {
        uint32_t status = http_response_status(http);
        if (status >= 200 && status <= 299)
        {
            stm = stm_memory(4096);
            if (http_response_body(http, stm, NULL) == FALSE)
                stm_close(&stm);
        }
    }

    http_destroy(&http);
    return stm;
}

/*---------------------------------------------------------------------------*/

wserv_t model_webserv(Model *model)
{
    Stream *stm = i_http_get();
    if (stm != NULL)
    {
        PJson *json = json_read(stm, NULL, PJson);
        stm_close(&stm);

        if (json != NULL)
        {
            cassert(json->size == arrpt_size(json->data, Product));
            dbind_destroy(&model->products, ArrPt(Product));
            model->products = json->data;
            json->data = NULL;
            json_destroy(&json, PJson);
            return ekWS_OK;
        }

        return ekWS_JSON;
    }

    return ekWS_CONNECT;
}

/*---------------------------------------------------------------------------*/

bool_t model_import(Model *model, const char_t *pathname, ferror_t *err)
{
    Stream *stm = stm_from_file(pathname, err);
    if (stm != NULL)
    {
        ArrPt(Product) *products = dbind_read(stm, ArrPt(Product));
        stm_close(&stm);

        if (products != NULL)
        {
            dbind_destroy(&model->products, ArrPt(Product));
            model->products = products;
            return TRUE;
        }
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

bool_t model_export(Model *model, const char_t *pathname, ferror_t *err)
{
    Stream *stm = stm_to_file(pathname, err);
    if (stm != NULL)
    {
        dbind_write(stm, model->products, ArrPt(Product));
        stm_close(&stm);
        return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

uint32_t model_count(const Model *model)
{
    uint32_t total = arrst_size(model->filter, uint32_t);
    if (total == 0)
        total = arrpt_size(model->products, Product);
    return total;
}

/*---------------------------------------------------------------------------*/

void model_clear(Model *model)
{
    dbind_destroy(&model->products, ArrPt(Product));
    arrst_clear(model->filter, NULL, uint32_t);
    model->products = dbind_create(ArrPt(Product));
}

/*---------------------------------------------------------------------------*/

void model_add(Model *model)
{
    Product *product = dbind_create(Product);
    arrpt_append(model->products, product, Product);
    arrst_clear(model->filter, NULL, uint32_t);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_index(ArrSt(uint32_t) *filter, const uint32_t index)
{
    if (arrst_size(filter, uint32_t) > 0)
        return *arrst_get(filter, index, uint32_t);
    else
        return index;
}

/*---------------------------------------------------------------------------*/

static __INLINE void i_destroy(Product **product)
{
    dbind_destroy(product, Product);
}

/*---------------------------------------------------------------------------*/

void model_delete(Model *model, const uint32_t index)
{
    uint32_t lindex = i_index(model->filter, index);
    arrpt_delete(model->products, lindex, i_destroy, Product);
    arrst_clear(model->filter, NULL, uint32_t);
}

/*---------------------------------------------------------------------------*/

bool_t model_filter(Model *model, const char_t *filter)
{
    ArrSt(uint32_t) *new_filter = arrst_create(uint32_t);

    arrpt_foreach(product, model->products, Product)
        if (str_str(tc(product->description), filter) != NULL)
            arrst_append(new_filter, product_i, uint32_t);
    arrpt_end();

    arrst_destroy(&model->filter, NULL, uint32_t);
    model->filter = new_filter;

    return (bool_t)(arrst_size(new_filter, uint32_t) > 0);
}

/*---------------------------------------------------------------------------*/

Product *model_product(Model *model, const uint32_t product_id)
{
    uint32_t lindex = i_index(model->filter, product_id);
    return arrpt_get(model->products, lindex, Product);
}

/*---------------------------------------------------------------------------*/

void model_bind(void)
{
    dbind_enum(type_t, ekCPU, "");
    dbind_enum(type_t, ekGPU, "");
    dbind_enum(type_t, ekHDD, "");
    dbind_enum(type_t, ekSCD, "");
    dbind(Product, type_t, type);
    dbind(Product, String*, code);
    dbind(Product, String*, description);
    dbind(Product, Image*, image64);
    dbind(Product, real32_t, price);
    dbind(PJson, int32_t, code);
    dbind(PJson, uint32_t, size);
    dbind(PJson, ArrPt(Product)*, data);
    dbind_default(Product, real32_t, price, 1);
    dbind_range(Product, real32_t, price, .50f, 1e6f);
    dbind_precision(Product, real32_t, price, .05f);
    dbind_increment(Product, real32_t, price, 5.f);
    dbind_suffix(Product, real32_t, price, "€");
    dbind_default(Product, Image*, image64, gui_image(NOIMAGE_PNG));
}

/*---------------------------------------------------------------------------*/

void model_layout(Layout *layout)
{
    layout_dbind(layout, NULL, Product);
}

/*---------------------------------------------------------------------------*/

void model_type(Cell *cell)
{
    cell_dbind(cell, Product, type_t, type);
}

/*---------------------------------------------------------------------------*/

void model_code(Cell *cell)
{
    cell_dbind(cell, Product, String*, code);
}

/*---------------------------------------------------------------------------*/

void model_desc(Cell *cell)
{
    cell_dbind(cell, Product, String*, description);
}

/*---------------------------------------------------------------------------*/

void model_image(Cell *cell)
{
    cell_dbind(cell, Product, Image*, image64);
}

/*---------------------------------------------------------------------------*/

void model_price(Cell *cell)
{
    cell_dbind(cell, Product, real32_t, price);
}
