/* ob_eject.h: Icons > Eject for any disk (Dale, 13 November 2026, item 13: "Eject works on any selected disk (DF,
 * USB, PCMCIA) from Icons > Eject"; gap row M35). Which way a selected disk's icon is ejected, from its DOS device
 * ("DF0") and the exec device its mount names (FileSysStartupMsg's fssm_Device, "trackdisk.device"). A USB stick is
 * found before this by its volume (usb_drive, GFOpenUSB's C:USBEject); everything else comes here:
 *   OB_EJECT_FLOPPY: DF0: to DF3:, through ACDisk (C:ACDisk DRIVE DFn EJECT QUIET).
 *   OB_EJECT_CARD:   a PCMCIA card (carddisk.device, compactflash.device, or a CF0:/CC0: drive): its writes flushed
 *                    and the volume let go (Inhibit on, then off), then "safe to remove".
 *   OB_EJECT_FIXED:  anything else (a hard drive, RAM:, a host drive): it isn't removable, and says so.
 * Plain C with no Amiga headers, tested on the host (tests/host/test_ob_eject.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_EJECT_H
#define OB_EJECT_H

enum { OB_EJECT_FIXED = 0, OB_EJECT_FLOPPY, OB_EJECT_CARD };

/* dev: the DOS device's name without its colon ("DF0"); exec_dev: its mount's exec device, or 0 when unknown. */
int ob_eject_kind(const char *dev, const char *exec_dev);

#endif
