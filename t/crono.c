/* crono.t */

#include <time.h>
#include <stdlib.h>

#include "tap.h"
#include "crono.h"

static time_t _timegm(struct tm *tm) {
  time_t ret;
  char *tz;

  tz = getenv("TZ");
  setenv("TZ", "", 1);
  tzset();
  ret = mktime(tm);
  if (tz)
    setenv("TZ", tz, 1);
  else
    unsetenv("TZ");
  tzset();
  return ret;
}

static void test_field(void) {
  crono_field cf;

  crono_field_init(&cf, 1, 31);
  crono_field_add(&cf, 3);
  crono_field_add(&cf, 11);
  crono_field_add(&cf, 12);
  crono_field_add(&cf, 31);

  crono_field_next(&cf);
  is(cf.pos, 3, "next = 3");
  crono_field_next(&cf);
  is(cf.pos, 11, "next = 11");
  crono_field_next(&cf);
  is(cf.pos, 12, "next = 12");
  crono_field_next(&cf);
  is(cf.pos, 31, "next = 31");
  crono_field_next(&cf);
  is(cf.pos, 3, "next = 3");

  crono_field_prev(&cf);
  is(cf.pos, 31, "prev = 31");
  crono_field_prev(&cf);
  is(cf.pos, 12, "prev = 12");
  crono_field_prev(&cf);
  is(cf.pos, 11, "prev = 11");
  crono_field_prev(&cf);
  is(cf.pos, 3, "prev = 3");
}

static void init_schedule(crono_schedule *cs) {
  crono_schedule_init(cs);

  crono_field_add_range(&cs->f[crono_MINUTE], 0, 59, 10); // */10
  crono_field_add(&cs->f[crono_HOUR], 8);
  crono_field_add(&cs->f[crono_HOUR], 12);
  crono_field_add(&cs->f[crono_HOUR], 18);
  crono_field_add_range(&cs->f[crono_DAY], 1, 31, 1); // *
  crono_field_add_range(&cs->f[crono_MONTH], 1, 12, 1); // *
}

static void test_schedule_get(void) {
  crono_schedule cs;
  struct tm tm;

  init_schedule(&cs);

  cs.f[crono_MINUTE].pos = 59;
  cs.f[crono_HOUR].pos = 13;
  cs.f[crono_DAY].pos = 13;
  cs.f[crono_MONTH].pos = 11;
  cs.f[crono_YEAR].pos = 2013;

  crono_schedule_get(&cs, &tm);
  time_t tt = _timegm(&tm);

  is(tt, 1384351140, "time set");
}

static int is_schedule(const crono_schedule *cs,
                       int year, int month, int day,
                       int hour, int minute) {
  int tst = cs->f[crono_YEAR].pos == year &&
            cs->f[crono_MONTH].pos == month &&
            cs->f[crono_DAY].pos == day &&
            cs->f[crono_HOUR].pos == hour &&
            cs->f[crono_MINUTE].pos == minute;
  ok(tst, "schedule: %04d/%02d/%02d %02d:%02d", year, month, day, hour, minute);
  if (!tst) {
    diag("wanted: %04d/%02d/%02d %02d:%02d", year, month, day, hour, minute);
    diag("   got: %04d/%02d/%02d %02d:%02d",
         cs->f[crono_YEAR].pos,
         cs->f[crono_MONTH].pos,
         cs->f[crono_DAY].pos,
         cs->f[crono_HOUR].pos,
         cs->f[crono_MINUTE].pos);
  }
  return tst;
}

static void test_schedule_set(void) {
  crono_schedule cs;
  struct tm tm;
  time_t tt;

  init_schedule(&cs);

  tt = 1384351140;
  gmtime_r(&tt, &tm);

  {
    crono_schedule_set(&cs, &tm);
    is_schedule(&cs, 2013, 11, 13, 13, 59);
    ok(!crono_schedule_snapped(&cs), "not snapped");

    crono_schedule_snap_prev(&cs);
    is_schedule(&cs, 2013, 11, 13, 12, 50);
    ok(crono_schedule_snapped(&cs), "snapped");
  }

  {
    crono_schedule_set(&cs, &tm);
    is_schedule(&cs, 2013, 11, 13, 13, 59);
    ok(!crono_schedule_snapped(&cs), "not snapped");

    crono_schedule_snap_next(&cs);
    is_schedule(&cs, 2013, 11, 13, 18, 0);
    ok(crono_schedule_snapped(&cs), "snapped");
  }
}

int main(void) {
  test_field();
  test_schedule_get();
  test_schedule_set();
  done_testing();
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
