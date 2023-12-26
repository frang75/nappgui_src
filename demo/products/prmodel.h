/* Products Model */

#include "products.hxx"

Model *model_create(void);

void model_destroy(Model **model);

wserv_t model_webserv(Model *model);

bool_t model_import(Model *model, const char_t *pathname, ferror_t *err);

bool_t model_export(Model *model, const char_t *pathname, ferror_t *err);

uint32_t model_count(const Model *model);

void model_clear(Model *model);

void model_add(Model *model);

void model_delete(Model *model, const uint32_t index);

bool_t model_filter(Model *model, const char_t *filter);

Product *model_product(Model *model, const uint32_t product_id);

void model_bind(void);

void model_layout(Layout *layout);

void model_type(Cell *cell);

void model_code(Cell *cell);

void model_desc(Cell *cell);

void model_image(Cell *cell);

void model_price(Cell *cell);
