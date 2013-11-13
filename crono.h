/* crono.h */

#ifndef CRONO_H_
#define CRONO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>

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

  int crono_field_size(const crono_field *cf);
  int crono_field_prev(crono_field *cf);
  int crono_field_next(crono_field *cf);

  int crono_schedule_prev(crono_schedule *cs);
  int crono_schedule_next(crono_schedule *cs);

#if 0
  struct tm {
    int tm_sec;         /* seconds */
    int tm_min;         /* minutes */
    int tm_hour;        /* hours */
    int tm_mday;        /* day of the month */
    int tm_mon;         /* month */
    int tm_year;        /* year */
    int tm_wday;        /* day of the week */
    int tm_yday;        /* day in the year */
    int tm_isdst;       /* daylight saving time */
  };
#endif

  int crono_schedule_get(const crono_schedule *cs, struct tm *tm);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
