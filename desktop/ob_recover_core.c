/* ob_recover_core: Icons > Recover deleted... and Put back, apart from the window.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "ob_recover_core.h"
#include "of_path.h"

int ob_rc_volume(const char *path, int disk, const char *trash, char *vol)
{
    const char *c;
    if (!path || !vol || !(c = strchr(path, ':')) || c == path)
        return 0;
    if (disk)
        return !c[1] && of_path_volume(vol, path);
    /* a trashcan left out: the trash drawer at its volume's top */
    return trash && trash[0] && !strchr(c + 1, '/') && of_path_equal(c + 1, trash) && of_path_volume(vol, path);
}

static int leap(int y) { return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0); }

/* "13-Nov-26 14:05" from a DateStamp's days (since 1 Jan 1978) and minute */
static void date_text(char *out, size_t size, int32_t days, int32_t minute)
{
    static const char mon[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    static const unsigned char len[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int y = 1978, m = 0;
    if (days < 0)
        days = 0;
    while (days >= (leap(y) ? 366 : 365))
        days -= leap(y++) ? 366 : 365;
    while (m < 11 && days >= len[m] + (m == 1 && leap(y)))
        days -= len[m] + (m == 1 && leap(y)), m++;
    snprintf(out, size, "%02d-%s-%02d %02d:%02d", (int)days + 1, mon[m], y % 100, (int)(minute / 60 % 24),
             (int)(minute % 60));
}

void ob_rc_row(char *out, size_t size, const of_gone *g)
{
    char d[24];
    date_text(d, sizeof d, g->deleted.days, g->deleted.minute);
    snprintf(out, size, "%s   %lu bytes   %s", g->name, (unsigned long)g->size, d);
}

typedef struct fill { ob_rc_add_fn add; void *user; int n, all, full; } fill;

static int fill_one(void *u, const of_gone *g)
{
    fill *f = u;
    char row[OF_GONE_NAME + 48];
    f->all++;
    if (f->full) return 0;                      /* no room: the rest aren't shown, only counted */
    ob_rc_row(row, sizeof row, g);
    if (f->add(f->user, g->id, row))
        f->full = 1;
    else
        f->n++;
    return 0;
}

static void cant(char *msg, size_t size, const char *vol, int err)
{
    if (err == OF_ERR_NOTSUP)
        snprintf(msg, size, "%s keeps no deleted files: only what is in its trash can be put back.", vol);
    else
        snprintf(msg, size, "The deleted files on %s can't be read: %s.", vol, of_strerror(err));
}

int ob_rc_fill(of_ctx *c, const char *vol, ob_rc_add_fn add, void *user, char *msg, size_t size)
{
    fill f;
    int rc;
    f.add = add;
    f.user = user;
    f.n = f.all = f.full = 0;
    msg[0] = 0;
    if ((rc = of_gone_list(c, vol, 1, fill_one, &f)) != OF_OK) {
        cant(msg, size, vol, rc);
        return -1;
    }
    if (!f.all)
        snprintf(msg, size, "Nothing deleted is kept on %s.", vol);
    else if (f.n < f.all)
        snprintf(msg, size, "%d deleted files kept on %s, the newest %d shown. Pick one, then Put back.", f.all, vol, f.n);
    else
        snprintf(msg, size, "%d deleted file%s kept on %s, newest first. Pick one, then Put back.", f.n,
                 f.n == 1 ? "" : "s", vol);
    return f.n;
}

int ob_rc_put_back(of_ctx *c, const char *vol, uint32_t id, char *msg, size_t size)
{
    char went[OF_PATH_MAX];
    int rc = of_undelete(c, vol, id, NULL, went);
    if (rc == OF_OK)
        snprintf(msg, size, "Put back as %s.", went);
    else if (rc == OF_ERR_NOTSUP)
        cant(msg, size, vol, rc);
    else
        snprintf(msg, size, "It couldn't be put back: %s.", of_strerror(rc));
    return rc;
}
