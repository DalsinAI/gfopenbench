/* WBGapAppPos: for WBGapTest APPDROP (row I8), the place on the screen of the desktop's icon named by the argument
   (the AppIcon WBGapTest made), on GFOpenBench and on OS 3.2.3's Workbench alike. Through the Workbench's ARexx port,
   window.left/top of the desktop (NAME 'root') plus the icon's .left/.top, as WBGapDesk.rexx finds its icons, and
   the icon's .width/.height: one line "x y w h" in RAM:wbgap-appicon. It waits up to 5 s for the icon to show.
   With no WORKBENCH port, or no such icon, it writes no file and APPDROP says SKIP I8.
   MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
PARSE ARG want
want = STRIP(want)
IF want = '' THEN want = 'WBGapDrop'
IF EXISTS('RAM:wbgap-appicon') THEN ADDRESS COMMAND 'Delete RAM:wbgap-appicon QUIET'
IF ~SHOW('P', 'WORKBENCH') THEN DO
  SAY 'INFO I8 no WORKBENCH port: the AppIcon not found'
  EXIT 0
END
ADDRESS WORKBENCH
GETATTR 'window.left' NAME 'root'; wl = RESULT
GETATTR 'window.top' NAME 'root'; wt = RESULT
IF wl = '' | wl = 'RESULT' THEN wl = 0
IF wt = '' | wt = 'RESULT' THEN wt = 0
DO try = 1 TO 5
  GETATTR 'window.icons.all.count' NAME 'root'; n = RESULT
  IF RC ~= 0 | n = '' | n = 'RESULT' THEN n = 0
  DO i = 0 TO n - 1
    GETATTR 'window.icons.all.'i'.name' NAME 'root'; nm = RESULT
    IF nm ~= want THEN ITERATE
    GETATTR 'window.icons.all.'i'.left' NAME 'root'; x = RESULT
    GETATTR 'window.icons.all.'i'.top' NAME 'root'; y = RESULT
    GETATTR 'window.icons.all.'i'.width' NAME 'root'; w = RESULT
    GETATTR 'window.icons.all.'i'.height' NAME 'root'; h = RESULT
    IF ~OPEN('out', 'RAM:wbgap-appicon', 'W') THEN EXIT 0
    CALL WRITELN('out', (wl + x) (wt + y) w h)
    CALL CLOSE('out')
    SAY 'INFO I8 the AppIcon' want 'at' (wl + x) (wt + y) 'size' w h
    EXIT 0
  END
  ADDRESS COMMAND 'Wait 1'
  ADDRESS WORKBENCH
END
SAY 'INFO I8 no icon' want 'on the desktop after 5 s'
EXIT 0
