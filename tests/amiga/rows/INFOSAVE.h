/* INFOSAVE (N2-N7): what the Information window's Save left on RAM:WBGapN/n, judged. Plain C, for the host checks too.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
/* dos.h's protection bits, set = not allowed */
#define WBGAP_FIBF_DELETE 1L
#define WBGAP_FIBF_EXECUTE 2L
#define WBGAP_FIBF_WRITE 4L
#define WBGAP_FIBF_READ 8L
#define WBGAP_FIBF_PURE 32L   /* set = yes, as Script and Archive */
#define WBGAP_FIBF_HOLD 128L

/* N2: Deletable off, Read, Write and Execute still on */
int wbgap_infosave_prot(long prot, char *why, unsigned cap);
/* N3 N4 N5: got (NULL: none) is want; what names it in the reason */
int wbgap_infosave_same(const char *what, const char *got, const char *want, char *why, unsigned cap);
/* an icon's tool types (NULL-ended, NULL: none) as one string, comma between, cut to cap */
void wbgap_join_tooltypes(const char *const *tt, char *out, unsigned cap);
/* N6: the icon's stack after Save (got) is want */
int wbgap_infosave_stack(long got, long want, char *why, unsigned cap);
/* N7: Hold and Pure ticked: after has both set and every other bit as before */
int wbgap_infosave_hp(long before, long after, char *why, unsigned cap);
