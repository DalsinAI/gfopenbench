/* WBGapRoot: the desktop's icons, for WBGapTest DISKS (row D5, the RAM Disk's icon), on GFOpenBench and on OS 3.2.3's
   Workbench alike. Through the Workbench's ARexx port, each icon of the desktop (NAME 'root') as one line
   "<type> <name>" (window.icons.all.N.type and .name) in RAM:wbgap-root. It prints only INFO lines; WBGapTest DISKS
   says PASS, FAIL or SKIP. With no WORKBENCH port it writes no file, and DISKS says SKIP.
   Arguments (both optional): the row its INFO lines name (D5) and the file (RAM:wbgap-root); WBGapTest DISKIO (row D1)
   runs it as "WBGapRoot.rexx D1 RAM:wbgap-root-d1".
   For row D5r (RAM Disk first in the volume column, Dale 11 Nov) the D5 run also writes each disk icon's place to
   RAM:wbgap-root-pos, "<left> <top> <width> <name>" a line; DISKS judges it (wbgap_ram_first). The D1 run doesn't.
   MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
PARSE ARG row out .
IF row = '' THEN row = 'D5'
IF out = '' THEN out = 'RAM:wbgap-root'
IF EXISTS(out) THEN ADDRESS COMMAND 'Delete' out 'QUIET'
IF row = 'D5' & EXISTS('RAM:wbgap-root-pos') THEN ADDRESS COMMAND 'Delete RAM:wbgap-root-pos QUIET'
IF ~SHOW('P', 'WORKBENCH') THEN DO
  SAY 'INFO' row 'no WORKBENCH port: the desktop''s icons not read'
  EXIT 0
END
ADDRESS WORKBENCH
GETATTR 'window.icons.all.count' NAME 'root'; n = RESULT
IF RC ~= 0 | n = '' | n = 'RESULT' THEN n = 0
IF ~OPEN('out', out, 'W') THEN DO
  SAY 'INFO' row out 'not made'
  EXIT 0
END
pos = 0
IF row = 'D5' THEN pos = OPEN('pos', 'RAM:wbgap-root-pos', 'W')
DO i = 0 TO n - 1
  GETATTR 'window.icons.all.'i'.type' NAME 'root'; t = RESULT
  GETATTR 'window.icons.all.'i'.name' NAME 'root'; nm = RESULT
  CALL WRITELN('out', UPPER(t) nm)
  IF pos & UPPER(t) = 'DISK' THEN DO
    GETATTR 'window.icons.all.'i'.left' NAME 'root'; x = RESULT
    GETATTR 'window.icons.all.'i'.top' NAME 'root'; y = RESULT
    GETATTR 'window.icons.all.'i'.width' NAME 'root'; w = RESULT
    CALL WRITELN('pos', x y w nm)
  END
END
CALL CLOSE('out')
IF pos THEN CALL CLOSE('pos')
SAY 'INFO' row 'the desktop has' n 'icons'
EXIT 0
