/* ob_rexx's host test: GFOpenBench's ARexx commands (0.1.50). Commands are found in any case with their arguments
 * after them, GFOpenBench's own only on OPENBENCH; GETATTR's object names match with their numbers taken out; HELP
 * lists the commands and gives a template.
 *   test_ob_rexx
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "ob_rexx.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

int main(void)
{
    const char *args = NULL;
    long nums[4];
    char help[512];
    int i;

    /* the commands, any case, the arguments after them */
    CHECK(ob_rexx_find("WINDOW RAM:WBGap OPEN", &args, 0) == OB_RX_WINDOW && !strcmp(args, "RAM:WBGap OPEN"),
          "WINDOW");
    CHECK(ob_rexx_find("  getattr   application.version", &args, 0) == OB_RX_GETATTR &&
          !strcmp(args, "application.version"), "getattr");
    CHECK(ob_rexx_find("NewDrawer RAM:WBGap/rexxdir", &args, 0) == OB_RX_NEWDRAWER, "NewDrawer");
    CHECK(ob_rexx_find("LOCKGUI", &args, 0) == OB_RX_LOCKGUI && !*args, "LOCKGUI, nothing after");
    CHECK(ob_rexx_find("RX CMD say 1", &args, 0) == OB_RX_RX, "RX");
    /* a word that only starts like one is not that one */
    CHECK(ob_rexx_find("WINDOWS root", &args, 0) == -1, "WINDOWS isn't WINDOW");
    CHECK(ob_rexx_find("WINDOWTOFRONT root", &args, 0) == OB_RX_WINDOWTOFRONT, "WINDOWTOFRONT");
    CHECK(ob_rexx_find("WIN root", &args, 0) == -1, "WIN");
    CHECK(ob_rexx_find("", &args, 0) == -1 && ob_rexx_find("   ", &args, 0) == -1 && ob_rexx_find(NULL, &args, 0) == -1,
          "nothing");
    /* GFOpenBench's own: on OPENBENCH only */
    CHECK(ob_rexx_find("VERSION", &args, 0) == -1, "VERSION on WORKBENCH");
    CHECK(ob_rexx_find("version", &args, 1) == OB_RX_VERSION, "VERSION on OPENBENCH");
    CHECK(ob_rexx_find("OPEN SYS:Utilities", &args, 1) == OB_RX_OPEN && !strcmp(args, "SYS:Utilities"), "OPEN");
    CHECK(ob_rexx_find("DELETE RAM:x ALL", &args, 1) == OB_RX_DELETE, "3.2.3's on OPENBENCH too");
    /* every command has a name and a template, and is found by it */
    for (i = 0; i < OB_RX_COUNT; i++)
        CHECK(ob_rexx_cmds[i].name && ob_rexx_cmds[i].template &&
              ob_rexx_find(ob_rexx_cmds[i].name, &args, 1) == i, "command %d", i);
    CHECK(!strcmp(ob_rexx_port_names[0], "WORKBENCH") && !strcmp(ob_rexx_port_names[1], "OPENBENCH"), "the ports");

    /* GETATTR's names */
    CHECK(ob_rexx_attr("application.version", "application.version", nums, 4), "plain");
    CHECK(ob_rexx_attr("Application.VERSION", "application.version", nums, 4), "any case");
    CHECK(!ob_rexx_attr("application.versions", "application.version", nums, 4), "longer");
    CHECK(!ob_rexx_attr("application.versio", "application.version", nums, 4), "shorter");
    CHECK(ob_rexx_attr("windows.12", "windows.#", nums, 4) && nums[0] == 12, "windows.12");
    CHECK(!ob_rexx_attr("windows.", "windows.#", nums, 4), "no number");
    CHECK(!ob_rexx_attr("windows.x", "windows.#", nums, 4), "not a number");
    CHECK(ob_rexx_attr("window.icons.all.3.name", "window.icons.all.#.name", nums, 4) && nums[0] == 3, "icon 3");
    CHECK(ob_rexx_attr("keycommands.0.key", "keycommands.#.key", nums, 4) && nums[0] == 0, "keycommand 0");
    CHECK(ob_rexx_attr("a.1.b.22", "a.#.b.#", nums, 4) && nums[0] == 1 && nums[1] == 22, "two numbers");
    CHECK(!ob_rexx_attr("windows.999999999999", "windows.#", nums, 4), "too big");

    /* HELP */
    CHECK(ob_rexx_help(help, sizeof help, NULL, 0) && strstr(help, "ACTIVATEWINDOW CHANGEWINDOW") == help &&
          strstr(help, " ZOOMWINDOW") && !strstr(help, "VERSION"), "HELP: %s", help);
    CHECK(ob_rexx_help(help, sizeof help, "", 1) && strstr(help, " VERSION"), "HELP on OPENBENCH: %s", help);
    CHECK(ob_rexx_help(help, sizeof help, "rename", 0) && !strcmp(help, "OLDNAME/A,NEWNAME/A"), "HELP RENAME: %s", help);
    CHECK(!ob_rexx_help(help, sizeof help, "NOSUCH", 0), "HELP NOSUCH");
    CHECK(ob_rexx_help(help, 20, NULL, 0) && !strcmp(help, "ACTIVATEWINDOW"), "whole names only: %s", help);

    printf("ob_rexx: %d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
