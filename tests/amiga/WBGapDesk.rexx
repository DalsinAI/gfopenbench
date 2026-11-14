/* WBGapDesk: rows I3 and I11 as a hand does them, on GFOpenBench and on OS 3.2.3's Workbench alike: the desktop's icons
   LaunchT, p1 and p2 (left out by WBGapTest LAUNCHDESK SETUP) found through the Workbench's ARexx port, then
   DragTest first drags a lasso over p1 and p2 from the empty desktop (row I3: p1 and p2 selected, LaunchT not,
   read back through the port), clicks the desktop to clear it, then clicks p1, Shift-clicks p2 and
   Shift-double-clicks LaunchT. Then row I10: a click on the empty desktop, a double-click on the tool OpenT alone,
   and one on the drawer Sub, whose window must open (read through the port's windows list, then closed).
   WBGapTest LAUNCHDESK CHECK reads what LaunchT and OpenT got. It prints INFO lines, and a SKIP I11 line only when it can't click (no port, no icons).
   Argument: DragTest's full path (default SYS:T/WBGap/DragTest).
   MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
PARSE ARG drag
drag = STRIP(drag)
IF drag = '' THEN drag = 'SYS:T/WBGap/DragTest'
want = 'p1 p2 LaunchT OpenT Sub'
/* LAUNCHDESK SETUP found a window it couldn't move off the desktop (BIG's RAM:WBGapBig): a click would land on its
   icons and stop the run on a "has no default tool" requester, so nothing is clicked */
IF EXISTS('RAM:wbgap-desk-covered') THEN DO
  SAY 'SKIP I3 RAM:WBGapBig''s window stays over the desktop: no clicks'
  EXIT 5
END
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
   Each icon by its place in want (1 p1, 2 p2, 3 LaunchT, 4 OpenT, 5 Sub), so a compound's tail is a number, not a
   name's case. */
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
  IF seen.1 & seen.2 & seen.3 & seen.4 & seen.5 THEN LEAVE
  ADDRESS COMMAND 'Wait 1'
  ADDRESS WORKBENCH
END
/* 3.2.3's Workbench reads a .backdrop only when it starts: there, Icons > Leave Out through its own ARexx
   (GFOpenBench has no MENU INVOKE yet, and reads RAM:.backdrop again) */
IF ~(seen.1 & seen.2 & seen.3 & seen.4 & seen.5) & ~triedmenu THEN DO
  triedmenu = 1
  WINDOW 'RAM:WBGapI' OPEN
  ICON WINDOW 'RAM:WBGapI' NAMES 'LaunchT' 'p1' 'p2' 'OpenT' 'Sub' SELECT
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
i10 = ''   /* why I10 isn't clicked, if it isn't: CHECK reads it from RAM:wbgap-open-skip */
DO j = 4 TO 5
  IF ~seen.j THEN i10 = i10 WORD(want, j)
END
IF i10 ~= '' THEN i10 = 'not left out on the desktop (RAM:.backdrop):' STRIP(i10)
IF missing ~= '' THEN DO
  SAY 'SKIP I11 not left out on the desktop (RAM:.backdrop):' STRIP(missing)
  IF i10 = '' THEN i10 = 'I11''s icons not left out, so nothing is clicked'
  ADDRESS COMMAND 'Echo >RAM:wbgap-open-skip "' || i10 || '"'
  EXIT 5
END
/* I3 (lasso, icons only; 13 Nov): a box from the empty desktop around p1 and p2, 6 pixels out, kept off LaunchT;
   what the port says is selected then must be p1 and p2 only. Then a click on the empty desktop clears it. */
bx0 = MIN(x.1, x.2) - 6; by0 = MIN(y.1, y.2) - 6
bx1 = MAX(x.1 + w.1, x.2 + w.2) + 6; by1 = MAX(y.1 + h.1, y.2 + h.2) + 6
IF bx0 < 0 THEN bx0 = 0
IF by0 < 0 THEN by0 = 0
inbox = ''
DO j = 3 TO 5
  IF seen.j & bx0 < x.j + w.j & x.j < bx1 & by0 < y.j + h.j & y.j < by1 THEN inbox = inbox WORD(want, j)
END
IF inbox ~= '' THEN
  SAY 'SKIP I3' STRIP(inbox) 'lies where a box around p1 and p2 would go'
ELSE DO
  ADDRESS COMMAND
  '"' || drag || '" CLICK' wl + bx0 wt + by0
  'Wait 1'
  '"' || drag || '" DRAG' wl + bx0 wt + by0 wl + bx1 wt + by1 'STEPS 8'
  lrc = RC
  'Wait 1'
  ADDRESS WORKBENCH
  IF lrc ~= 0 THEN SAY 'SKIP I3 DragTest failed dragging the lasso ('lrc')'
  ELSE DO
    GETATTR 'window.icons.selected.count' NAME 'root'; ns = RESULT
    IF RC ~= 0 | ns = '' THEN ns = 0
    got = ''
    DO i = 0 TO ns - 1
      GETATTR 'window.icons.selected.'i'.name' NAME 'root'; got = got RESULT
    END
    got = STRIP(got)
    SAY 'INFO I3 lasso' wl + bx0','wt + by0 'to' wl + bx1','wt + by1': selected' ns '('got')'
    IF ns = 2 & FIND(got, 'p1') > 0 & FIND(got, 'p2') > 0 THEN SAY 'PASS I3'
    ELSE SAY 'FAIL I3 the lasso over p1 and p2 selected' ns '('got'), not p1 and p2 only'
  END
  ADDRESS COMMAND
  '"' || drag || '" CLICK' wl + bx0 wt + by0
  'Wait 1'
  ADDRESS WORKBENCH
END
/* each icon clicked in its picture: across its middle, a third of the way down (above the name) */
DO j = 1 TO 5
  IF ~seen.j THEN ITERATE
  cx.j = wl + x.j + w.j % 2
  cy.j = wt + y.j + MAX(h.j % 3, 2)
  SAY 'INFO I11' WORD(want, j) 'at' x.j','y.j 'size' w.j'x'h.j', clicked at' cx.j','cy.j
END
/* each click's RC read: a click DragTest couldn't give is SKIP, not the launcher's FAIL */
ADDRESS COMMAND
bad = ''
'"' || drag || '" CLICK' cx.1 cy.1
IF RC ~= 0 THEN bad = bad 'p1('RC')'
'Wait 1'
'"' || drag || '" CLICK' cx.2 cy.2 'QUAL LSHIFT'
IF RC ~= 0 THEN bad = bad 'p2('RC')'
'Wait 1'
'"' || drag || '" CLICK' cx.3 cy.3 'DOUBLE QUAL LSHIFT'
IF RC ~= 0 THEN bad = bad 'LaunchT('RC')'
'Wait 3'
IF bad ~= '' THEN DO   /* marked, so CHECK skips too even if LaunchT started */
  SAY 'SKIP I11 DragTest failed clicking:' STRIP(bad)
  'Echo >RAM:wbgap-desk-bad "' || STRIP(bad) || '"'
END
ELSE DO
  SAY 'INFO I11 clicked p1, Shift-clicked p2, Shift-double-clicked LaunchT'
  'Echo >RAM:wbgap-desk-clicked "clicked"'
END
/* I10: the selection cleared on the empty desktop (where the lasso began), then OpenT double-clicked alone: it runs
   with itself as its one argument; then Sub double-clicked: its window opens. The windows list read before and after,
   so a Sub window left open by an earlier run doesn't count. */
IF i10 = '' & bad = '' THEN DO
  ADDRESS WORKBENCH
  GETATTR 'windows.count'; nw0 = RESULT
  IF RC ~= 0 | nw0 = '' THEN nw0 = 0
  had = 0
  DO i = 0 TO nw0 - 1
    GETATTR 'windows.'i; wn = UPPER(RESULT)
    IF RIGHT(wn, 10) = 'WBGAPI/SUB' THEN had = 1
  END
  IF had THEN DO
    WINDOW 'RAM:WBGapI/Sub' CLOSE
    ADDRESS COMMAND 'Wait 1'
  END
  ADDRESS COMMAND
  obad = ''
  '"' || drag || '" CLICK' wl + MAX(bx0, 0) wt + MAX(by0, 0)
  IF RC ~= 0 THEN obad = obad 'desktop('RC')'
  'Wait 1'
  '"' || drag || '" CLICK' cx.4 cy.4 'DOUBLE'
  IF RC ~= 0 THEN obad = obad 'OpenT('RC')'
  'Wait 2'
  '"' || drag || '" CLICK' cx.5 cy.5 'DOUBLE'
  IF RC ~= 0 THEN obad = obad 'Sub('RC')'
  IF obad ~= '' THEN DO
    SAY 'INFO I10 DragTest failed double-clicking:' STRIP(obad)
    'Echo >RAM:wbgap-open-bad "' || STRIP(obad) || '"'
  END
  ELSE DO
    'Echo >RAM:wbgap-open-clicked "clicked"'
    /* up to 5 s for Sub's window */
    ADDRESS WORKBENCH
    found = ''
    DO try = 1 TO 5 WHILE found = ''
      ADDRESS COMMAND 'Wait 1'
      GETATTR 'windows.count'; nw = RESULT
      IF RC ~= 0 | nw = '' THEN nw = 0
      DO i = 0 TO nw - 1
        GETATTR 'windows.'i; wn = RESULT
        IF RIGHT(UPPER(wn), 10) = 'WBGAPI/SUB' THEN found = wn
      END
    END
    IF found ~= '' THEN DO
      SAY 'INFO I10 double-clicked OpenT, then Sub: window' found 'open'
      ADDRESS COMMAND 'Echo >RAM:wbgap-open-sub "' || found || '"'
      WINDOW '"' || found || '"' CLOSE
    END
    ELSE SAY 'INFO I10 double-clicked OpenT, then Sub: no window of Sub in the windows list'
  END
END
ELSE IF i10 ~= '' THEN DO
  SAY 'INFO I10 not clicked:' i10
  ADDRESS COMMAND 'Echo >RAM:wbgap-open-skip "' || i10 || '"'
END
ELSE ADDRESS COMMAND 'Echo >RAM:wbgap-open-skip "I11''s clicks failed, so I10''s weren''t given"'
ADDRESS COMMAND
IF triedmenu THEN DO   /* what Leave Out left out, put away again */
  ADDRESS WORKBENCH
  ICON WINDOW 'root' NAMES 'LaunchT' 'p1' 'p2' 'OpenT' 'Sub' SELECT
  MENU WINDOW 'root' INVOKE 'ICONS.PUTAWAY'
END
IF bad ~= '' THEN EXIT 5
EXIT 0
