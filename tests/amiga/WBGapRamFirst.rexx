/* WBGapRamFirst: row D5r (Dale, 11 Nov 15:36: "RAM should always come first on the column"), the desktop's volume
   column read through the Workbench's ARexx port, on GFOpenBench and on OS 3.2.3's Workbench alike.
     D5r  of the disk icons in the desktop's leftmost column, the topmost is RAM Disk
   It prints the column's order as an INFO line. MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
IF ~SHOW('P', 'WORKBENCH') THEN DO
  SAY 'FAIL D5r no WORKBENCH port'
  EXIT 5
END
ADDRESS WORKBENCH
GETATTR 'window.icons.all.count' NAME 'root'; n = RESULT
IF RC ~= 0 | n = '' THEN n = 0
k = 0
DO i = 0 TO n - 1
  GETATTR 'window.icons.all.'i'.type' NAME 'root'; t = UPPER(RESULT)
  IF t ~= 'DISK' THEN ITERATE
  k = k + 1
  GETATTR 'window.icons.all.'i'.name' NAME 'root'; nm.k = RESULT
  GETATTR 'window.icons.all.'i'.left' NAME 'root'; x.k = RESULT
  GETATTR 'window.icons.all.'i'.top' NAME 'root'; y.k = RESULT
  GETATTR 'window.icons.all.'i'.width' NAME 'root'; w.k = RESULT
END
IF k = 0 THEN DO
  SAY 'FAIL D5r no disk icons on the desktop'
  EXIT 5
END
/* the column: the disks whose left edge is within one icon's width of the leftmost */
minx = x.1; cw = w.1
DO j = 2 TO k
  IF x.j < minx THEN DO; minx = x.j; cw = w.j; END
END
IF cw < 16 THEN cw = 16
top = 0; order = ''
DO j = 1 TO k
  IF x.j - minx >= cw THEN ITERATE
  order = order '['y.j']' nm.j
  IF top = 0 THEN top = j
  ELSE IF y.j < y.top THEN top = j
END
SAY 'INFO D5r the column, top edge then name:' order
IF UPPER(nm.top) = 'RAM DISK' THEN SAY 'PASS D5r'
ELSE SAY 'FAIL D5r the column starts with' nm.top', not RAM Disk'
