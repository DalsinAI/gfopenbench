/* GFOpenBenchmark: GL through ogles3.library (OpenGL ES 3.1 on GFOpenGPU's GL.module: virgl on the
 * PC's graphics chip, or softpipe on the 68k), reached through its dispatch table as a program built
 * with ogles3's SDK reaches it (as gfopenrtg's OGLES3Check does). The library's own 640x480 window,
 * no VSYNC, five fixed scenes, each frame presented with aglSwapBuffers and each timed run ended
 * with glFinish. They are timed by the host's clock (gfb_time_host: the guest's E clock falls
 * behind while the CPU waits on GL):
 *   present   an empty frame: glClear and the swap (frames/s; its inverse is a frame's fixed cost)
 *   fill      16 full-screen quads a frame, blended, a flat colour (Mpixels/s)
 *   triangles a static mesh of 130,050 triangles in buffers, drawn 4 times a frame (Mtriangles/s)
 *   shader    one full-screen quad of a 128-step Mandelbrot at a fixed place (Mpixels/s)
 *   compute   an ES 3.1 compute dispatch: 1,048,576 invocations, 64 multiply-adds each (dispatches/s)
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include "gfb.h"
#include "ogles3_slots.h"

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef long GLintptr;
typedef long GLsizeiptr;
typedef unsigned int GLbitfield;
typedef unsigned char GLboolean;
typedef float GLfloat;

#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_FLOAT 0x1406
#define GL_UNSIGNED_SHORT 0x1403
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_COPY 0x88EA
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPUTE_SHADER 0x91B9
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_SHADER_STORAGE_BARRIER_BIT 0x00002000

#define CCT_MIN (TAG_USER + 0x4f4c0000UL)
#define CCT_VSYNC (CCT_MIN + 5)
#define CCT_GET_WIDTH (CCT_MIN + 7)
#define CCT_GET_HEIGHT (CCT_MIN + 8)
#define CCT_PS_WIDTH (CCT_MIN + 0x1002)
#define CCT_PS_HEIGHT (CCT_MIN + 0x1003)
#define CCT_PS_ES_VERSION (CCT_MIN + 0x1006)

struct dispatch {
    ULONG abiVersion, structSize, backendFlags, reserved;
    APTR entry[380];
};

static struct Library *OGLES3Base;
static const struct dispatch *D;
static void *ctx;
static ULONG W, H;

static const struct dispatch *get_dispatch(void)
{
    register const struct dispatch *d __asm("d0");
    register struct Library *a6 __asm("a6") = OGLES3Base;
    __asm__ __volatile__("jsr -30(%%a6)" : "=r"(d) : "r"(a6) : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
    return d;
}

#define CALL(slot, type) ((type)D->entry[OGL3_SLOT_##slot])
#define V0(slot) CALL(slot, void (*)(void *))(ctx)
#define V1(slot, t1, a1) CALL(slot, void (*)(void *, t1))(ctx, a1)
#define V2(slot, t1, a1, t2, a2) CALL(slot, void (*)(void *, t1, t2))(ctx, a1, a2)
#define V3(slot, t1, a1, t2, a2, t3, a3) CALL(slot, void (*)(void *, t1, t2, t3))(ctx, a1, a2, a3)
#define V4(slot, t1, a1, t2, a2, t3, a3, t4, a4) CALL(slot, void (*)(void *, t1, t2, t3, t4))(ctx, a1, a2, a3, a4)

static GLenum gl_error(void) { return CALL(GLGetError, GLenum (*)(void *))(ctx); }
static void swap(void) { CALL(aglSwapBuffers, void (*)(void *))(ctx); }
static void finish(void) { V0(GLFinish); }

static char log_text[256];

static GLuint shader(GLenum kind, const char *src)
{
    GLuint s = CALL(GLCreateShader, GLuint (*)(void *, GLenum))(ctx, kind);
    GLint ok = 0;
    CALL(GLShaderSource, void (*)(void *, GLuint, GLsizei, const char *const *, const GLint *))(ctx, s, 1, &src, NULL);
    V1(GLCompileShader, GLuint, s);
    CALL(GLGetShaderiv, void (*)(void *, GLuint, GLenum, GLint *))(ctx, s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        log_text[0] = 0;
        CALL(GLGetShaderInfoLog, void (*)(void *, GLuint, GLsizei, GLsizei *, char *))(ctx, s, sizeof log_text, NULL, log_text);
        printf("  (a shader didn't compile: %s)\n", log_text);
        return 0;
    }
    return s;
}

static GLuint program(const char *vs, const char *fs, const char *cs)
{
    GLuint p = CALL(GLCreateProgram, GLuint (*)(void *))(ctx), s;
    GLint ok = 0;
    if (cs) {
        if (!(s = shader(GL_COMPUTE_SHADER, cs))) return 0;
        V2(GLAttachShader, GLuint, p, GLuint, s);
    } else {
        if (!(s = shader(GL_VERTEX_SHADER, vs))) return 0;
        V2(GLAttachShader, GLuint, p, GLuint, s);
        if (!(s = shader(GL_FRAGMENT_SHADER, fs))) return 0;
        V2(GLAttachShader, GLuint, p, GLuint, s);
    }
    V1(GLLinkProgram, GLuint, p);
    CALL(GLGetProgramiv, void (*)(void *, GLuint, GLenum, GLint *))(ctx, p, GL_LINK_STATUS, &ok);
    if (!ok) {
        log_text[0] = 0;
        CALL(GLGetProgramInfoLog, void (*)(void *, GLuint, GLsizei, GLsizei *, char *))(ctx, p, sizeof log_text, NULL, log_text);
        printf("  (a program didn't link: %s)\n", log_text);
        return 0;
    }
    return p;
}

static GLint uniform(GLuint p, const char *name)
{
    return CALL(GLGetUniformLocation, GLint (*)(void *, GLuint, const char *))(ctx, p, name);
}

static GLuint buffer(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
    GLuint b = 0;
    V2(GLGenBuffers, GLsizei, 1, GLuint *, &b);
    V2(GLBindBuffer, GLenum, target, GLuint, b);
    V4(GLBufferData, GLenum, target, GLsizeiptr, size, const void *, data, GLenum, usage);
    return b;
}

static void attrib(GLuint buf)
{
    V2(GLBindBuffer, GLenum, GL_ARRAY_BUFFER, GLuint, buf);
    CALL(GLVertexAttribPointer, void (*)(void *, GLuint, GLint, GLenum, GLboolean, GLsizei, const void *))
        (ctx, 0, 2, GL_FLOAT, 0, 0, NULL);
    V1(GLEnableVertexAttribArray, GLuint, 0);
}

/* ---- the scenes ---------------------------------------------------------------------------------------- */

static const char vs_quad[] =
    "#version 300 es\n"
    "layout(location = 0) in vec2 p;\n"
    "out vec2 uv;\n"
    "void main() { uv = p * 0.5 + 0.5; gl_Position = vec4(p, 0.0, 1.0); }\n";
static const char fs_flat[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "uniform vec4 col;\n"
    "out vec4 o;\n"
    "void main() { o = col; }\n";
static const char fs_heavy[] =
    "#version 300 es\n"
    "precision highp float;\n"
    "in vec2 uv;\n"
    "out vec4 o;\n"
    "void main() {\n"
    "  vec2 c = vec2(-0.745, 0.186) + (uv - 0.5) * vec2(0.016, 0.012);\n"
    "  vec2 z = vec2(0.0);\n"
    "  float n = 0.0;\n"
    "  for (int i = 0; i < 128; i++) {\n"
    "    z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;\n"
    "    if (dot(z, z) > 4.0) break;\n"
    "    n += 1.0;\n"
    "  }\n"
    "  o = vec4(n / 128.0, sin(n * 0.1) * 0.5 + 0.5, cos(n * 0.07) * 0.5 + 0.5, 1.0);\n"
    "}\n";
static const char vs_mesh[] =
    "#version 300 es\n"
    "layout(location = 0) in vec2 p;\n"
    "uniform float a;\n"
    "out float s;\n"
    "void main() {\n"
    "  float c = cos(a), si = sin(a);\n"
    "  gl_Position = vec4(0.9 * (c * p.x - si * p.y), 0.9 * (si * p.x + c * p.y), 0.0, 1.0);\n"
    "  s = p.x;\n"
    "}\n";
static const char fs_mesh[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in float s;\n"
    "out vec4 o;\n"
    "void main() { o = vec4(s * 0.5 + 0.5, 0.3, 0.6, 1.0); }\n";
static const char cs_fma[] =
    "#version 310 es\n"
    "layout(local_size_x = 256) in;\n"
    "layout(std430, binding = 0) buffer B { float v[]; };\n"
    "void main() {\n"
    "  uint i = gl_GlobalInvocationID.x;\n"
    "  float x = v[i];\n"
    "  for (int k = 0; k < 64; k++) x = x * 0.999 + 0.001;\n"
    "  v[i] = x;\n"
    "}\n";

#define MESH 255                        /* quads a side: 256 x 256 vertices, the most 16-bit indices reach */
#define MESH_TRIS (2UL * MESH * MESH)   /* 130,050 */
#define MESH_DRAWS 4
#define FILL_QUADS 16
#define CS_GROUPS 4096
#define CS_ITEMS (256UL * CS_GROUPS)

static GLuint quad_buf, mesh_vbuf, mesh_ibuf, p_flat, p_heavyp, p_meshp, p_cs, cs_buf;
static GLint u_col, u_a;
static unsigned long frame_no;

static void s_finish(void *u) { (void)u; finish(); }

static void p_present(void) { }
static void w_present(void *u)
{
    (void)u;
    CALL(GLClearColor, void (*)(void *, GLfloat, GLfloat, GLfloat, GLfloat))(ctx, 0.1f, 0.2f, (frame_no++ & 1) ? 0.3f : 0.4f, 1.0f);
    V1(GLClear, GLbitfield, GL_COLOR_BUFFER_BIT);
    swap();
}

static void p_fill(void)
{
    V1(GLUseProgram, GLuint, p_flat);
    attrib(quad_buf);
    V1(GLEnable, GLenum, GL_BLEND);
    V2(GLBlendFunc, GLenum, GL_SRC_ALPHA, GLenum, GL_ONE_MINUS_SRC_ALPHA);
}
static void w_fill(void *u)
{
    int q;
    (void)u;
    for (q = 0; q < FILL_QUADS; q++) {
        CALL(GLUniform4f, void (*)(void *, GLint, GLfloat, GLfloat, GLfloat, GLfloat))(ctx, u_col, q * (1.0f / FILL_QUADS), 0.5f, 0.25f, 0.5f);
        V3(GLDrawArrays, GLenum, GL_TRIANGLE_STRIP, GLint, 0, GLsizei, 4);
    }
    swap();
}

static void p_mesh(void)
{
    V1(GLDisable, GLenum, GL_BLEND);
    V1(GLUseProgram, GLuint, p_meshp);
    attrib(mesh_vbuf);
    V2(GLBindBuffer, GLenum, GL_ELEMENT_ARRAY_BUFFER, GLuint, mesh_ibuf);
}
static void w_mesh(void *u)
{
    int d;
    (void)u;
    V1(GLClear, GLbitfield, GL_COLOR_BUFFER_BIT);
    for (d = 0; d < MESH_DRAWS; d++) {
        CALL(GLUniform1f, void (*)(void *, GLint, GLfloat))(ctx, u_a, (float)(frame_no % 628) * 0.01f + d * 0.4f);
        V4(GLDrawElements, GLenum, GL_TRIANGLES, GLsizei, (GLsizei)(MESH_TRIS * 3), GLenum, GL_UNSIGNED_SHORT, const void *, NULL);
    }
    frame_no++;
    swap();
}

static void p_heavy(void)
{
    V1(GLUseProgram, GLuint, p_heavyp);
    attrib(quad_buf);
}
static void w_heavy(void *u)
{
    (void)u;
    V3(GLDrawArrays, GLenum, GL_TRIANGLE_STRIP, GLint, 0, GLsizei, 4);
    swap();
}

/* a step is CS_BATCH dispatches and a glFinish, so the count is of work the GPU has done */
#define CS_BATCH 16
static void p_compute(void)
{
    V1(GLUseProgram, GLuint, p_cs);
    V3(GLBindBufferBase, GLenum, GL_SHADER_STORAGE_BUFFER, GLuint, 0, GLuint, cs_buf);
}
static void w_compute(void *u)
{
    int i;
    (void)u;
    for (i = 0; i < CS_BATCH; i++) {
        V3(GLDispatchCompute, GLuint, CS_GROUPS, GLuint, 1, GLuint, 1);
        V1(GLMemoryBarrier, GLbitfield, GL_SHADER_STORAGE_BARRIER_BIT);
    }
    finish();
}

static void scene(const char *key, const char *label, const char *unit, void (*prep)(void), gfb_step w, double scale, int ready)
{
    struct gfb_result r;
    GLenum e;
    memset(&r, 0, sizeof r);
    r.key = key;
    r.label = label;
    r.unit = unit;
    if (!ready) gfb_fail(&r, "its shaders or buffers weren't made");
    else {
        prep();
        gfb_time_host(&r, w, s_finish, NULL, scale);
        if ((e = gl_error())) sprintf(r.note + strlen(r.note), " (GL error 0x%x)", (unsigned)e);
    }
    gfb_report(&r);
}

static void make_mesh(void)
{
    ULONG nv = (MESH + 1) * (MESH + 1), ni = MESH_TRIS * 3, i, x, y;
    float *v = AllocMem(nv * 8, MEMF_ANY);
    UWORD *ix = AllocMem(ni * 2, MEMF_ANY);
    if (v && ix) {
        for (y = 0, i = 0; y <= MESH; y++)
            for (x = 0; x <= MESH; x++) {
                v[i++] = (float)x * (2.0f / MESH) - 1.0f;
                v[i++] = (float)y * (2.0f / MESH) - 1.0f;
            }
        for (y = 0, i = 0; y < MESH; y++)
            for (x = 0; x < MESH; x++) {
                UWORD a = (UWORD)(y * (MESH + 1) + x), b = (UWORD)(a + 1), c = (UWORD)(a + MESH + 1), d = (UWORD)(c + 1);
                ix[i++] = a; ix[i++] = b; ix[i++] = c;
                ix[i++] = b; ix[i++] = d; ix[i++] = c;
            }
        mesh_vbuf = buffer(GL_ARRAY_BUFFER, (GLsizeiptr)(nv * 8), v, GL_STATIC_DRAW);
        mesh_ibuf = buffer(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(ni * 2), ix, GL_STATIC_DRAW);
    }
    if (v) FreeMem(v, nv * 8);
    if (ix) FreeMem(ix, ni * 2);
}

void gfb_gl_tests(void)
{
    static const float quad[8] = { -1, -1, 1, -1, -1, 1, 1, 1 };
    ULONG err = 0;
    char buf[200];
    double px;
    void *zero;

    gfb_section("GL (ogles3.library, its own 640x480 window, no VSYNC)");
    if (!(OGLES3Base = OpenLibrary((CONST_STRPTR) "ogles3.library", 1))) {
        strcpy(gfb_gl_renderer, "none: no ogles3.library");
        printf("  not measured: no ogles3.library\n");
        return;
    }
    D = get_dispatch();
    if (!D || D->abiVersion != 1 || D->structSize < sizeof(struct dispatch)) {
        strcpy(gfb_gl_renderer, "none: ogles3.library of another ABI");
        printf("  not measured: ogles3.library's dispatch table is of another ABI\n");
        CloseLibrary(OGLES3Base);
        return;
    }
    {
        struct TagItem tags[] = {
            { CCT_PS_ES_VERSION, 31 }, { CCT_VSYNC, 0 }, { CCT_PS_WIDTH, 640 }, { CCT_PS_HEIGHT, 480 },
            { CCT_GET_WIDTH, (ULONG)&W }, { CCT_GET_HEIGHT, (ULONG)&H }, { TAG_DONE, 0 },
        };
        ctx = CALL(aglCreateContext2, void *(*)(ULONG *, struct TagItem *))(&err, tags);
    }
    if (!ctx) {
        sprintf(gfb_gl_renderer, "none: aglCreateContext2 failed (error %lu)", (unsigned long)err);
        printf("  not measured: %s\n", gfb_gl_renderer);
        CloseLibrary(OGLES3Base);
        return;
    }
    CALL(aglMakeCurrent, void (*)(void *))(ctx);
    {
        const char *s = (const char *)CALL(GLGetString, const unsigned char *(*)(void *, GLenum))(ctx, GL_RENDERER);
        unsigned n = s ? strlen(s) : 0;
        if (n > sizeof gfb_gl_renderer - 1) n = sizeof gfb_gl_renderer - 1;
        memcpy(gfb_gl_renderer, s ? s : "", n);
        gfb_gl_renderer[n] = 0;
    }
    snprintf(buf, sizeof buf, "%s", (const char *)CALL(GLGetString, const unsigned char *(*)(void *, GLenum))(ctx, GL_VERSION));
    gfb_info("gl_version", "GL version", buf);
    gfb_info("gl_renderer", "GL renderer", gfb_gl_renderer);
    sprintf(buf, "%lux%lu", (unsigned long)W, (unsigned long)H);
    gfb_info("gl_size", "GL window", buf);
    px = (double)W * H;
    V4(GLViewport, GLint, 0, GLint, 0, GLsizei, (GLsizei)W, GLsizei, (GLsizei)H);

    quad_buf = buffer(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);
    p_flat = program(vs_quad, fs_flat, NULL);
    if (p_flat) u_col = uniform(p_flat, "col");
    p_heavyp = program(vs_quad, fs_heavy, NULL);
    p_meshp = program(vs_mesh, fs_mesh, NULL);
    if (p_meshp) u_a = uniform(p_meshp, "a");
    make_mesh();
    p_cs = program(NULL, NULL, cs_fma);
    if (p_cs && (zero = AllocMem(CS_ITEMS * 4, MEMF_ANY | MEMF_CLEAR))) {
        cs_buf = buffer(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)(CS_ITEMS * 4), zero, GL_DYNAMIC_COPY);
        FreeMem(zero, CS_ITEMS * 4);
    }
    finish();
    if ((err = gl_error())) printf("  (GL error 0x%lx setting the scenes up)\n", (unsigned long)err);

    scene("gl_present_fps", "Present (empty frames)", "frames/s", p_present, w_present, 1.0, 1);
    scene("gl_fill_mpix", "Fill, 16 blended quads a frame", "Mpixels/s", p_fill, w_fill, FILL_QUADS * px / 1e6, p_flat != 0);
    scene("gl_tris_mtri", "Triangles, 130,050 x 4 a frame", "Mtris/s", p_mesh, w_mesh, MESH_DRAWS * MESH_TRIS / 1e6,
          p_meshp && mesh_vbuf && mesh_ibuf);
    scene("gl_shader_fps", "Shader, 128-step Mandelbrot", "frames/s", p_heavy, w_heavy, 1.0, p_heavyp != 0);
    scene("gl_compute_dps", "Compute, 1M invocations x 64 FMA", "dispatches/s", p_compute, w_compute, CS_BATCH, p_cs && cs_buf);

    CALL(aglDestroyContext, void (*)(void *))(ctx);
    ctx = NULL;
    CloseLibrary(OGLES3Base);
    OGLES3Base = NULL;
}
