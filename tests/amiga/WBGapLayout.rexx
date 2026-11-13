/* WBGapLayout: row L1 (Dale, 13 Nov 06:43), the layout of a drawer with no snapshot, through the Workbench's
   ARexx port, on GFOpenBench and on OS 3.2.3's Workbench alike (on 3.2.3 for reference only).
   RAM:WBGapL must exist (WBGapTest LAYOUT): tools ztool and atool, projects mproj and bproj, drawers ydrawer
   and cdrawer, no icon snapshotted.
     L1a  the icons in reading order are atool, ztool, bproj, mproj, cdrawer, ydrawer
     L1b  no icon reaches past the window's inside
     L1c  L1b, and no more than one icon's room spare to the right of and below the icons
   MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
want = 'atool ztool bproj mproj cdrawer ydrawer'
IF ~SHOW('P', 'WORKBENCH') THEN DO
  SAY 'FAIL L1a no WORKBENCH port'; SAY 'SKIP L1b no WORKBENCH port'; SAY 'SKIP L1c no WORKBENCH port'
  EXIT 5
END
ADDRESS WORKBENCH
WINDOW 'RAM:WBGapL' OPEN
IF RC ~= 0 THEN DO
  SAY 'FAIL L1a RAM:WBGapL did not open'; SAY 'SKIP L1b no window'; SAY 'SKIP L1c no window'
  EXIT 5
END
ADDRESS COMMAND 'Wait 2'
ADDRESS WORKBENCH
GETATTR 'window.width' NAME 'RAM:WBGapL'; ww = RESULT
GETATTR 'window.height' NAME 'RAM:WBGapL'; wh = RESULT
GETATTR 'window.icons.all.count' NAME 'RAM:WBGapL'; n = RESULT
IF RC ~= 0 | n = '' THEN n = 0
/* the inside: the window less its borders (3.2.3's drawer window: 4 left, 18 right, 20 top, 10 bottom) */
iw = ww - 22; ih = wh - 30
SAY 'INFO L1 window' ww'x'wh', inside about' iw'x'ih',' n 'icons'
DO i = 0 TO n - 1
  GETATTR 'window.icons.all.'i'.name' NAME 'RAM:WBGapL'; nm = RESULT
  GETATTR 'window.icons.all.'i'.left' NAME 'RAM:WBGapL'; x.nm = RESULT
  GETATTR 'window.icons.all.'i'.top' NAME 'RAM:WBGapL'; y.nm = RESULT
  GETATTR 'window.icons.all.'i'.width' NAME 'RAM:WBGapL'; w.nm = RESULT
  GETATTR 'window.icons.all.'i'.height' NAME 'RAM:WBGapL'; h.nm = RESULT
  seen.nm = 1
  SAY 'INFO L1' nm 'at' x.nm','y.nm 'size' w.nm'x'h.nm
END
missing = ''
DO k = 1 TO WORDS(want)
  nm = WORD(want, k)
  IF SYMBOL('seen.nm') ~= 'VAR' THEN missing = missing nm
END
IF missing ~= '' THEN DO
  SAY 'FAIL L1a icons not in the window:' missing; SAY 'SKIP L1b icons missing'; SAY 'SKIP L1c icons missing'
  ADDRESS WORKBENCH; WINDOW 'RAM:WBGapL' CLOSE
  EXIT 5
END
/* L1a: each icon after the one before it, in reading order: a lower row, or the same row further right */
bad = ''
DO k = 2 TO WORDS(want)
  a = WORD(want, k - 1); b = WORD(want, k)
  half = h.a % 2
  IF y.b > y.a + half THEN ITERATE
  IF ABS(y.b - y.a) <= half & x.b > x.a THEN ITERATE
  bad = bad a'>'b
END
IF bad = '' THEN SAY 'PASS L1a'
ELSE SAY 'FAIL L1a out of order:' bad
/* L1b, L1c */
clip = ''; maxr = 0; maxb = 0
DO k = 1 TO WORDS(want)
  nm = WORD(want, k)
  r = x.nm + w.nm; bt = y.nm + h.nm
  IF x.nm < 0 | y.nm < 0 | r > iw | bt > ih THEN clip = clip nm
  maxr = MAX(maxr, r); maxb = MAX(maxb, bt)
  cellw = MAX(w.nm, 0); cellh = MAX(h.nm, 0)
END
IF clip = '' THEN SAY 'PASS L1b'
ELSE SAY 'FAIL L1b clipped:' clip '(inside about' iw'x'ih')'
spare_w = iw - maxr; spare_h = ih - maxb
SAY 'INFO L1 spare room' spare_w'x'spare_h
IF clip ~= '' THEN SAY 'FAIL L1c too small for its icons'
ELSE IF spare_w > 2 * cellw + 16 | spare_h > 2 * cellh + 16 THEN SAY 'FAIL L1c more than an icon''s room spare:' spare_w'x'spare_h
ELSE SAY 'PASS L1c'
ADDRESS WORKBENCH
WINDOW 'RAM:WBGapL' CLOSE
EXIT 0
