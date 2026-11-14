/* ---- A5: WorkbenchControlA's tags ----
 * Each settable value is read, set to a test value, read back and put back as it was: the default stack size, the
 * type restart time, the global flags (WBF_OLDDATESFIRST flipped) and the hidden devices (WBGAPA5 added, looked
 * for, removed). Every reading must be what 3.2.3's workbench.library gives (wbgap_pairs_judge).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/lists.h>
#include <utility/tagitem.h>
#include <workbench/workbench.h>
#include <proto/exec.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "WBCONTROL.h"

WBGAP_MODE(WBCONTROL, wbt_wbcontrol, SCRIPT)
WBGAP_ROWMODE(A5, WBCONTROL)

static int hidden_has(const char *name)
{
    struct List *hidden = NULL;
    struct Node *nd;
    int found = 0;
    if (!WorkbenchControl(NULL, WBCTRLA_GetHiddenDeviceList, (ULONG)&hidden, TAG_END)) return -1;
    if (!hidden) return 0;
    for (nd = hidden->lh_Head; nd->ln_Succ; nd = nd->ln_Succ)
        if (nd->ln_Name && !stricmp(nd->ln_Name, name)) found++;
    WorkbenchControl(NULL, WBCTRLA_FreeHiddenDeviceList, (ULONG)hidden, TAG_END);
    return found;
}

int wbt_wbcontrol(int argc, char **argv)
{
    static const char *const names[] = {
        "GetDefaultStackSize", "the stack after SetDefaultStackSize 12344", "GetTypeRestartTime",
        "the restart time after SetTypeRestartTime 7", "GetGlobalFlags", "the flags after OLDDATESFIRST flipped",
        "AddHiddenDeviceName WBGAPA5", "WBGAPA5 in GetHiddenDeviceList", "RemoveHiddenDeviceName WBGAPA5",
        "WBGAPA5 in the list after Remove"
    };
    long want[10], got[10];
    ULONG stack = 0, restart = 0, flags = 0, v;
    char why[160];
    int n = 0, ok;
    (void)argc, (void)argv;
    /* a value is set and put back only if its Get worked: a Set with nothing to put back would leave it changed */
    ok = WorkbenchControl(NULL, WBCTRLA_GetDefaultStackSize, (ULONG)&stack, TAG_END) != 0;
    got[n] = ok; want[n++] = 1;
    v = 0;
    if (wbgap_ctl_may_set(ok)) {
        WorkbenchControl(NULL, WBCTRLA_SetDefaultStackSize, 12344, TAG_END);
        WorkbenchControl(NULL, WBCTRLA_GetDefaultStackSize, (ULONG)&v, TAG_END);
        WorkbenchControl(NULL, WBCTRLA_SetDefaultStackSize, stack, TAG_END);
    }
    got[n] = wbgap_ctl_reading(ok, (long)v); want[n++] = 12344;
    ok = WorkbenchControl(NULL, WBCTRLA_GetTypeRestartTime, (ULONG)&restart, TAG_END) != 0;
    got[n] = ok; want[n++] = 1;
    v = 0;
    if (wbgap_ctl_may_set(ok)) {
        WorkbenchControl(NULL, WBCTRLA_SetTypeRestartTime, 7, TAG_END);
        WorkbenchControl(NULL, WBCTRLA_GetTypeRestartTime, (ULONG)&v, TAG_END);
        WorkbenchControl(NULL, WBCTRLA_SetTypeRestartTime, restart, TAG_END);
    }
    got[n] = wbgap_ctl_reading(ok, (long)v); want[n++] = 7;
    ok = WorkbenchControl(NULL, WBCTRLA_GetGlobalFlags, (ULONG)&flags, TAG_END) != 0;
    got[n] = ok; want[n++] = 1;
    v = 0;
    if (wbgap_ctl_may_set(ok)) {
        WorkbenchControl(NULL, WBCTRLA_SetGlobalFlags, flags ^ WBF_OLDDATESFIRST, TAG_END);
        WorkbenchControl(NULL, WBCTRLA_GetGlobalFlags, (ULONG)&v, TAG_END);
        WorkbenchControl(NULL, WBCTRLA_SetGlobalFlags, flags, TAG_END);
    }
    got[n] = wbgap_ctl_reading(ok, (long)v); want[n++] = (long)(flags ^ WBF_OLDDATESFIRST);
    got[n] = WorkbenchControl(NULL, WBCTRLA_AddHiddenDeviceName, (ULONG)"WBGAPA5", TAG_END) != 0; want[n++] = 1;
    got[n] = hidden_has("WBGAPA5"); want[n++] = 1;
    got[n] = WorkbenchControl(NULL, WBCTRLA_RemoveHiddenDeviceName, (ULONG)"WBGAPA5", TAG_END) != 0; want[n++] = 1;
    got[n] = hidden_has("WBGAPA5"); want[n++] = 0;
    printf("INFO A5 default stack %lu, restart time %lu, flags $%lx (each put back if its Get worked)\n", (unsigned long)stack,
           (unsigned long)restart, (unsigned long)flags);
    wbt_say("A5", wbgap_pairs_judge(n, names, want, got, why, sizeof why), why);
    return wbt_fails() ? 5 : 0;
}
