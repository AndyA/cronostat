/* parse.c */

#include <stdlib.h>
#include "crono.h"

int crono_parse_term(crono_field *cf, const char *spec) {
  char *sp;
  int min, max, step = 1;

  for (;;) {

    min = max = (int) strtoul(spec, &sp, 10);
    if (sp == spec) return -1;

    if (*sp == '-') {
      const char *ep = sp + 1;
      max = (int) strtoul(ep, &sp, 10);
      if (sp == ep) return -1;
    }

    if (*sp == '/') {
      const char *ep = sp + 1;
      step = (int) strtoul(ep, &sp, 10);
      if (sp == ep) return -1;
    }

    if (crono_field_add_range(cf, min, max, step))
      return -1;
  }
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
