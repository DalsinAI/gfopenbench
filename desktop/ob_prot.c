/* ob_prot.c: see ob_prot.h. MIT, Copyright (c) 2026 AmiCradle Team. */
#include "ob_prot.h"

#include <string.h>

int ob_prot_is_advanced(const char *name)
{
    return name && (!strcmp(name, "Hold") || !strcmp(name, "Pure"));
}

int ob_prot_is_main_row(const char *name)
{
    static const char *const row[] = {
        "Script", "Archived", "Readable", "Writable", "Executable", "Deletable", 0
    };
    int i;
    if (!name) return 0;
    for (i = 0; row[i]; i++)
        if (!strcmp(name, row[i])) return 1;
    return 0;
}

int ob_prot_hold_on(long prot)
{
    return (prot & OB_PROT_HOLD) != 0;
}

int ob_prot_pure_on(long prot)
{
    return (prot & OB_PROT_PURE) != 0;
}

long ob_prot_apply(long prot, int script, int archived, int readable, int writable,
                   int executable, int deletable, int hold, int pure)
{
    long p = prot & ~0xFFL;
    if (script) p |= OB_PROT_SCRIPT;
    if (archived) p |= OB_PROT_ARCHIVE;
    if (!readable) p |= OB_PROT_READ;
    if (!writable) p |= OB_PROT_WRITE;
    if (!executable) p |= OB_PROT_EXECUTE;
    if (!deletable) p |= OB_PROT_DELETE;
    if (hold) p |= OB_PROT_HOLD;
    if (pure) p |= OB_PROT_PURE;
    return p;
}
