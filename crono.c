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
  crono_field_init(&cs->f[crono_WEEK_DAY], 1, 7);
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

static int is_leap(int year) {
  if (year % 400 == 0) return 1;
  if (year % 100 == 0) return 0;
  if (year %   4 == 0) return 1;
  return 0;
}

static int month_days(int month, int year) {
  static int mday[] = { -1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if (month == 2 && is_leap(year)) return 29;
  if (month >= 1 && month <= 12) return mday[month];
  return -1;
}

static int valid_date(int day, int month, int year) {
  if (month < 1 || month > 12) return 0;
  if (day < 1 || day > month_days(month, year)) return 0;
  return 1;
}

static int week_day(int day, int month, int year) {
  struct tm tm;

  tm.tm_sec = 0;
  tm.tm_min = 0;
  tm.tm_hour = 0;
  tm.tm_mday = day;
  tm.tm_mon = month;
  tm.tm_year = year;
  tm.tm_isdst = -1;

  mktime(&tm);

  return tm.tm_wday ? tm.tm_wday : 7;
}

int crono_schedule_valid(const crono_schedule *cs) {
  /* is date valid */
  if (!valid_date(cs->f[crono_DAY].pos,
                  cs->f[crono_MONTH].pos,
                  cs->f[crono_YEAR].pos))
    return 0;

  /* filter by week day */
  int wday = week_day(cs->f[crono_DAY].pos,
                      cs->f[crono_MONTH].pos,
                      cs->f[crono_YEAR].pos);

  if (!(cs->f[crono_WEEK_DAY].set & BIT(wday)))
    return 0;

  return 1;
}

static int crono__schedule_prev_day(crono_schedule *cs) {
  for (int i = 0; i < crono_DAY; i++) {
    crono_field_reset(&cs->f[i]);
    crono_field_prev(&cs->f[i]);
  }
  return crono__schedule_prev(cs, crono_DAY);
}

static int crono__schedule_next_day(crono_schedule *cs) {
  for (int i = 0; i < crono_DAY; i++)
    crono_field_reset(&cs->f[i]);
  return crono__schedule_next(cs, crono_DAY);
}

int crono_schedule_prev_valid(crono_schedule *cs) {
  crono_schedule_prev(cs);
  while (!crono_schedule_valid(cs))
    crono__schedule_prev_day(cs);
}

int crono_schedule_next_valid(crono_schedule *cs) {
  crono_schedule_next(cs);
  while (!crono_schedule_valid(cs))
    crono__schedule_next_day(cs);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
