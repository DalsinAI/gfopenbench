/* ob_menustate: which of the desktop's menu items can act now (GF-127, Dale 23 Nov 2026: "menu options that are not
 * available over what your currently above dont work ... icons options are available on bits of openbench that have
 * no right being enabled"). An item is enabled only when picking it would do something: Icons > Information with no
 * icon selected, Put Away with no icon left out, Format with no disk, and the like, are ghosted, as Workbench ghosts
 * them. The same state goes to the menu at the pointer (0.1.73), which shows only enabled items.
 * Plain C, no Amiga headers: tests/host/test_ob_menustate.c.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_MENUSTATE_H
#define OB_MENUSTATE_H

/* What the desktop has now. The counts are of the selected desktop icons unless named otherwise. */
typedef struct {
    int sel;            /* selected icons of any kind */
    int sel_file;       /* selected icons with a file behind them: disks and icons left out (not AppIcons) */
    int disks;          /* selected disks */
    int louts;          /* selected icons left out */
    int louts_temp;     /* selected icons left out for this run only (not in .backdrop yet) */
    int recover;        /* selected disks, or a left-out trashcan, with deleted files to look at */
    int recover_open;   /* the Recover deleted window is open (Put back acts on the file picked there) */
    int shown;          /* icons on the desktop that can be seen and picked (none while No icons is on) */
    int files;          /* icons on the desktop with a file behind them (Snapshot > All has something to keep) */
    int all_disks;      /* disks on the desktop (Open volume... has a list to show) */
} ob_ms_facts;

/* What an item needs to act */
enum {
    OB_MS_ALWAYS = 0,   /* the desktop's own commands: New Shell, Find..., About... */
    OB_MS_SEL,          /* any icon selected: Open, Clear selection */
    OB_MS_SEL_FILE,     /* an icon with a file: Information..., Rename..., Snapshot, Unsnapshot */
    OB_MS_DISK,         /* a disk selected: Eject, Format Disk... */
    OB_MS_LOUT,         /* an icon left out selected: Put Away, Delete... */
    OB_MS_LOUT_TEMP,    /* an icon left out for this run selected: Leave Out */
    OB_MS_RECOVER,      /* a disk or a left-out trashcan: Recover deleted... */
    OB_MS_PUTBACK,      /* the Recover deleted window open: Put back */
    OB_MS_SHOWN,        /* icons to select: Select contents */
    OB_MS_FILES,        /* icons to keep places for: Snapshot > All, Clean Up */
    OB_MS_ANY_DISK      /* disks on the desktop: Open volume... */
};

/* 1 when an item needing `need` can act with these facts */
int ob_ms_enabled(const ob_ms_facts *f, int need);

#endif
