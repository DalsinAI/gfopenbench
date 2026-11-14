/* ob_info_ids: the gadget ids of GFOpenBench's Information window (ob_info.c). WBGapTest INFOSAVE
 * (tests/amiga) finds the window's gadgets by these ids to drive it as a hand does, so both include this.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_INFO_IDS_H
#define OB_INFO_IDS_H

enum { G_NAME = 1, G_TYPE, G_SIZE, G_DATE, G_PS, G_PA, G_PR, G_PW, G_PE, G_PD, G_COMMENT, G_TOOL, G_TTLIST, G_TTSTR,
       G_TTNEW, G_TTDEL, G_SAVE, G_CANCEL, G_SIZE2, G_STACK, G_ADV, G_HOLD, G_PURE, G_COUNT };

#endif
