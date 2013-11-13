/* crono.c */

#include "crono.h"

#define BIT(x) ((uint64_t) 1 << (x))

int crono_field_init(crono_field *cf, int min, int max) {
  cf->min = min;
  cf->max = max;
  cf->set = 0;
  cf->pos = min - 1;
  return 0;
}

int crono_field_add(crono_field *cf, int pos) {
  cf->set |= BIT(pos);
  return 0;
}

int crono_field_add_range(crono_field *cf, int min, int max, int step) {
  for (int i = min; i <= max; i += step)
    crono_field_add(cf, i);
  return 0;
}

int crono_field_remove(crono_field *cf, int pos) {
  cf->set &= ~BIT(pos);
  return 0;
}

int crono_field_size(const crono_field *cf) {
  int size = 0;
  for (int i = cf->min; i <= cf->max; i++) {
    if (cf->set & BIT(i)) size++;
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
    if (cf->set & BIT(pos)) {
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
    if (cf->set & BIT(pos)) {
      cf->pos = pos;
      return wrap;
    }
  }
}

int crono_field_snapped(const crono_field *cf) {
  return !cf->set ||
         (cf->pos >= cf->min &&
          cf->pos <= cf->max &&
          (cf->set & BIT(cf->pos)));
}

int crono_field_snap_prev(crono_field *cf) {
  if (crono_field_snapped(cf)) return 0;
  return crono_field_prev(cf);
}

int crono_field_snap_next(crono_field *cf) {
  if (crono_field_snapped(cf)) return 0;
  return crono_field_next(cf);
}

int crono_field_reset(crono_field *cf) {
  if (!cf->set) return 0; /* unbounded */
  cf->pos = cf->min - 1;
  return crono_field_next(cf);
}

int crono_schedule_init(crono_schedule *cs) {
  crono_field_init(&cs->f[crono_MINUTE], 0, 59);
  crono_field_init(&cs->f[crono_HOUR], 0, 23);
  crono_field_init(&cs->f[crono_DAY], 1, 31);
  crono_field_init(&cs->f[crono_MONTH], 1, 12);
  crono_field_init(&cs->f[crono_YEAR], 0, 0);
  return 0;
}

static int crono__schedule_prev(crono_schedule *cs, int fp) {
  for (int i = fp; i <= crono_YEAR; i++) {
    int wrap = crono_field_prev(&(cs->f[i]));
    if (wrap <= 0) return wrap;
  }
  return 1;
}

static int crono__schedule_next(crono_schedule *cs, int fp) {
  for (int i = fp; i <= crono_YEAR; i++) {
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
  tm->tm_mon = cs->f[crono_MONTH].pos - 1;
  tm->tm_year = cs->f[crono_YEAR].pos - 1900;
  tm->tm_isdst = -1;
  return 0;
}

int crono_schedule_set(crono_schedule *cs, const struct tm *tm) {
  cs->f[crono_MINUTE].pos = tm->tm_min;
  cs->f[crono_HOUR].pos = tm->tm_hour;
  cs->f[crono_DAY].pos = tm->tm_mday;
  cs->f[crono_MONTH].pos = tm->tm_mon + 1;
  cs->f[crono_YEAR].pos = tm->tm_year + 1900;
  return 0;
}

int crono_schedule_snapped(const crono_schedule *cs) {
  for (int i = 0; i <= crono_YEAR; i++)
    if (!crono_field_snapped(&cs->f[i]))
      return 0;
  return 1;
}

int crono_schedule_snap_next(crono_schedule *cs) {
  for (int i = 0; i <= crono_YEAR; i++) {
    if (!crono_field_snapped(&cs->f[i]))
      crono__schedule_next(cs, i);
  }
  return 0;
}

int crono_schedule_snap_prev(crono_schedule *cs) {
  for (int i = 0; i <= crono_YEAR; i++) {
    if (!crono_field_snapped(&cs->f[i]))
      crono__schedule_prev(cs, i);
  }
  return 0;
}

int crono_schedule_prev(crono_schedule *cs) {
  if (!crono_schedule_snapped(cs))
    return crono_schedule_snap_prev(cs);
  return crono__schedule_prev(cs, 0);
}

int crono_schedule_next(crono_schedule *cs) {
  if (!crono_schedule_snapped(cs))
    return crono_schedule_snap_next(cs);
  return crono__schedule_next(cs, 0);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
