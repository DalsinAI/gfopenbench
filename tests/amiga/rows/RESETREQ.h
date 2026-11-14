/* C5a (RESETREQ): the Workbench reset requester (amicradle-teams designs/RESETWB-REQUESTER-20261114.md). A ScreenMode
 * change made while a program's window is on the Workbench must bring up a requester; it must go once that window
 * closes, and the Workbench be reset into the new mode; Cancel on it must keep the Workbench as it is.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
struct wbgap_resetreq {
    int no_prefs;          /* ENV:Sys/screenmode.prefs not there, or no SCRM in it: SKIP */
    int shown;             /* a new window (not the Workbench's own, not ours) came up after the change */
    int gone;              /* it went once our window closed */
    int depth_before, depth_want, depth_after;   /* the Workbench's depth before, asked for, and after */
    int cancel_tried;      /* the Cancel half ran */
    int cancel_shown, cancel_gone, depth_cancel; /* its requester, gone after Cancel, the depth then */
};
int wbgap_resetreq_judge(const struct wbgap_resetreq *r, char *why, unsigned cap);
/* A screenmode.prefs' SCRM depth (big-endian UWORD at the chunk's data + 24) put to depth; the old one in *old.
 * 0 when there's no whole SCRM chunk in buf. */
int wbgap_smod_depth(unsigned char *buf, long len, int depth, int *old);
/* The depth the C5a change asks for: another the same kind of mode has (16 <-> 24 on RTG; one plane less on the
 * chipset, or one more from 1) */
int wbgap_other_depth(int depth);
