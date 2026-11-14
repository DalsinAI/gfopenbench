/* W2, keyboard window cycling: GFOpenWindows' switcher (gfopenprefs Windows), Amiga+Tab brings the window at the back
 * of the screen to the front and activates it, Amiga+Shift+Tab sends the front one to the back and activates the one
 * behind it. ENV:GFOpenPrefs/Windows (text, may be NULL): 0 when it says "switcher off", else 1; its "switcher.key"
 * into key (cap bytes), "lcommand tab" when it has none. MIT, Copyright (c) 2026 AmiCradle Team. */
/* W2: a window Amiga+Tab cycles, by its Intuition Flags and size (GFOpenWindows 0.12.4, gfopenprefs
 * Windows/src/switch_rule.h): not a backdrop, at least 8x8, not Workbench's root (a Workbench window without a close
 * gadget), and with a system gadget (drag bar, depth, close or size), so tray buttons and panels don't take part */
int wbgap_switch_offers(unsigned long flags, int width, int height);
int wbgap_switcher_on(const char *env, char *key, unsigned cap);
/* the step judged from the screen's switchable windows front to back before (nb of them) and after (na), and the
 * active window after (ids, any unique numbers): back 0 for Amiga+Tab, 1 for Amiga+Shift+Tab. WBGAP_PASS,
 * WBGAP_FAIL (the reason in why) or WBGAP_SKIP (fewer than two windows) */
int wbgap_switch_judge(int back, const unsigned long *before, int nb, const unsigned long *after, int na,
                       unsigned long active, char *why, unsigned cap);
