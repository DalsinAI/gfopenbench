/* WBGapDesk: row I11 as a hand does it, on GFOpenBench and on OS 3.2.3's Workbench alike: the desktop's icons
   LaunchT, p1 and p2 (left out by WBGapTest LAUNCHDESK SETUP) found through the Workbench's ARexx port, then
   DragTest clicks p1, Shift-clicks p2 and Shift-double-clicks LaunchT. WBGapTest LAUNCHDESK CHECK reads what
   LaunchT got. It prints INFO lines, and a SKIP I11 line only when it can't click (no port, no icons).
   Argument: DragTest's full path (default SYS:T/WBGap/DragTest).
   MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
PARSE ARG drag
drag = STRIP(drag)
IF drag = '' THEN drag = 'SYS:T/WBGap/DragTest'
want = 'p1 p2 LaunchT'
IF ~SHOW('P', 'WORKBENCH') THEN DO
  SAY 'SKIP I11 no WORKBENCH port to find the icons by'
  EXIT 5
END
ADDRESS WORKBENCH
GETATTR 'window.left' NAME 'root'; wl = RESULT
GETATTR 'window.top' NAME 'root'; wt = RESULT
IF wl = '' | wl = 'RESULT' THEN wl = 0
IF wt = '' | wt = 'RESULT' THEN wt = 0
triedmenu = 0
again:
/* the desktop reads RAM:.backdrop again after the disk event: up to 10 s for the icons to come.
   Each icon by its place in want (1 p1, 2 p2, 3 LaunchT), so a compound's tail is a number, not a name's case. */
DO try = 1 TO 10
  GETATTR 'window.icons.all.count' NAME 'root'; n = RESULT
  IF RC ~= 0 | n = '' THEN n = 0
  seen. = 0
  DO i = 0 TO n - 1
    GETATTR 'window.icons.all.'i'.name' NAME 'root'; nm = RESULT
    j = FIND(want, nm)   /* ARexx's FIND: 3.2.3's rexxsyslib has no WORDPOS */
    IF j = 0 THEN ITERATE
    GETATTR 'window.icons.all.'i'.left' NAME 'root'; x.j = RESULT
    GETATTR 'window.icons.all.'i'.top' NAME 'root'; y.j = RESULT
    GETATTR 'window.icons.all.'i'.width' NAME 'root'; w.j = RESULT
    GETATTR 'window.icons.all.'i'.height' NAME 'root'; h.j = RESULT
    seen.j = 1
  END
  IF seen.1 & seen.2 & seen.3 THEN LEAVE
  ADDRESS COMMAND 'Wait 1'
  ADDRESS WORKBENCH
END
/* 3.2.3's Workbench reads a .backdrop only when it starts: there, Icons > Leave Out through its own ARexx
   (GFOpenBench has no MENU INVOKE yet, and reads RAM:.backdrop again) */
IF ~(seen.1 & seen.2 & seen.3) & ~triedmenu THEN DO
  triedmenu = 1
  WINDOW 'RAM:WBGapI' OPEN
  ICON WINDOW 'RAM:WBGapI' NAMES 'LaunchT' 'p1' 'p2' SELECT
  MENU WINDOW 'RAM:WBGapI' INVOKE 'ICONS.LEAVEOUT'
  menurc = RC
  WINDOW 'RAM:WBGapI' CLOSE
  SAY 'INFO I11 not on the desktop from RAM:.backdrop; Icons > Leave Out through ARexx: RC' menurc
  IF menurc = 0 THEN SIGNAL again
END
missing = ''
DO j = 1 TO 3
  IF ~seen.j THEN missing = missing WORD(want, j)
END
IF missing ~= '' THEN DO
  SAY 'SKIP I11 not left out on the desktop (RAM:.backdrop):' STRIP(missing)
  EXIT 5
END
/* each icon clicked in its picture: across its middle, a third of the way down (above the name) */
DO j = 1 TO 3
  cx.j = wl + x.j + w.j % 2
  cy.j = wt + y.j + MAX(h.j % 3, 2)
  SAY 'INFO I11' WORD(want, j) 'at' x.j','y.j 'size' w.j'x'h.j', clicked at' cx.j','cy.j
END
ADDRESS COMMAND
'"' || drag || '" CLICK' cx.1 cy.1
'Wait 1'
'"' || drag || '" CLICK' cx.2 cy.2 'QUAL LSHIFT'
'Wait 1'
'"' || drag || '" CLICK' cx.3 cy.3 'DOUBLE QUAL LSHIFT'
'Wait 3'
SAY 'INFO I11 clicked p1, Shift-clicked p2, Shift-double-clicked LaunchT'
'Echo >RAM:wbgap-desk-clicked "clicked"'
IF triedmenu THEN DO   /* what Leave Out left out, put away again */
  ADDRESS WORKBENCH
  ICON WINDOW 'root' NAMES 'LaunchT' 'p1' 'p2' SELECT
  MENU WINDOW 'root' INVOKE 'ICONS.PUTAWAY'
END
EXIT 0
