/* crono.c */

#include <stdio.h>
#include <stdint.h>

typedef struct {
  uint64_t set;
  int min, max;
  int pos;
} crono_field;

enum {
  crono_MINUTE, crono_HOUR, crono_DAY, crono_MONTH, crono_YEAR, /* iter */
  crono_WEEK_DAY, /* filter */
  crono_FIELDS
};

typedef struct {
  crono_field f[crono_FIELDS];
} crono_schedule;

int crono_field_size(const crono_field *cf) {
  int size = 0;
  for (int i = cf->min; i <= cf->max; i++) {
    if (cf->set & (1 << i)) size++;
  }
  return size;
}

int crono_field_prev(crono_field *cf) {
  int wrap = 0;

  /* special case: empty set is unbounded iter */
  if (!cf->set) {
    cf->pos--;
    return 0;
  }

  int pos = cf->pos;

  for (;;) {
    pos--;
    if (pos < cf->min) {
      if (wrap) return -1;
      wrap++;
      pos = cf->max;
    }
    if (cf->set & (1 << pos)) {
      cf->pos = pos;
      return wrap;
    }
  }
}

int crono_field_next(crono_field *cf) {
  int wrap = 0;

  /* special case: empty set is unbounded iter */
  if (!cf->set) {
    cf->pos++;
    return 0;
  }

  int pos = cf->pos;

  for (;;) {
    pos++;
    if (pos > cf->max) {
      if (wrap) return -1;
      wrap++;
      pos = cf->min;
    }
    if (cf->set & (1 << pos)) {
      cf->pos = pos;
      return wrap;
    }
  }
}

int crono_schedule_prev(crono_schedule *cs) {
  for (int i = 0; i <= crono_YEAR; i++) {
    int wrap = crono_field_prev(&(cs->f[i]));
    if (wrap <= 0) return wrap;
  }
  return 1;
}

int crono_schedule_next(crono_schedule *cs) {
  for (int i = 0; i <= crono_YEAR; i++) {
    int wrap = crono_field_next(&(cs->f[i]));
    if (wrap <= 0) return wrap;
  }
  return 1;
}

int main(void) {
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */





































