/* INFOSAVE (N2-N5): what the Information window's Save left on RAM:WBGapN/n, judged. Plain C, for the host checks too.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
/* dos.h's protection bits, set = not allowed */
#define WBGAP_FIBF_DELETE 1L
#define WBGAP_FIBF_EXECUTE 2L
#define WBGAP_FIBF_WRITE 4L
#define WBGAP_FIBF_READ 8L

/* N2: Deletable off, Read, Write and Execute still on */
int wbgap_infosave_prot(long prot, char *why, unsigned cap);
/* N3 N4 N5: got (NULL: none) is want; what names it in the reason */
int wbgap_infosave_same(const char *what, const char *got, const char *want, char *why, unsigned cap);
/* an icon's tool types (NULL-ended, NULL: none) as one string, comma between, cut to cap */
void wbgap_join_tooltypes(const char *const *tt, char *out, unsigned cap);
