/* crono.c */

#include "crono.h"

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

int crono_schedule_get(const crono_schedule *cs, struct tm *tm) {
  tm->tm_sec = 0;
  tm->tm_min = cs->f[crono_MINUTE].pos;
  tm->tm_hour = cs->f[crono_HOUR].pos;
  tm->tm_mday = cs->f[crono_DAY].pos;
  tm->tm_mon = cs->f[crono_MONTH].pos;
  tm->tm_year = cs->f[crono_YEAR].pos;
  tm->tm_isdst = -1;
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
