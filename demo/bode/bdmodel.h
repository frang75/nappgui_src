/* Bode Model */

#include "bode.hxx"

void model_dbind(void);

void model_default(Model *model);

Model *model_read(void);

void model_save(Model *model, Window *window);

bool_t model_validate(Model *model);
