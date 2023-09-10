#ifndef ASL_HANDLER_H
#define ASL_HANDLER_H

#include "anim_sequence.h"

#include "utils/resource_pool.h"

#define MAX_ASL_SCRIPTS 256
#define INVALID_ASL_ID INVALID_RES_ID

typedef res_id_t asl_id_t;

void init_asl_scripting();
asl_id_t load_asl_script(const char* filename);
void update_asl_scripts(float dt);

#endif // ASL_HANDLER_H
