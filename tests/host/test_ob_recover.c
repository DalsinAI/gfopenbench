/* Host test for Icons > Recover deleted... and Put back (desktop/ob_recover_core.c) against GFOpenFiles' in-memory
 * disk answering as an ACFS drive (of_mem_acfs): which volume a selected icon names, the rows, the status line,
 * Put back where it was and beside a name in use, and a drive that isn't ACFS.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "of_ops.h"
#include "of_fs_mem.h"
#include "ob_recover_core.h"

static int checks, fails;
#define CHECK(c) do { checks++; if (!(c)) { fails++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

static uint32_t ids[8];
static char rows[8][300];
static int n, room = 8;
static int add(void *u, uint32_t id, const char *row)
{
    (void)u;
    if (n >= room) return 1;
    ids[n] = id;
    strcpy(rows[n++], row);
    return 0;
}

int main(void)
{
    of_fs *fs = of_mem_new();
    of_ctx c;
    char vol[512], msg[600], row[300];
    unsigned long len;
    of_gone g;

    puts("the volume a selected icon names");
    CHECK(ob_rc_volume("Work:", 1, "Trashcan", vol) && !strcmp(vol, "Work:"));
    CHECK(ob_rc_volume("Work:Trashcan", 0, "Trashcan", vol) && !strcmp(vol, "Work:"));
    CHECK(ob_rc_volume("Work:trashcan", 0, "Trashcan", vol));
    CHECK(!ob_rc_volume("Work:Tools/Trashcan", 0, "Trashcan", vol));
    CHECK(!ob_rc_volume("Work:Tools", 0, "Trashcan", vol));
    CHECK(!ob_rc_volume("Work:Tools", 1, "Trashcan", vol));
    CHECK(!ob_rc_volume(":x", 1, "Trashcan", vol) && !ob_rc_volume(NULL, 1, "Trashcan", vol));

    puts("a row");
    memset(&g, 0, sizeof g);
    g.size = 1840;
    g.deleted.days = 17848;
    g.deleted.minute = 845;
    strcpy(g.name, "Docs/Letter");
    ob_rc_row(row, sizeof row, &g);
    CHECK(!strcmp(row, "Docs/Letter   1840 bytes   13-Nov-26 14:05"));

    of_mem_add_volume(fs, "Work:", 1000000, 0);
    of_ctx_init(&c, fs);

    puts("a drive that isn't ACFS");
    CHECK(ob_rc_fill(&c, "Work:", add, NULL, msg, sizeof msg) == -1 && strstr(msg, "Work: keeps no deleted files"));
    CHECK(ob_rc_put_back(&c, "Work:", 1, msg, sizeof msg) == OF_ERR_NOTSUP && strstr(msg, "keeps no deleted files"));

    puts("ACFS: nothing kept, then the whole drive's, newest first");
    of_mem_acfs(fs, 1);
    CHECK(ob_rc_fill(&c, "Work:", add, NULL, msg, sizeof msg) == 0 && !strcmp(msg, "Nothing deleted is kept on Work:."));
    of_mem_kept(fs, "Work:.amigachrome-deleted/3 a", "A", 1, "Docs/a");
    of_mem_kept(fs, "Work:.amigachrome-deleted/7 top", "T", 1, "top");
    n = 0;
    CHECK(ob_rc_fill(&c, "Work:", add, NULL, msg, sizeof msg) == 2 && n == 2);
    CHECK(ids[0] == 7 && ids[1] == 3 && !strncmp(rows[0], "top   1 bytes   ", 16) && !strncmp(rows[1], "Docs/a   ", 9));
    CHECK(strstr(msg, "2 deleted files kept on Work:"));
    room = 1;
    n = 0;
    CHECK(ob_rc_fill(&c, "Work:", add, NULL, msg, sizeof msg) == 1 && n == 1);
    /* the window holds 256 rows: past that the status line still counts every kept file, and says how many show */
    CHECK(!strcmp(msg, "2 deleted files kept on Work:, the newest 1 shown. Pick one, then Put back."));
    room = 8;

    puts("Put back: where it was; a name in use gets another");
    CHECK(ob_rc_put_back(&c, "Work:", 3, msg, sizeof msg) == OF_OK && !strcmp(msg, "Put back as Work:Docs/a."));
    CHECK(of_mem_get(fs, "Work:Docs/a", &len) != NULL);
    of_mem_put(fs, "Work:top", "NEW", 3);
    CHECK(ob_rc_put_back(&c, "Work:", 7, msg, sizeof msg) == OF_OK && !strcmp(msg, "Put back as Work:top.1."));
    CHECK(ob_rc_put_back(&c, "Work:", 7, msg, sizeof msg) == OF_ERR_NOTFOUND && strstr(msg, "couldn't be put back"));
    n = 0;
    CHECK(ob_rc_fill(&c, "Work:", add, NULL, msg, sizeof msg) == 0);

    of_mem_free(fs);
    printf("test_ob_recover: %d checks, %d failed\n", checks, fails);
    return fails != 0;
}
