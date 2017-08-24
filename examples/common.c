#include "common.h"
#include <stdio.h>
#include <stdlib.h>

char dbpath[1024];

void
db_set_path(void)
{
        sprintf(dbpath, "%s/tmp/%s", getenv("HOME"), DBNAME);
}
