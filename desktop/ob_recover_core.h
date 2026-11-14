/* ob_recover_core: what Icons > Recover deleted... and Put back do, apart from the window (Dale 13 Nov 2026: ACFS
 * keeps deleted files and does the work; gfopenfiles.library lists and restores them; Workbench has the two menu
 * items). Plain C over GFOpenFiles' engine calls (of_gone_list, of_undelete), so the host tests run it against the
 * in-memory disk.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_RECOVER_CORE_H
#define OB_RECOVER_CORE_H

#include <stddef.h>
#include "of_ops.h"

/* The volume ("Work:", into vol, OF_PATH_MAX) a selected desktop icon names: a disk ("Work:"), or a trashcan left
 * out on the desktop, at its volume's top ("Work:Trashcan", trash its name). 1, or 0 for anything else. */
int ob_rc_volume(const char *path, int disk, const char *trash, char *vol);

/* One kept file's row: "Docs/Letter   1840 bytes   13-Nov-26 14:05" */
void ob_rc_row(char *out, size_t size, const of_gone *g);

/* Each kept file of vol's whole volume (never the rest of its drive) given to add (newest first), its row made;
 * msg (size bytes) gets what the window's status line says. The count, or -1 when the volume can't (msg says why). */
typedef int (*ob_rc_add_fn)(void *user, uint32_t id, const char *row);
int ob_rc_fill(of_ctx *c, const char *vol, ob_rc_add_fn add, void *user, char *msg, size_t size);

/* Put back: kept file id of vol where it was (a name there gets another); msg says where it went or why not.
 * OF_OK or the engine's OF_ERR_*. */
int ob_rc_put_back(of_ctx *c, const char *vol, uint32_t id, char *msg, size_t size);

#endif
