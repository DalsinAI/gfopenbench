/* GFOpenBench's About window (Project > About...): GFOpenLayout's olabout (gfopenlayout_about.h), with the GFOS mark
 * (Team 5, 14 November 2026); the name, version and lines here, the same on the EasyRequest it falls back to when
 * GFOpenBench is built without GFOpenLayout or the window can't open. Plain C (tests/host/test_ob_about.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_ABOUT_H
#define OB_ABOUT_H

#define OB_ABOUT_NAME "GFOpenBench"
#define OB_ABOUT_TEXT "Our own Workbench for AmigaOS 3.2.\nMIT, Copyright (c) 2026 AmiCradle Team."
/* the fallback EasyRequest's text: the name and version, a blank line, the lines and the Team's credit */
#define OB_ABOUT_REQ(version) OB_ABOUT_NAME " " version "\n\n" OB_ABOUT_TEXT \
    "\nDesigned, tested and delivered by The AmiCradle Team."

#endif
