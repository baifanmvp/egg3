#ifndef EGGALLOCFREE_H_
#define EGGALLOCFREE_H_
#include "emacros.h"
#include <stdlib.h>

E_BEGIN_DECLS

void *eggCalloc(char *name, size_t nmemb, size_t size);
void *eggMalloc(char *name, size_t size);
void eggFree(char *name, void *ptr);
void *eggRealloc(char *name, void *ptr, size_t size);


E_END_DECLS


#endif //EGGALLOCFREE_H_

