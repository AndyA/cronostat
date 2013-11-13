/* parse.c */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "crono.h"

static int parse_field(crono_field *cf, const char *spec, char **sp) {
  for (;;) {
    int min, max, step = 1;

    if (*spec == '*') {
      min = cf->min;
      max = cf->max;
      *sp = spec + 1;
    }
    else {
      min = max = (int) strtoul(spec, sp, 10);
      if (*sp == spec) return -1;

      if (**sp == '-') {
        const char *ep = *sp + 1;
        max = (int) strtoul(ep, sp, 10);
        if (*sp == ep) return -1;
      }
    }

    if (**sp == '/') {
      const char *ep = *sp + 1;
      step = (int) strtoul(ep, sp, 10);
      if (*sp == ep) return -1;
    }

    if (crono_field_add_range(cf, min, max, step))
      return -1;

    if (**sp != ',') break;

    spec = *sp + 1; /* round again */
  }

  return 0;
}

int crono_rule_parse(crono_rule *cr, const char *ent) {
  char *sp;
  for (int i = 0; i < crono_FIELDS; i++) {
    if (i == crono_YEAR) continue; /* no year field */
    if (parse_field(&cr->s.f[i], ent, &sp))
      return -1;
    while (isspace(*sp)) sp++;
    ent = sp;
  }
  cr->action = strdup(ent);
  if (!cr->action) return -1;
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
