/* A7: UpdateWorkbench's verdict (tests/amiga/rows/API_judge.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "API.h"

void api_plan(void);
WBGAP_PLAN(api_plan)

void api_plan(void)
{
    char why[200];
    CHECK(wbgap_api_update_judge(0, 1, 1, why, sizeof why) == WBGAP_PASS && !why[0], "IoErr 0, shown: PASS");
    CHECK(wbgap_api_update_judge(205, 1, 1, why, sizeof why) == WBGAP_FAIL && !strcmp(why, "UpdateWorkbench set IoErr"),
          "IoErr set: FAIL first, even when shown, \"%s\"", why);
    CHECK(wbgap_api_update_judge(-1, 0, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "IoErr"),
          "any IoErr but 0 counts, before the window");
    CHECK(wbgap_api_update_judge(0, 0, 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "isn't open"),
          "no window: FAIL, \"%s\"", why);
    CHECK(wbgap_api_update_judge(0, 1, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "100 ms"),
          "not shown: FAIL, \"%s\"", why);
    CHECK(wbgap_api_update_judge(0, 1, 1, NULL, 0) == WBGAP_PASS, "no room for a reason: still judged");
}
