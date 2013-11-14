/* main.c */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "crono.h"
#include "utils.h"

static int action(void *ctx, const char *action, time_t when) {
  printf("Fired %s\n", action);
  return 0;
}

static crono_rule *read_file(crono_rule *rules, FILE *ct, struct tm *tm) {
  char lbuf[1024];

  while (fgets(lbuf, sizeof(lbuf), ct)) {
    size_t sl = strlen(lbuf);
    if (sl > 0 && lbuf[sl - 1] != '\n') die("Truncated line");
    lbuf[sl - 1] = '\0'; // chomp
    char *lp = lbuf;
    while (isspace(*lp)) lp++;
    if (*lp == '\0' || *lp == '#') continue;

    crono_rule *cr = crono_rule_new(action, NULL);
    if (crono_rule_parse(cr, lp))
      die("Can't parse %s", lbuf);

    crono_schedule_set(&cr->s, tm);
    crono_rule_prev(cr);

    rules = crono_rule_insert(rules, cr);
  }

  return rules;
}

static crono_rule *read_crontab(crono_rule *rules, const char *crontab, struct tm *tm) {
  FILE *fl;
  if (fl = fopen(crontab, "r"), !fl)
    die("Can't read %s: %m");
  rules = read_file(rules, fl, tm);
  fclose(fl);
  return rules;
}

static time_t get_time(struct tm *tm) {
  time_t t = time(NULL);
  localtime_r(&t, tm);
  return t;
}

static void dump_rules(crono_rule *rules) {
  while (rules) {
    printf("%lu [%04d/%02d/%02d %02d:%02d:%02d] %s\n",
           (unsigned long) rules->tm,
           rules->s.f[crono_YEAR].pos,
           rules->s.f[crono_MONTH].pos,
           rules->s.f[crono_DAY].pos,
           rules->s.f[crono_HOUR].pos,
           rules->s.f[crono_MINUTE].pos,
           rules->s.f[crono_SECOND].pos,
           rules->action);
    rules = rules->next;
  }
}

int main(int argc, char *argv[]) {
  crono_rule *rules = NULL;
  struct tm tm;
  time_t t = get_time(&tm);

  for (int i = 1; i < argc; i++)  {
    rules = read_crontab(rules, argv[i], &tm);
  }

  dump_rules(rules);

  rules = crono_rule_trigger(rules, t);

  dump_rules(rules);

  crono_rule_free(rules);
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
