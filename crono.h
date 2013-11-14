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

  typedef time_t (*crono_mktime_cb)(struct tm *tm);
  typedef int (*crono_action_cb)(void *ctx, const char *action, time_t when);

  typedef struct crono_rule {
    crono_schedule s;
    time_t tm; /* derived from schedule */

    const char *action;
    struct crono_rule *next;

    crono_mktime_cb mktime_cb;

    void *action_ctx;
    crono_action_cb action_cb;

  } crono_rule;

  int crono_field_init(crono_field *cf, int min, int max);
  int crono_field_add(crono_field *cf, int pos);
  int crono_field_add_range(crono_field *cf, int min, int max, int step);
  int crono_field_size(const crono_field *cf);
  int crono_field_prev(crono_field *cf);
  int crono_field_next(crono_field *cf);
  int crono_field_snapped(const crono_field *cf);
  int crono_field_snap_prev(crono_field *cf);
  int crono_field_snap_next(crono_field *cf);
  int crono_field_reset(crono_field *cf);

  int crono_schedule_init(crono_schedule *cs);

  int crono_schedule_get(const crono_schedule *cs, struct tm *tm);
  int crono_schedule_set(crono_schedule *cs, const struct tm *tm);

  int crono_schedule_snapped(const crono_schedule *cs);
  int crono_schedule_snap_next(crono_schedule *cs);
  int crono_schedule_snap_prev(crono_schedule *cs);

  int crono_schedule_prev(crono_schedule *cs);
  int crono_schedule_next(crono_schedule *cs);

  int crono_schedule_valid(const crono_schedule *cs);
  int crono_schedule_prev_valid(crono_schedule *cs);
  int crono_schedule_next_valid(crono_schedule *cs);

  int crono_rule_init(crono_rule *cr);
  crono_rule *crono_rule_new(crono_action_cb cb, void *ctx);
  void crono_rule_free(crono_rule *cr);
  crono_rule *crono_rule_add(crono_rule *list, crono_rule *cr);

  int crono_rule_parse(crono_rule *cr, const char *ent);
  int crono_rule_prev(crono_rule *cr);
  int crono_rule_next(crono_rule *cr);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
