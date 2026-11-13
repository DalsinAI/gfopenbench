/* ob_wb: GFOpenBench as the Workbench that workbench.library knows (GFOpenBench gap item 4; Dale, 12 November 2026:
 * "we need to close this"). See ob_wb.c.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_WB_H
#define OB_WB_H

#include <exec/types.h>
#include <exec/ports.h>
#include <utility/tagitem.h>
#include <workbench/workbench.h>
#include "wb_drawer.h"

/* The Team's WorkbenchControlA tags for a registered desktop: a copy of amicradle-guest
 * libraries/openworkbench/owb_team.h's lines (keep the two the same). OS 3.2.3's workbench.library ignores them. */
#ifndef OWBCTRLA_Dummy
#define OWBCTRLA_Dummy          (TAG_USER + 0x4F574200UL)
#endif
#define OWBCTRLA_Desktop        (OWBCTRLA_Dummy + 3)
#define OWBCTRLA_TakeNotes      (OWBCTRLA_Dummy + 4)
#define OWB_NOTE_APPICONS       0x02
#define OWB_NOTE_MENUS          0x04
#define OWBCTRLA_HoldAppObjects (OWBCTRLA_Dummy + 5)
#define OWBCTRLA_NextAppObject  (OWBCTRLA_Dummy + 6)
#define OWB_AO_APPICON  1
#define OWB_AO_APPMENU  2
#define OWB_MENU_ITEM   0
#define OWB_MENU_PARENT 1
#define OWB_MENU_TITLE  2

struct OWBAppObject {
    ULONG ao_Kind;
    APTR ao_Handle;
    ULONG ao_Find;
    ULONG ao_ID, ao_UserData;
    struct MsgPort *ao_Port;
    STRPTR ao_Text;
    APTR ao_DiskObj;
    ULONG ao_Supports;
    APTR ao_Parent;
    UBYTE ao_Role;
    UBYTE ao_CommandKey;
    UBYTE ao_Separator;
    UBYTE ao_Visible;
};

#define OWBREQ_OPEN     1
#define OWBREQ_CLOSE    2
#define OWBREQ_VISIBLE  3
#define OWBREQ_SELECT   4
#define OWBREQ_WHICH    5
#define OWBREQ_UPDATE   6               /* UpdateWorkbench: or_Name in or_Lock's drawer, or_Y the action */
#define OWBREQ_INFO     7               /* WBInfo: or_Name in or_Lock's drawer, on or_Screen */

struct OWBRequest {
    struct Message or_Msg;
    ULONG or_Call;
    CONST_STRPTR or_Name;
    struct TagItem *or_Tags;
    struct Hook *or_Hook;
    struct Window *or_Window;
    LONG or_X, or_Y;
    LONG or_Result;
    LONG or_IoErr;
    BPTR or_Lock;
    struct Screen *or_Screen;
};

/* What GFOpenBench does for the calls programs make. */
typedef struct ob_wb_env {
    /* OpenWorkbenchObjectA: a drawer or disk opened in its window, anything else as its icon says. 1 when done. */
    int (*open)(const char *path);
    /* CloseWorkbenchObjectA: the window showing that drawer closed. 1 when one was open. */
    int (*close)(const char *path);
    /* The drawer window showing dir: name in it shown (wb_drawer_show_name). 0 when no window shows dir, or no name.
     * Row 3: each drawer window is a task of its own, so these ask that task (ob_dtask.h). */
    int (*show_name)(const char *dir, const char *name);
    /* The window showing drawer path, or NULL; and each entry it shows handed to fn (wb_drawer_each), -1 when no
     * window shows path. */
    struct Window *(*window)(const char *path);
    int (*each)(const char *path, int (*fn)(void *ctx, const char *name, int type, int selected, WORD x, WORD y),
                void *ctx);
    /* AppIcons (OWB_NOTE_APPICONS) or AppMenuItems (OWB_NOTE_MENUS) came or went. */
    void (*changed)(ULONG notes);
    /* UpdateWorkbench: the drawer windows showing dir read again now ("" : the disks on the desktop). */
    void (*reread)(const char *dir);
    /* WBInfo: path's Information window. 1 when it is open. */
    int (*info)(const char *path);
} ob_wb_env;

/* A program's AppIcon, as the desktop shows it */
typedef struct ob_wb_icon {
    APTR handle;                                /* the library's, looked up again before each message */
    char label[64];
    struct DiskObject *dobj;                    /* GFOpenBench's copy (DupDiskObjectA), or NULL: the default tool's */
} ob_wb_icon;

/* A program's AppMenuItem */
typedef struct ob_wb_menu {
    APTR handle, parent;                        /* parent: its title or parent item's handle, NULL: Tools */
    char label[64];
    char key[2];                                /* its command key as GadTools wants it, or "" */
    UBYTE role, separator, visible;
} ob_wb_menu;

/* Registers GFOpenBench as workbench.library's Workbench (before the dock and WBStartup, so their AppIcons are
 * taken). 1, or 0 with no port (programs then get OS 3.2.3's no-Workbench answers). */
int ob_wb_start(const ob_wb_env *env);
/* Leaves: the calls already put are answered FALSE, the AppMessages out waited for (for a while). */
void ob_wb_stop(void);
ULONG ob_wb_sigmask(void);
/* Its signal came: the calls put are answered, the changes told (env.changed), AppMessages replied to freed. */
void ob_wb_handle(void);
/* The AppIcons, at most max, oldest first; each dobj to be freed with FreeDiskObject. How many. */
int ob_wb_icons(ob_wb_icon *out, int max);
/* The AppMenuItems, titles and parent items, at most max, in the order added. How many. */
int ob_wb_menus(ob_wb_menu *out, int max);
/* An AppMessage to the AppIcon or AppMenuItem handle (kind OWB_AO_*), if it is still there: n icons as WBArgs
 * (a drawer or disk is its own lock and an empty name, a file its drawer's lock and its name), the place (mx, my)
 * and am_Class. 1 when sent. */
int ob_wb_send(APTR handle, int kind, const char *const *paths, int n, WORD mx, WORD my, UWORD amclass);

#endif
