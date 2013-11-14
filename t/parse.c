/* parse.t */

#include "tap.h"
#include "crono.h"

static void test_parse(void) {
  crono_rule cr;

  crono_rule_init(&cr);

  int rc = crono_rule_parse(&cr, "* * * * * t1");
  ok(rc == 0, "parse OK");
}

int main(void) {
  test_parse();
  done_testing();
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
