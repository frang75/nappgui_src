/* JSON parsing examples */

#include "res_htjson.h"
#include <draw2d/draw2dall.h>
#include <inet/json.h>

/*---------------------------------------------------------------------------*/

/* C structs that map a Json object */
typedef struct _product_t Product;
typedef struct _products_t Products;

struct _product_t
{
    String *description;
    real32_t price;
};

struct _products_t
{
    uint32_t size;
    ArrSt(Product) *data;
};

DeclSt(Product);

/*---------------------------------------------------------------------------*/

static Stream *i_stm_from_json(const char_t *json_data)
{
    return stm_from_block(cast_const(json_data, byte_t), str_len_c(json_data));
}

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    unref(argc);
    unref(argv);
    draw2d_start();

    /* Parsing a Json boolean */
    {
        Stream *stm = i_stm_from_json("true");
        bool_t *json = json_read(stm, NULL, bool_t);
        bstd_printf("bool_t from Json: %d\n", *json);
        json_destroy(&json, bool_t);
        stm_close(&stm);
    }

    /* Parsing a Json unsigned int */
    {
        Stream *stm = i_stm_from_json("6654");
        uint16_t *json = json_read(stm, NULL, uint16_t);
        bstd_printf("uint16_t from Json: %d\n", *json);
        json_destroy(&json, uint16_t);
        stm_close(&stm);
    }

    /* Parsing a Json signed int */
    {
        Stream *stm = i_stm_from_json("-567");
        int16_t *json = json_read(stm, NULL, int16_t);
        bstd_printf("int16_t from Json: %d\n", *json);
        json_destroy(&json, int16_t);
        stm_close(&stm);
    }

    /* Parsing a Json real */
    {
        Stream *stm = i_stm_from_json("456.45");
        real32_t *json = json_read(stm, NULL, real32_t);
        bstd_printf("real32_t from Json: %.3f\n", *json);
        json_destroy(&json, real32_t);
        stm_close(&stm);
    }

    /* Parsing a Json string */
    {
        Stream *stm = i_stm_from_json("\"Hello World\"");
        String *json = json_read(stm, NULL, String);
        bstd_printf("String from Json: %s\n", tc(json));
        json_destroy(&json, String);
        stm_close(&stm);
    }

    /* Parsing a Json b64 encoded image */
    {
        uint32_t size;
        ResPack *pack = res_htjson_respack("");
        const byte_t *data = respack_file(pack, JSON_B64_IMAGE_TXT, &size);
        Stream *stm = stm_from_block(data, size);
        Image *json = json_read(stm, NULL, Image);
        uint32_t width = image_width(json);
        uint32_t height = image_height(json);
        bstd_printf("Image from Json: width: %d height: %d\n", width, height);
        json_destroy(&json, Image);
        stm_close(&stm);
        respack_destroy(&pack);
    }

    /* Parsing a Json int array */
    {
        Stream *stm = i_stm_from_json("[ -321, 12, -8943, 228, -220, 347 ]");
        ArrSt(int16_t) *json = json_read(stm, NULL, ArrSt(int16_t));
        bstd_printf("ArrSt(int16_t) from Json: ");
        arrst_foreach(id, json, int16_t)
            bstd_printf("%d ", *id);
        arrst_end()
        bstd_printf("\n");
        json_destroy(&json, ArrSt(int16_t));
        stm_close(&stm);
    }

    /* Parsing a Json String array */
    {
        Stream *stm = i_stm_from_json("[ \"Red\", \"Green\", \"Blue\", \"Yellow\", \"Orange\" ]");
        ArrPt(String) *json = json_read(stm, NULL, ArrPt(String));
        bstd_printf("ArrPt(String) from Json: ");
        arrpt_foreach(str, json, String)
            bstd_printf("%s ", tc(str));
        arrpt_end()
        bstd_printf("\n");
        json_destroy(&json, ArrPt(String));
        stm_close(&stm);
    }

    /* Data binding (only once time in application) */
    /* This allows the Json parser to know the structure of the objects */
    dbind(Product, String *, description);
    dbind(Product, real32_t, price);
    dbind(Products, uint32_t, size);
    dbind(Products, ArrSt(Product) *, data);

    /* Parsing a Json object */
    {
        static const char_t *JSON_OBJECT = "\
        {\
            \"size\" : 3,\
            \"data\" : [\
                {\
                    \"description\" : \"Intel i7-7700K\",\
                    \"price\" : 329.99\
                },\
                {\
                    \"description\" : \"Ryzen-5-1600\",\
                    \"price\" : 194.99\
                },\
                {\
                    \"description\" : \"GTX-1060\",\
                    \"price\" : 449.99\
                }\
            ]\
        }";

        Stream *stm = i_stm_from_json(JSON_OBJECT);
        Products *json = json_read(stm, NULL, Products);
        bstd_printf("Products object from Json: size %d\n", json->size);
        arrst_foreach(elem, json->data, Product)
            bstd_printf("    Product: %s Price %.2f\n", tc(elem->description), elem->price);
        arrst_end()
        bstd_printf("\n");
        json_destroy(&json, Products);
        stm_close(&stm);
    }

    /* Writting data/objects to JSon */
    {
        Stream *stm = stm_memory(1024);

        /* Write boolean as Json */
        {
            bool_t data_bool = TRUE;
            stm_writef(stm, "Json from bool_t: ");
            json_write(stm, &data_bool, NULL, bool_t);
            stm_writef(stm, "\n");
        }

        /* Write unsigned integer as Json */
        {
            uint16_t data_uint = 6654;
            stm_writef(stm, "Json from uint16_t: ");
            json_write(stm, &data_uint, NULL, uint16_t);
            stm_writef(stm, "\n");
        }

        /* Write integer as Json */
        {
            int16_t data_int = -567;
            stm_writef(stm, "Json from int16_t: ");
            json_write(stm, &data_int, NULL, int16_t);
            stm_writef(stm, "\n");
        }

        /* Write real32_t as Json */
        {
            real32_t data_real = 456.45f;
            stm_writef(stm, "Json from real32_t: ");
            json_write(stm, &data_real, NULL, real32_t);
            stm_writef(stm, "\n");
        }

        /* Write String as Json */
        {
            String *data_str = str_c("Hello World");
            stm_writef(stm, "Json from String: ");
            json_write(stm, data_str, NULL, String);
            stm_writef(stm, "\n");
            str_destroy(&data_str);
        }

        /* Write Image as Json (string b64) */
        {
            Pixbuf *pixbuf = pixbuf_create(2, 2, ekGRAY8);
            Image *data_image = NULL;
            bmem_set1(pixbuf_data(pixbuf), 2 * 2, 128);
            data_image = image_from_pixbuf(pixbuf, NULL);
            stm_writef(stm, "Json from Image: ");
            json_write(stm, data_image, NULL, Image);
            stm_writef(stm, "\n");
            pixbuf_destroy(&pixbuf);
            image_destroy(&data_image);
        }

        /* Write int array as Json */
        {
            ArrSt(int16_t) *array = arrst_create(int16_t);
            arrst_append(array, -321, int16_t);
            arrst_append(array, 12, int16_t);
            arrst_append(array, -8943, int16_t);
            arrst_append(array, 228, int16_t);
            arrst_append(array, -220, int16_t);
            arrst_append(array, 347, int16_t);
            stm_writef(stm, "Json from int array: ");
            json_write(stm, array, NULL, ArrSt(int16_t));
            stm_writef(stm, "\n");
            arrst_destroy(&array, NULL, int16_t);
        }

        /* Write string array as Json */
        {
            ArrPt(String) *array = arrpt_create(String);
            arrpt_append(array, str_c("Red"), String);
            arrpt_append(array, str_c("Green"), String);
            arrpt_append(array, str_c("Blue"), String);
            arrpt_append(array, str_c("Yellow"), String);
            arrpt_append(array, str_c("Orange"), String);
            stm_writef(stm, "Json from string array: ");
            json_write(stm, array, NULL, ArrPt(String));
            stm_writef(stm, "\n");
            arrpt_destroy(&array, str_destroy, String);
        }

        /* Write object as Json */
        {
            Products *products = heap_new(Products);
            products->size = 3;
            products->data = arrst_create(Product);

            {
                Product *product = arrst_new(products->data, Product);
                product->description = str_c("Intel i7-7700K");
                product->price = 329.99f;
            }

            {
                Product *product = arrst_new(products->data, Product);
                product->description = str_c("Ryzen-5-1600");
                product->price = 194.99f;
            }

            {
                Product *product = arrst_new(products->data, Product);
                product->description = str_c("GTX-1060");
                product->price = 449.99f;
            }

            stm_writef(stm, "Json from object: ");
            json_write(stm, products, NULL, Products);
            stm_writef(stm, "\n");
            dbind_destroy(&products, Products);
        }

        {
            String *str = stm_str(stm);
            bstd_printf("%s\n", tc(str));
            str_destroy(&str);
        }

        stm_close(&stm);
    }

    draw2d_finish();
    return 0;
}
