#include "electron_fx.h"

#include <SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   Simulation and appearence parameters
   ============================================================ */
enum {
    TRACE_W_PX        = 1,
    TRACE_ALPHA       = 32,
    MASK_LIGHT_A      = 22,
    MASK_DARK_A       = 18,
    PAD_RADIUS_PX     = 5,
    PAD_DRILL_PX      = 2,

    GRID_W            = 38,
    GRID_H            = 22,

    EDGE_PORTS_PER_SIDE = 6,

    DENSITY_BUSES     = 3,
    DENSITY_SPINES    = 6,
    DENSITY_DIFFPAIRS = 4,
    DENSITY_MEANDERS  = 4,
    DENSITY_ZONES     = 3,

    MAX_BAL_ITER      = 28,

    DOT_SIZE          = 10,
    ELECTRON_COUNT    = 82,

    TRAIL_FADE_ALPHA  = 14
};

static const float HV_BAL_TOL_PX = 0.02f;
static const float SPEED_MIN = 140.0f, SPEED_MAX = 260.0f;
static const float PAD_DWELL_MIN = 0.03f, PAD_DWELL_MAX = 0.10f;

static const SDL_Color COL_COPPER  = {66, 66, 66, 255};
static const SDL_Color COL_MASK_L  = {120,120,120,255};
static const SDL_Color COL_MASK_D  = {20, 20, 20, 255};

/* ============================================================
   Internal data types
   ============================================================ */
typedef struct { int gx, gy; } GPoint;

typedef struct {
    int count;
    GPoint p[200];
} TraceDef;

typedef struct { float x, y; } Pt;

typedef struct {
    int   count;
    Pt    p[600];
    float seg[599];
    float cum[600];
    float total;
} Poly;

typedef struct {
    int        poly_idx;
    float      dist;
    float      speed;
    float      dwell;
    SDL_Color  color;
} Electron;

struct fx_electron_t {
    int W, H;

    /* Escena */
    TraceDef* traces;  int traceCount;
    GPoint*   pads;    int padCount;
    Poly*     polys;

    /* Texturas */
    SDL_Texture* board;       /* prerender (bg + cobre + máscara + pads) */
    SDL_Texture* trails;      /* target para trails (fade acumulativo)   */
    SDL_Texture* electron_tex;/* sprite (PNG) o fallback radial          */

    /* Electrones */
    Electron* E;
};

/* ============================================================
   Utilidades
   ============================================================ */
static inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline int   clampi(int v, int lo, int hi)       { return v < lo ? lo : (v > hi ? hi : v); }
static inline float lerpf(float a, float b, float t)    { return a + (b - a) * t; }
static inline float hypotf_s(float x, float y)          { return sqrtf(x*x + y*y); }
static inline float frand(void)                         { return (float)(rand() % 10000) / 10000.0f; }
static inline int   ri(int a, int b)                    { return a + rand() % ((b - a) + 1); }

static void cell_size(int W, int H, float* cx, float* cy) {
    /* márgenes para que el “cobre” no toque bordes */
    const float mx = 0.055f, my = 0.095f;
    const float sx = 1.0f - 2*mx, sy = 1.0f - 2*my;
    *cx = (sx * W) / (GRID_W - 1);
    *cy = (sy * H) / (GRID_H - 1);
}

static void grid_to_screen(int W,int H,int gx,int gy,float* x,float* y) {
    const float mx = 0.055f, my = 0.095f;
    const float sx = 1.0f - 2*mx, sy = 1.0f - 2*my;
    *x = (mx + (float)gx/(GRID_W-1) * sx) * W;
    *y = (my + (float)gy/(GRID_H-1) * sy) * H;
}

/* ¿el segmento es horizontal o vertical (ejes)? */
static int is_axis_segment(Pt a, Pt b) {
    return (fabsf(a.y - b.y) < 0.5f) || (fabsf(a.x - b.x) < 0.5f);
}

/* ============================================================
   Generadores de trazas (grid -> luego se convierte a polígonos)
   ============================================================ */
static void push_gpt(TraceDef* t, int gx, int gy) {
    if (t && t->count < (int)(sizeof(t->p)/sizeof(t->p[0])))
        t->p[t->count++] = (GPoint){gx, gy};
}

static TraceDef make_bus_row(int gy) {
    TraceDef t = {0};
    int x = -2;
    push_gpt(&t, x, gy); x = 0; push_gpt(&t, x, gy);
    while (x < GRID_W + 1) {
        int jog = (frand() < 0.14f) ? (frand() < 0.5f ? -1 : +1) : 0;
        int nx  = x + ri(4, 7);
        if (nx > GRID_W + 1) nx = GRID_W + 1;
        if (jog) push_gpt(&t, x, clampi(gy + jog, 0, GRID_H - 1));
        push_gpt(&t, nx, clampi(gy + jog, 0, GRID_H - 1));
        x = nx;
    }
    return t;
}

static TraceDef make_spine_col(int gx) {
    TraceDef t = {0};
    int y = -2;
    push_gpt(&t, gx, y); y = 0; push_gpt(&t, gx, y);
    while (y < GRID_H + 1) {
        int jog = (frand() < 0.18f) ? (frand() < 0.5f ? -1 : +1) : 0;
        int ny  = y + ri(4, 7);
        if (ny > GRID_H + 1) ny = GRID_H + 1;
        if (jog) push_gpt(&t, clampi(gx + jog, 0, GRID_W - 1), y);
        push_gpt(&t, clampi(gx + jog, 0, GRID_W - 1), ny);
        y = ny;
    }
    return t;
}

static void add_hdiffpair(TraceDef* A, TraceDef* B, int gy) {
    *A = make_bus_row(gy);
    *B = make_bus_row(clampi(gy + 1, 0, GRID_H - 1));
}
static void add_vdiffpair(TraceDef* A, TraceDef* B, int gx) {
    *A = make_spine_col(gx);
    *B = make_spine_col(clampi(gx + 1, 0, GRID_W - 1));
}

static TraceDef make_meander_h(int x0,int y0,int w,int h){
    TraceDef t = {0};
    int dir = 1;
    for (int y = y0; y <= y0 + h; y += 2) {
        int xs = x0, xe = x0 + w;
        if (dir < 0) { int tmp = xs; xs = xe; xe = tmp; }
        push_gpt(&t, xs, y); push_gpt(&t, xe, y);
        if (y < y0 + h) push_gpt(&t, xe, y + 1);
        dir *= -1;
    }
    return t;
}
static TraceDef make_meander_v(int x0,int y0,int w,int h){
    TraceDef t = {0};
    int dir = 1;
    for (int x = x0; x <= x0 + w; x += 2) {
        int ys = y0, ye = y0 + h;
        if (dir < 0) { int tmp = ys; ys = ye; ye = tmp; }
        push_gpt(&t, x, ys); push_gpt(&t, x, ye);
        if (x < x0 + w) push_gpt(&t, x + 1, ye);
        dir *= -1;
    }
    return t;
}

static void measure_HV_screen(const TraceDef* t,int W,int H,float cellx,float celly,double* Hp,double* Vp){
    for (int i = 0; i < t->count - 1; i++) {
        int dx = t->p[i+1].gx - t->p[i].gx;
        int dy = t->p[i+1].gy - t->p[i].gy;
        *Hp += fabs((double)dx) * (double)cellx;
        *Vp += fabs((double)dy) * (double)celly;
    }
}

/* Construye una escena pseudo-aleatoria en grid y devuelve punteros a buffers estáticos
   (luego el caller los copia a memoria propia). */
static void build_scene(int W,int H, TraceDef** outTr,int* outCnt, GPoint** outPads,int* outPCnt){
    static TraceDef traces[1200];
    static GPoint pads[2400];
    int tCount = 0, pCount = 0;

    /* Puertos desde bordes hacia el interior (como conectores externos) */
    for (int i=0; i<EDGE_PORTS_PER_SIDE; i++){
        int gx = ri(0, GRID_W-1);
        TraceDef s = (TraceDef){0};
        push_gpt(&s, gx, -2); push_gpt(&s, gx, ri(0, 2)); traces[tCount++] = s;

        gx = ri(0, GRID_W-1); s = (TraceDef){0};
        push_gpt(&s, gx, GRID_H+1); push_gpt(&s, gx, GRID_H - ri(0,2)); traces[tCount++] = s;

        int gy = ri(0, GRID_H-1);
        s = (TraceDef){0}; push_gpt(&s, -2, gy); push_gpt(&s, ri(0,2), gy); traces[tCount++] = s;

        gy = ri(0, GRID_H-1);
        s = (TraceDef){0}; push_gpt(&s, GRID_W+1, gy); push_gpt(&s, GRID_W - ri(0,2), gy); traces[tCount++] = s;
    }

    /* Buses y spines principales */
    for (int i=0; i<DENSITY_BUSES;  i++){ int gy = clampi(ri(1, GRID_H-2), 1, GRID_H-2); traces[tCount++] = make_bus_row(gy); }
    for (int i=0; i<DENSITY_SPINES; i++){ int gx = clampi(ri(1, GRID_W-2), 1, GRID_W-2); traces[tCount++] = make_spine_col(gx); }

    /* Pares diferenciales (H/V) */
    for (int i=0; i<DENSITY_DIFFPAIRS; i++){
        if (i % 2 == 0){ TraceDef A,B; add_hdiffpair(&A,&B, clampi(ri(3,GRID_H-4), 3, GRID_H-4)); traces[tCount++]=A; traces[tCount++]=B; }
        else            { TraceDef A,B; add_vdiffpair(&A,&B, clampi(ri(3,GRID_W-4), 3, GRID_W-4)); traces[tCount++]=A; traces[tCount++]=B; }
    }

    /* Meandros (típicos para igualar longitudes) */
    for (int i=0; i<DENSITY_MEANDERS; i++){
        int w=ri(4,7), h=ri(3,4);
        int x0=ri(2, GRID_W-w-3), y0=ri(2, GRID_H-h-3);
        traces[tCount++] = (i%2==0) ? make_meander_h(x0,y0,w,h) : make_meander_v(x0,y0,w,h);
    }

    /* Pequeñas “islas” de pads (vías o pads de componentes) */
    for (int i=0; i<DENSITY_ZONES; i++){
        int cx=ri(3,GRID_W-4), cy=ri(3,GRID_H-4);
        pads[pCount++] = (GPoint){cx,cy};
        pads[pCount++] = (GPoint){cx+1,cy};
        pads[pCount++] = (GPoint){cx,cy+1};
        pads[pCount++] = (GPoint){cx+1,cy+1};
    }

    /* Balance horizontal/vertical para que el patrón se vea uniforme. */
    float cellx, celly; cell_size(W,H,&cellx,&celly);
    int iter=0;
    while (iter++ < MAX_BAL_ITER){
        double Hpx=0.0, Vpx=0.0;
        for (int i=0; i<tCount; i++) measure_HV_screen(&traces[i], W,H, cellx,celly, &Hpx,&Vpx);
        double tot = Hpx + Vpx; if (tot <= 1.0) break;
        double diff = fabs(Hpx - Vpx) / tot;
        if (diff <= HV_BAL_TOL_PX) break;

        if (Hpx > Vpx){
            int gx = clampi(ri(1, GRID_W-2), 1, GRID_W-2);
            traces[tCount++] = make_spine_col(gx);
        } else {
            int gy = clampi(ri(1, GRID_H-2), 1, GRID_H-2);
            traces[tCount++] = make_bus_row(gy);
        }
    }

    *outTr   = traces; *outCnt  = tCount;
    *outPads = pads;   *outPCnt = pCount;
}

/* ============================================================
   Conversión de grid a polígonos (trayectorias rectangulares)
   ============================================================ */
static Poly build_poly(int W, int H, const TraceDef* tr){
    Poly out = {0};
    if (!tr || tr->count < 2) return out;

    /* 1) grid -> puntos de pantalla */
    for (int i=0; i<tr->count; i++){
        float x,y; grid_to_screen(W,H, tr->p[i].gx, tr->p[i].gy, &x,&y);
        out.p[out.count++] = (Pt){x,y};
    }

    /* 2) Insertar codos a 90° si el segmento no es axial (H/V).
          Esto elimina diagonales y “puntas” agudas. */
    Pt tmp[600]; int n2 = 0;
    for (int i=0; i<out.count-1; i++){
        Pt A = out.p[i], B = out.p[i+1];
        tmp[n2++] = A;
        if (!is_axis_segment(A,B)){
            Pt M1 = (Pt){A.x, B.y}; /* codo tipo └/┘ */
            tmp[n2++] = M1;
        }
    }
    tmp[n2++] = out.p[out.count-1];

    /* 3) Reescribir y precomputar longitudes acumuladas */
    out.count = 0;
    for (int i=0; i<n2; i++) out.p[out.count++] = tmp[i];
    out.total = 0.0f;
    if (out.count > 0) out.cum[0] = 0.0f;
    for (int i=0; i<out.count-1; i++){
        float L = hypotf_s(out.p[i+1].x - out.p[i].x, out.p[i+1].y - out.p[i].y);
        out.seg[i] = L;
        out.total += L;
        out.cum[i+1] = out.total;
    }
    return out;
}

/* ============================================================
   Render helpers
   ============================================================ */
static void draw_bg(SDL_Renderer* r,int W,int H){
    /* fondo muy sutil verdoso (no uniforme) */
//    const Uint8 top[3]={8,10,8}, bot[3]={6,8,6};
    const Uint8 top[3]={0,0,0}, bot[3]={0,0,0};
    for (int y=0; y<H; y++){
        float t=(float)y/(float)(H-1);
        Uint8 R=(Uint8)lerpf(top[0],bot[0],t);
        Uint8 G=(Uint8)lerpf(top[1],bot[1],t);
        Uint8 B=(Uint8)lerpf(top[2],bot[2],t);
        SDL_SetRenderDrawColor(r, R,G,B, 255);
        SDL_RenderDrawLine(r, 0,y, W,y);
    }
}

/* Dibuja una “pista” rectangular (con codo a 90° si es diagonal). */
static void draw_line_rect(SDL_Renderer* r, Pt a, Pt b, int w, SDL_Color c, Uint8 alpha){
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, c.r,c.g,c.b, alpha);

    if (fabsf(a.y - b.y) < 0.5f){ /* horizontal */
        int x1=(int)fminf(a.x,b.x), x2=(int)fmaxf(a.x,b.x);
        int y =(int)a.y - w/2;
        SDL_Rect rr = { x1, y, x2-x1+1, w };
        SDL_RenderFillRect(r, &rr);
        return;
    }
    if (fabsf(a.x - b.x) < 0.5f){ /* vertical */
        int y1=(int)fminf(a.y,b.y), y2=(int)fmaxf(a.y,b.y);
        int x =(int)a.x - w/2;
        SDL_Rect rr = { x, y1, w, y2-y1+1 };
        SDL_RenderFillRect(r, &rr);
        return;
    }

    /* codo: primero horizontal, luego vertical */
    Pt m = (Pt){a.x, b.y};
    SDL_Rect r1 = { (int)fminf(a.x,m.x), (int)(a.y - w/2), (int)fabsf(m.x - a.x)+1, w };
    SDL_RenderFillRect(r, &r1);
    int y1=(int)fminf(m.y,b.y), y2=(int)fmaxf(m.y,b.y);
    int x =(int)b.x - w/2;
    SDL_Rect r2 = { x, y1, w, y2-y1+1 };
    SDL_RenderFillRect(r, &r2);
}

static void draw_pad(SDL_Renderer* r, Pt c){
    SDL_Color ring = {90,90,90,255};
    SDL_Color hole = {0,0,0,255};
    int R = PAD_RADIUS_PX;
    int x = (int)c.x, y = (int)c.y;

    /* anillo */
    for (int i=-R; i<=R; i++){
        for (int j=-R; j<=R; j++){
            int d=i*i + j*j;
            if (d >= (R-1)*(R-1) && d <= R*R){
                SDL_SetRenderDrawColor(r, ring.r, ring.g, ring.b, TRACE_ALPHA+12);
                SDL_RenderDrawPoint(r, x+i, y+j);
            }
        }
    }
    /* taladro */
    int r0 = PAD_DRILL_PX;
    for (int i=-r0; i<=r0; i++){
        for (int j=-r0; j<=r0; j++){
            if (i*i + j*j <= r0*r0){
                SDL_SetRenderDrawColor(r, hole.r, hole.g, hole.b, 230);
                SDL_RenderDrawPoint(r, x+i, y+j);
            }
        }
    }
}

static SDL_Texture* make_radial_sprite(SDL_Renderer* r, int size){
    SDL_Surface* s = SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
    if (!s) return NULL;

    Uint32* p = (Uint32*)s->pixels;
    int pitch = s->pitch / 4;

    float R = size/2.0f, R2 = R*R;
    for (int y=0; y<size; y++){
        for (int x=0; x<size; x++){
            float dx = x - R + 0.5f, dy = y - R + 0.5f;
            float d2 = dx*dx + dy*dy;
            float a = 0.0f;
            if (d2 <= R2){
                float d = sqrtf(d2) / R;
                a = powf(1.0f - d, 2.0f); /* radial suave */
            }
            Uint8 A = (Uint8)(a * 255.0f);
            p[y*pitch + x] = SDL_MapRGBA(s->format, 255, 255, 255, A);
        }
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);
    if (!tex) return NULL;
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
    return tex;
}

static SDL_Texture* load_png_texture(SDL_Renderer* r, const char* path){
    if (!path) return NULL;
    SDL_Texture* tex = IMG_LoadTexture(r, path);
    if (!tex) SDL_Log("IMG_LoadTexture('%s') failed: %s", path, IMG_GetError());
    return tex;
}

static SDL_Texture* build_board(SDL_Renderer* r, int W, int H,
                                const TraceDef* traces, int traceCount,
                                const GPoint* pads, int padCount,
                                Poly* out_polys)
{
    SDL_Texture* tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, W, H);
    if (!tex) return NULL;

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    if (SDL_SetRenderTarget(r, tex) != 0){
        SDL_DestroyTexture(tex);
        return NULL;
    }

    /* fondo */
    draw_bg(r, W, H);

    /* cobre + máscara */
    for (int i=0; i<traceCount; i++){
        out_polys[i] = build_poly(W, H, &traces[i]);
        const Poly* P = &out_polys[i];
        for (int k=0; k<P->count-1; k++){
            Pt a = P->p[k], b = P->p[k+1];

            /* cobre principal */
            draw_line_rect(r, a, b, TRACE_W_PX, COL_COPPER, TRACE_ALPHA);

            /* máscara/soldermask en leve desplazamiento para “volumen” */
            if (is_axis_segment(a, b)){
                Pt aL = {a.x-1, a.y-1}, bL = {b.x-1, b.y-1};
                draw_line_rect(r, aL, bL, TRACE_W_PX, COL_MASK_L, MASK_LIGHT_A);

                Pt aD = {a.x+1, a.y+1}, bD = {b.x+1, b.y+1};
                draw_line_rect(r, aD, bD, TRACE_W_PX, COL_MASK_D, MASK_DARK_A);
            }
        }
    }

    /* pads */
    for (int i=0; i<padCount; i++){
        float x,y; grid_to_screen(W,H, pads[i].gx, pads[i].gy, &x,&y);
        draw_pad(r, (Pt){x,y});
    }

    SDL_SetRenderTarget(r, NULL);
    return tex;
}

/* ============================================================
   Recorridos sobre polígonos
   ============================================================ */
static Pt poly_sample(const Poly* P, float dist){
    if (!P || P->count < 2) return (Pt){0,0};
    if (dist <= 0)         return P->p[0];
    if (dist >= P->total)  return P->p[P->count-1];

    int lo=0, hi=P->count-1;
    while (lo + 1 < hi){
        int mid=(lo+hi)/2;
        if (P->cum[mid] <= dist) lo = mid;
        else                     hi = mid;
    }
    float seg = dist - P->cum[lo];
    float t   = (P->seg[lo] > 1e-5f ? seg / P->seg[lo] : 0.0f);
    Pt A=P->p[lo], B=P->p[lo+1];
    return (Pt){ lerpf(A.x,B.x,t), lerpf(A.y,B.y,t) };
}

static int poly_is_vertex(const Poly* P, float d, float eps){
    for (int i=0; i<P->count; i++){
        if (fabsf(P->cum[i] - d) < eps) return 1;
    }
    return 0;
}

/* ============================================================
   Electrones
   ============================================================ */
static void electron_reset(Electron* e, int poly_count){
    e->poly_idx = (poly_count ? rand() % poly_count : 0);
    e->dist  = frand() * 30.0f;
    e->speed = SPEED_MIN + frand() * (SPEED_MAX - SPEED_MIN);
    e->dwell = 0.0f;

    /* Color verde terminal */
    Uint8 r = (Uint8)(10 + (int)(frand()*10));
    Uint8 g = (Uint8)(10 + (int)(frand()*10));
    Uint8 b = (Uint8)(10 + (int)(frand()*10));
    e->color = (SDL_Color){r,g,b,255};
}

/* ============================================================
   API pública
   ============================================================ */
fx_electron_t* fx_electron_create(SDL_Renderer* r, int W, int H, const char* electron_png){
    if (!r || W <= 0 || H <= 0) return NULL;

    fx_electron_t* fx = (fx_electron_t*)calloc(1, sizeof(fx_electron_t));
    if (!fx) return NULL;
    fx->W = W; fx->H = H;

    /* 1) Construir escena (buffers estáticos) y copiar a memoria propia */
    TraceDef* tr_ptr; int tr_cnt;
    GPoint*   pad_ptr; int pad_cnt;
    build_scene(W,H, &tr_ptr,&tr_cnt, &pad_ptr,&pad_cnt);

    fx->traceCount = tr_cnt;
    fx->padCount   = pad_cnt;

    fx->traces = (TraceDef*)malloc(sizeof(TraceDef) * fx->traceCount);
    fx->pads   = (GPoint*)  malloc(sizeof(GPoint)   * fx->padCount);
    if (!fx->traces || !fx->pads){
        aa_electron_fx_destroy(fx);
        return NULL;
    }
    memcpy(fx->traces, tr_ptr, sizeof(TraceDef) * fx->traceCount);
    memcpy(fx->pads,   pad_ptr, sizeof(GPoint)  * fx->padCount);

    /* 2) Polígonos + board pre-render */
    fx->polys = (Poly*)malloc(sizeof(Poly) * (fx->traceCount ? fx->traceCount : 1));
    if (!fx->polys){
        aa_electron_fx_destroy(fx);
        return NULL;
    }

    fx->board = build_board(r, W, H, fx->traces, fx->traceCount, fx->pads, fx->padCount, fx->polys);

    /* 3) Target para trails */
    fx->trails = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, W, H);
    if (fx->trails) SDL_SetTextureBlendMode(fx->trails, SDL_BLENDMODE_BLEND);

    /* 4) Sprite de electrón */
    fx->electron_tex = load_png_texture(r, electron_png);
    if (!fx->electron_tex) fx->electron_tex = make_radial_sprite(r, DOT_SIZE);

    /* 5) Partículas */
    fx->E = (Electron*)malloc(sizeof(Electron) * ELECTRON_COUNT);
    if (!fx->E){
        aa_electron_fx_destroy(fx);
        return NULL;
    }
    for (int i=0; i<ELECTRON_COUNT; i++) electron_reset(&fx->E[i], fx->traceCount);

    return fx;
}

void fx_electron_update(fx_electron_t* fx, float dt){
    if (!fx) return;

    /* sub-steps para estabilidad si dt es grande */
    int sub = (dt > 0.016f) ? 2 : 1;
    float sdt = dt / sub;

    for (int step=0; step<sub; ++step){
        for (int i=0; i<ELECTRON_COUNT; i++){
            Electron* e = &fx->E[i];
            const Poly* P = &fx->polys[e->poly_idx];

            if (!P || P->count < 2){
                electron_reset(e, fx->traceCount);
                continue;
            }

            if (e->dwell > 0.0f){
                e->dwell -= sdt;
                if (e->dwell < 0.0f) e->dwell = 0.0f;
            } else {
                e->dist += e->speed * sdt;

                if (e->dist >= P->total){
                    e->poly_idx = (fx->traceCount ? rand() % fx->traceCount : 0);
                    P = &fx->polys[e->poly_idx];
                    e->dist = (P->total > 0.0f ? fmodf(e->dist, P->total) : 0.0f);
                }

                /* pequeña pausa en vértices/pads */
                if (poly_is_vertex(P, e->dist, 2.0f))
                    e->dwell = PAD_DWELL_MIN + frand() * (PAD_DWELL_MAX - PAD_DWELL_MIN);
            }
        }
    }
}

void fx_electron_render(fx_electron_t* fx, SDL_Renderer* r){
    if (!fx || !r) return;

    /* 1) Actualizar la textura de trails con fade */
    if (fx->trails){
        if (SDL_SetRenderTarget(r, fx->trails) == 0){
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 0,0,0, TRAIL_FADE_ALPHA);
            SDL_Rect fr = {0,0, fx->W, fx->H};
            SDL_RenderFillRect(r, &fr);

            for (int i=0; i<ELECTRON_COUNT; i++){
                const Poly* P = &fx->polys[fx->E[i].poly_idx];
                if (!P || P->count < 2) continue;

                float d = clampf(fx->E[i].dist, 0.0f, P->total);
                Pt pos = poly_sample(P, d);

                int s = (fx->E[i].dwell > 0.0f ? DOT_SIZE + 2 : DOT_SIZE);
                if (fx->electron_tex){
                    SDL_SetTextureColorMod(fx->electron_tex,
                                           fx->E[i].color.r, 
										   fx->E[i].color.g, 
										   fx->E[i].color.b);
#if SDL_VERSION_ATLEAST(2,0,10)
                    SDL_FRect dst = { pos.x - s*0.5f, 
									  pos.y - s*0.5f, 
									  (float)s, 
									  (float)s };
                    SDL_RenderCopyF(r, fx->electron_tex, NULL, &dst);
#else
                    SDL_Rect dst = { (int)(pos.x - s*0.5f), 
									 (int)(pos.y - s*0.5f), s, s };
                    SDL_RenderCopy(r, fx->electron_tex, NULL, &dst);
#endif
                }
            }
            SDL_SetRenderTarget(r, NULL);
        }
    }

    /* 2) Dibujar tablero y superponer trails */
    if (fx->board)  SDL_RenderCopy(r, fx->board,  NULL, NULL);
    if (fx->trails){
        SDL_SetTextureBlendMode(fx->trails, SDL_BLENDMODE_ADD);
        SDL_SetTextureAlphaMod(fx->trails, 255);
        SDL_RenderCopy(r, fx->trails, NULL, NULL);
    }
}

void aa_electron_fx_clear_trails(fx_electron_t* fx, SDL_Renderer* r){
    if (!fx || !r || !fx->trails) return;
    if (SDL_SetRenderTarget(r, fx->trails) == 0){
        SDL_SetRenderDrawColor(r, 0,0,0, 255);
        SDL_RenderClear(r);
        SDL_SetRenderTarget(r, NULL);
    }
}

void aa_electron_fx_destroy(fx_electron_t* fx){
    if (!fx) return;
    if (fx->board)        SDL_DestroyTexture(fx->board);
    if (fx->trails)       SDL_DestroyTexture(fx->trails);
    if (fx->electron_tex) SDL_DestroyTexture(fx->electron_tex);

    free(fx->traces);
    free(fx->pads);
    free(fx->polys);
    free(fx->E);
    free(fx);
}
