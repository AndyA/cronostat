/* main.c */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "crono.h"
#include "utils.h"

#define CHANNELS 4

static int temp[CHANNELS];

static void parse_action(const char *action, int *channel, int *temp) {
  const char *sp = action;
  char *ep;
  if (*sp == '"') sp++;
  *channel = strtoul(sp, &ep, 10);
  if (sp == ep) die("Bad channel");
  sp = ep;
  *temp = strtoul(sp, &ep, 10);
  if (sp == ep) die("Bad temperature");
  sp = ep;
  if (*action == '"' && *sp == '"') sp++;
  if (*sp) die("Bad action");
}

static int action(void *ctx, const char *action, time_t when) {
  int ch, tp;
  (void) ctx;
  (void) when;
  parse_action(action, &ch, &tp);
  if (ch < 1 || ch > CHANNELS) die("Bad channel: %d", ch);
  temp[ch - 1] = tp;
  return 0;
}

static void dump_temp(int *temp, int channels) {
  for (int i = 0; i < channels; i++) {
    if (i) printf(", ");
    printf("%02d: ", i + 1);
    if (temp[i] < 0) printf("???");
    else printf("%3d", temp[i]);
  }
  printf("\n");
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

static void ts(char *s, size_t max) {
  time_t now = time(NULL);
  struct tm tm;
  localtime_r(&now, &tm);
  strftime(s, max, "%Y/%m/%d %H:%M:%S", &tm);
}

int main(int argc, char *argv[]) {
  crono_rule *rules = NULL;
  struct tm tm;
  char tsbuf[100];
  int i;

  get_time(&tm);

  for (i = 1; i < argc; i++)
    rules = read_crontab(rules, argv[i], &tm);

  for (i = 0; i < CHANNELS; i++)
    temp[i] = -1;

  for (;;) {
    time_t now = time(NULL);
    /*    dump_rules(rules);*/
    rules = crono_rule_trigger(rules, now);

    ts(tsbuf, sizeof(tsbuf));
    printf("%s ", tsbuf);
    dump_temp(temp, CHANNELS);

    unsigned snooze = rules->tm - now;
    printf("(sleeping for %u seconds)\n", snooze);
    sleep(snooze);
  }

  // keep compiler happy
  if (0) dump_rules(rules);
  crono_rule_free(rules);
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
