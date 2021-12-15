/*
 * Simple APIs for Windows .ini file parsing.
 *
 * Copyright (c) 2021 Man Hung-Coeng <udc577@126.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "simple_ini_config.h"

#ifdef __cplusplus
extern "C" {
#endif

char const *g_ini_newline = "\n";

int ini_parse(const FILE *stream, struct ini_cfg_t *cfg, char *desc)
{
    return 0; /* TODO */
}

int ini_dump(const struct ini_cfg_t *cfg, FILE *stream)
{
    return 0; /* TODO */
}

void ini_destroy(struct ini_cfg_t *cfg)
{
    ; /* TODO */
}

struct ini_node_t* ini_find_section(struct ini_cfg_t *cfg, const char *name)
{
    return NULL; /* TODO */
}

struct ini_node_t* ini_find_item(struct ini_section_t *section, const char *key)
{
    return NULL; /* TODO */
}

#ifdef TEST

int main(int argc, char **argv)
{
    printf("TODO ...\n");

    return 0;
}

#endif /* #ifdef TEST */

#ifdef __cplusplus
}
#endif

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2021-12-15, Man Hung-Coeng:
 *  01. Create.
 */

