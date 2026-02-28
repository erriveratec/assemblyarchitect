
// src/title_screen.c -- v1.4a GLOBAL MANHATTAN + ELECTRONS
// 100% Manhattan routing (no diagonals, no arcs) + animated electrons with additive trails.

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WINDOW_W 1280
#define WINDOW_H 720

#define TRACE_W_PX      1
#define TRACE_ALPHA     32
#define MASK_LIGHT_A    22
#define MASK_DARK_A     18

#define PAD_RADIUS_PX   5
#define PAD_RING_PX     2
#define PAD_DRILL_PX    2

#define GRID_W  38
#define GRID_H  22

#define EDGE_PORTS_PER_SIDE 6
#define DENSITY_BUSES       3
#define DENSITY_SPINES      6
#define DENSITY_DIFFPAIRS   4
#define DENSITY_MEANDERS    4
#define DENSITY_ZONES       3

#define HV_BAL_TOL_PX     0.02f
#define MAX_BAL_ITER         28

#define ELECTRON_COUNT    82
#define SPEED_MIN        140.0f
#define SPEED_MAX        260.0f
#define PAD_DWELL_MIN      0.03f
#define PAD_DWELL_MAX      0.10f
#define TRAIL_FADE_ALPHA  14
#define DOT_SIZE          10

#define TYPE_DELAY_MS     90

#define GLOBAL_MANHATTAN   1
#define NO_ARCS            1
#define AXIS_MASK_ONLY     1
#define ELECTRONS_ENABLED  1

// ---- Types ----
typedef struct { int gx,gy; } GPoint;
typedef struct { int count; GPoint p[200]; } TraceDef;
typedef struct { float x,y; } Pt;
typedef struct { int count; Pt p[600]; float seg[599]; float cum[600]; float total; } Poly;

// ---- Utils ----
static inline float clampf(float v,float a,float b){ return v<a? a : (v>b? b : v); }
static inline float lerpf(float a,float b,float t){ return a + (b-a)*t; }
static inline float hypotf_s(float x,float y){ return sqrtf(x*x+y*y); }
static inline float frand(){ return (float)(rand()%10000)/10000.0f; }
static inline int sgnf(float x){ return (x>0) - (x<0); }

static void grid_to_screen(int gx,int gy,float* x,float* y){ const float mx=0.055f,my=0.095f; const float sx=1.0f-2*mx, sy=1.0f-2*my; *x=(mx + (float)gx/(GRID_W-1)*sx)*WINDOW_W; *y=(my + (float)gy/(GRID_H-1)*sy)*WINDOW_H; }
static void cell_size(float* cx,float* cy){ const float mx=0.055f,my=0.095f; const float sx=1.0f-2*mx, sy=1.0f-2*my; *cx = (sx * WINDOW_W) / (GRID_W-1); *cy = (sy * WINDOW_H) / (GRID_H-1); }

static bool is_axis_segment(Pt a, Pt b){ return fabsf(a.y-b.y) < 0.5f || fabsf(a.x-b.x) < 0.5f; }

// ---- Manhattan-only poly build ----
static Poly build_poly(const TraceDef* tr){
    Poly out={0}; if(tr->count<2) return out;
    // Convert raw grid points to screen points; keep Manhattan corners (L).
    for(int i=0;i<tr->count;i++){ float x,y; grid_to_screen(tr->p[i].gx,tr->p[i].gy,&x,&y); out.p[out.count++] = (Pt){x,y}; }
    // Safety: decompose any non-axis A->B into A->(A.x,B.y)->B
    Pt tmp[600]; int n2=0; for(int i=0;i<out.count-1;i++){
        Pt A=out.p[i], B=out.p[i+1]; tmp[n2++]=A; if(!is_axis_segment(A,B)){ Pt M1={A.x, B.y}; tmp[n2++]=M1; }
    }
    tmp[n2++]=out.p[out.count-1];
    out.count=0; for(int i=0;i<n2;i++) out.p[out.count++]=tmp[i];
    // Lengths
    out.total=0; if(out.count>0) out.cum[0]=0; for(int i=0;i<out.count-1;i++){ float L=hypotf_s(out.p[i+1].x-out.p[i].x, out.p[i+1].y-out.p[i].y); out.seg[i]=L; out.total+=L; out.cum[i+1]=out.total; }
    return out;
}

// ---- Generators ----
static void push_gpt(TraceDef* t,int gx,int gy){ if(t->count<200){ t->p[t->count++] = (GPoint){gx,gy}; } }
static int ri(int a,int b){ return a + rand()%((b-a)+1); }
static int clampi(int v,int a,int b){ return v<a? a : (v>b? b : v); }

static TraceDef make_bus_row(int gy){ TraceDef t={0}; int x=-2; push_gpt(&t,x,gy); x=0; push_gpt(&t,x,gy); while(x<GRID_W+1){ int jog=(frand()<0.14f)? (frand()<0.5f? -1:+1):0; int nx=x+ri(4,7); if(nx>GRID_W+1) nx=GRID_W+1; if(jog) push_gpt(&t,x, clampi(gy+jog,0,GRID_H-1)); push_gpt(&t,nx, clampi(gy+jog,0,GRID_H-1)); x=nx; } return t; }
static TraceDef make_spine_col(int gx){ TraceDef t={0}; int y=-2; push_gpt(&t,gx,y); y=0; push_gpt(&t,gx,y); while(y<GRID_H+1){ int jog=(frand()<0.18f)? (frand()<0.5f? -1:+1):0; int ny=y+ri(4,7); if(ny>GRID_H+1) ny=GRID_H+1; if(jog) push_gpt(&t, clampi(gx+jog,0,GRID_W-1), y); push_gpt(&t, clampi(gx+jog,0,GRID_W-1), ny); y=ny; } return t; }
static void add_hdiffpair(TraceDef* A,TraceDef* B,int gy){ *A=make_bus_row(gy); *B=make_bus_row(clampi(gy+1,0,GRID_H-1)); }
static void add_vdiffpair(TraceDef* A,TraceDef* B,int gx){ *A=make_spine_col(gx); *B=make_spine_col(clampi(gx+1,0,GRID_W-1)); }
static TraceDef make_meander_h(int x0,int y0,int w,int h){ TraceDef t={0}; int dir=1; for(int y=y0; y<=y0+h; y+=2){ int xs=x0, xe=x0+w; if(dir<0){ int tmp=xs; xs=xe; xe=tmp; } push_gpt(&t,xs,y); push_gpt(&t,xe,y); if(y<y0+h) push_gpt(&t,xe,y+1); dir*=-1;} return t; }
static TraceDef make_meander_v(int x0,int y0,int w,int h){ TraceDef t={0}; int dir=1; for(int x=x0; x<=x0+w; x+=2){ int ys=y0, ye=y0+h; if(dir<0){ int tmp=ys; ys=ye; ye=tmp; } push_gpt(&t,x,ys); push_gpt(&t,x,ye); if(x<x0+w) push_gpt(&t,x+1,ye); dir*=-1;} return t; }

// ---- Scene building + H/V balance ----
static void measure_HV_screen(const TraceDef* t, float cellx, float celly, double* Hp, double* Vp){ for(int i=0;i<t->count-1;i++){ int dx=t->p[i+1].gx - t->p[i].gx; int dy=t->p[i+1].gy - t->p[i].gy; *Hp += fabs((double)dx)*(double)cellx; *Vp += fabs((double)dy)*(double)celly; } }

static void build_scene(TraceDef** outTraces,int* outCount,GPoint** outPads,int* outPadCount){ static TraceDef traces[1200]; int tCount=0; static GPoint pads[2400]; int pCount=0;
    // Edge ports on 4 edges
    for(int i=0;i<EDGE_PORTS_PER_SIDE;i++){ int gx=ri(0,GRID_W-1); TraceDef s={0}; push_gpt(&s,gx,-2); push_gpt(&s,gx,ri(0,2)); traces[tCount++]=s; gx=ri(0,GRID_W-1); s=(TraceDef){0}; push_gpt(&s,gx,GRID_H+1); push_gpt(&s,gx,GRID_H-ri(0,2)); traces[tCount++]=s; int gy=ri(0,GRID_H-1); s=(TraceDef){0}; push_gpt(&s,-2,gy); push_gpt(&s,ri(0,2),gy); traces[tCount++]=s; gy=ri(0,GRID_H-1); s=(TraceDef){0}; push_gpt(&s,GRID_W+1,gy); push_gpt(&s,GRID_W-ri(0,2),gy); traces[tCount++]=s; }
    // Seeds
    for(int i=0;i<DENSITY_BUSES;i++){ int gy=clampi(ri(1,GRID_H-2),1,GRID_H-2); traces[tCount++]=make_bus_row(gy); }
    for(int i=0;i<DENSITY_SPINES;i++){ int gx=clampi(ri(1,GRID_W-2),1,GRID_W-2); traces[tCount++]=make_spine_col(gx); }
    for(int i=0;i<DENSITY_DIFFPAIRS;i++){ if(i%2==0){ TraceDef A,B; add_hdiffpair(&A,&B,clampi(ri(3,GRID_H-4),3,GRID_H-4)); traces[tCount++]=A; traces[tCount++]=B; } else { TraceDef A,B; add_vdiffpair(&A,&B,clampi(ri(3,GRID_W-4),3,GRID_W-4)); traces[tCount++]=A; traces[tCount++]=B; } }
    for(int i=0;i<DENSITY_MEANDERS;i++){ int w=ri(4,7), h=ri(3,4); int x0=ri(2,GRID_W-w-3), y0=ri(2,GRID_H-h-3); traces[tCount++] = (i%2==0? make_meander_h(x0,y0,w,h) : make_meander_v(x0,y0,w,h)); }
    for(int i=0;i<DENSITY_ZONES;i++){ int cx=ri(3,GRID_W-4), cy=ri(3,GRID_H-4); pads[pCount++] = (GPoint){cx,cy}; pads[pCount++] = (GPoint){cx+1,cy}; pads[pCount++] = (GPoint){cx,cy+1}; pads[pCount++] = (GPoint){cx+1,cy+1}; }

    // Screen-space H/V balancing
    float cellx, celly; cell_size(&cellx,&celly);
    int iter=0; while(iter++<MAX_BAL_ITER){ double Hpx=0.0, Vpx=0.0; for(int i=0;i<tCount;i++) measure_HV_screen(&traces[i], cellx, celly, &Hpx, &Vpx); double tot=Hpx+Vpx; if(tot<=1.0) break; double diff=fabs(Hpx-Vpx)/tot; if(diff<=HV_BAL_TOL_PX) break; if(Hpx>Vpx){ int gx=clampi(ri(1,GRID_W-2),1,GRID_W-2); traces[tCount++]=make_spine_col(gx); } else { int gy=clampi(ri(1,GRID_H-2),1,GRID_H-2); traces[tCount++]=make_bus_row(gy); } }

    *outTraces=traces; *outCount=tCount; *outPads=pads; *outPadCount=pCount; }

// ---- Electron system ----
typedef struct { int polyIdx; float dist; float speed; float dwell; SDL_Color color; } Electron;

static Pt poly_sample(const Poly* P, float dist){ if(P->count<2) return (Pt){0,0}; if(dist<=0) return P->p[0]; if(dist>=P->total) return P->p[P->count-1]; int lo=0,hi=P->count-1; while(lo+1<hi){ int mid=(lo+hi)/2; if(P->cum[mid]<=dist) lo=mid; else hi=mid; } float seg=dist-P->cum[lo]; float t=(P->seg[lo]>1e-5f? seg/P->seg[lo]:0.0f); Pt A=P->p[lo],B=P->p[lo+1]; return (Pt){lerpf(A.x,B.x,t), lerpf(A.y,B.y,t)}; }
static bool poly_is_vertex(const Poly* P,float d,float eps){ for(int i=0;i<P->count;i++){ if(fabsf(P->cum[i]-d)<eps) return true; } return false; }
static void electron_reset(Electron* e,int polyCount){ e->polyIdx=rand()%polyCount; e->dist=frand()*30.0f; e->speed=SPEED_MIN + frand()*(SPEED_MAX-SPEED_MIN); e->dwell=0.0f; Uint8 g=(Uint8)(220 + (int)(frand()*35)), r=(Uint8)(10 + (int)(frand()*25)), b=(Uint8)(10 + (int)(frand()*40)); e->color=(SDL_Color){r,g,b,255}; }

static SDL_Texture* make_radial_sprite(SDL_Renderer* r,int size){
    SDL_Surface* s=SDL_CreateRGBSurfaceWithFormat(0,size,size,32,SDL_PIXELFORMAT_RGBA32);
    if(!s) return NULL;
    Uint32* p=(Uint32*)s->pixels; int pitch=s->pitch/4; float R=size/2.0f; float R2=R*R;
    for(int y=0;y<size;y++){
        for(int x=0;x<size;x++){
            float dx=x-R+0.5f, dy=y-R+0.5f; float d2=dx*dx+dy*dy; float a=0.0f;
            if(d2<=R2){ float d=sqrtf(d2)/R; a = powf(1.0f-d, 2.0f);} Uint8 A=(Uint8)(a*255.0f);
            p[y*pitch+x]=(A<<24)|(255<<16)|(255<<8)|255; // white with alpha
        }
    }
    SDL_Texture* tex=SDL_CreateTextureFromSurface(r,s); SDL_FreeSurface(s);
    SDL_SetTextureBlendMode(tex,SDL_BLENDMODE_ADD);
    return tex;
}

// ---- Rendering helpers ----
static void draw_bg(SDL_Renderer* r){ const Uint8 top[3]={8,10,8}, bot[3]={6,8,6}; for(int y=0;y<WINDOW_H;y++){ float t=(float)y/(float)(WINDOW_H-1); Uint8 R=(Uint8)lerpf(top[0],bot[0],t), G=(Uint8)lerpf(top[1],bot[1],t), B=(Uint8)lerpf(top[2],bot[2],t); SDL_SetRenderDrawColor(r,R,G,B,255); SDL_RenderDrawLine(r,0,y,WINDOW_W,y);} }

static void draw_line_rect(SDL_Renderer* r, Pt a, Pt b, int w, SDL_Color c, Uint8 apha){
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r,c.r,c.g,c.b,apha);
    if (fabsf(a.y-b.y) < 0.5f){ int x1=(int)fminf(a.x,b.x), x2=(int)fmaxf(a.x,b.x); int y=(int)a.y - w/2; SDL_Rect rr={x1,y,x2-x1+1,w}; SDL_RenderFillRect(r,&rr); return; }
    if (fabsf(a.x-b.x) < 0.5f){ int y1=(int)fminf(a.y,b.y), y2=(int)fmaxf(a.y,b.y); int x=(int)a.x - w/2; SDL_Rect rr={x,y1,w,y2-y1+1}; SDL_RenderFillRect(r,&rr); return; }
    // Safety: render non-axis as two Manhattan legs
    Pt m={a.x, b.y}; SDL_Rect r1={ (int)fminf(a.x,m.x), (int)(a.y-w/2), (int)fabsf(m.x-a.x)+1, w}; SDL_RenderFillRect(r,&r1);
    int y1=(int)fminf(m.y,b.y), y2=(int)fmaxf(m.y,b.y); int x=(int)b.x - w/2; SDL_Rect r2={ x, y1, w, y2-y1+1}; SDL_RenderFillRect(r,&r2);
}

static void draw_pad(SDL_Renderer* r, Pt c){ SDL_Color ring={90,90,90,255}, hole={0,0,0,255}; int R=PAD_RADIUS_PX; int x=(int)c.x, y=(int)c.y; for(int i=-R;i<=R;i++){ for(int j=-R;j<=R;j++){ int d=i*i+j*j; if(d>=(R-1)*(R-1) && d<=R*R){ SDL_SetRenderDrawColor(r,ring.r,ring.g,ring.b,TRACE_ALPHA+12); SDL_RenderDrawPoint(r,x+i,y+j);} }} int r0=PAD_DRILL_PX; for(int i=-r0;i<=r0;i++){ for(int j=-r0;j<=r0;j++){ if(i*i+j*j<=r0*r0){ SDL_SetRenderDrawColor(r,hole.r,hole.g,hole.b,230); SDL_RenderDrawPoint(r,x+i,y+j); }}} }

static SDL_Texture* build_board(SDL_Renderer* r, const TraceDef* traces, int traceCount, const GPoint* pads, int padCount, Poly* outPolys){ SDL_Texture* tex=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,WINDOW_W,WINDOW_H); SDL_SetTextureBlendMode(tex,SDL_BLENDMODE_BLEND); SDL_SetRenderTarget(r,tex); draw_bg(r); SDL_Color copper={66,66,66,255}, light={120,130,120,255}, dark={20,20,20,255}; for(int i=0;i<traceCount;i++){ outPolys[i]=build_poly(&traces[i]); const Poly* P=&outPolys[i]; for(int k=0;k<P->count-1;k++){ Pt a=P->p[k], b=P->p[k+1]; draw_line_rect(r,a,b,TRACE_W_PX,copper,TRACE_ALPHA); if (is_axis_segment(a,b)){ Pt aL={a.x-1,a.y-1}, bL={b.x-1,b.y-1}; draw_line_rect(r,aL,bL,TRACE_W_PX,light,MASK_LIGHT_A); Pt aD={a.x+1,a.y+1}, bD={b.x+1,b.y+1}; draw_line_rect(r,aD,bD,TRACE_W_PX,dark,MASK_DARK_A);} } } for(int i=0;i<padCount;i++){ float x,y; grid_to_screen(pads[i].gx,pads[i].gy,&x,&y); draw_pad(r,(Pt){x,y}); } SDL_SetRenderTarget(r,NULL); return tex; }

// ---- Title helpers ----
typedef struct { SDL_Texture* tex; int w; int h; } TextTex;
static TextTex RenderText(SDL_Renderer* renderer, TTF_Font* font,const char* text, SDL_Color color){ TextTex out={0}; if(!text||!*text) return out; SDL_Surface* s=TTF_RenderUTF8_Blended(font,text,color); if(!s){ SDL_Log("TTF_RenderUTF8_Blended: %s", TTF_GetError()); return out; } out.tex=SDL_CreateTextureFromSurface(renderer,s); out.w=s->w; out.h=s->h; SDL_FreeSurface(s); if(!out.tex) SDL_Log("CreateTextureFromSurface(text): %s", SDL_GetError()); return out; }
static void DestroyTextTex(TextTex* t){ if(t->tex) SDL_DestroyTexture(t->tex); t->tex=NULL; t->w=t->h=0; }
static void DrawTextCenter(SDL_Renderer* r,TextTex* t,int cx,int cy,Uint8 alpha){ if(!t||!t->tex) return; SDL_Rect d={cx-t->w/2, cy-t->h/2, t->w, t->h}; SDL_SetTextureAlphaMod(t->tex,alpha); SDL_RenderCopy(r,t->tex,NULL,&d);} 
static SDL_Texture* LoadPNGTexture(SDL_Renderer* r,const char* path){ SDL_Texture* tex=IMG_LoadTexture(r,path); if(!tex) SDL_Log("IMG_LoadTexture failed '%s': %s", path, IMG_GetError()); return tex; }
static SDL_Texture* make_default_electron(SDL_Renderer* r){ SDL_Texture* t=make_radial_sprite(r,DOT_SIZE); return t; }

int main(){ if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0){ SDL_Log("SDL_Init: %s", SDL_GetError()); return 1; } if(TTF_Init()==-1){ SDL_Log("TTF_Init: %s", TTF_GetError()); SDL_Quit(); return 1; } if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,1024)<0){ SDL_Log("Mix_OpenAudio: %s", Mix_GetError()); }
    int imgFlags=IMG_INIT_PNG; if((IMG_Init(imgFlags)&imgFlags)!=imgFlags){ SDL_Log("IMG_Init PNG failed: %s", IMG_GetError()); }

    SDL_Window* win=SDL_CreateWindow("Assembly Architect - Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
    if(!win){ SDL_Log("CreateWindow failed: %s", SDL_GetError()); return 1; }
    SDL_Renderer* r=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!r){ SDL_Log("CreateRenderer failed: %s", SDL_GetError()); return 1; }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"2");

    const char* FONT_PATH="assets/fonts/PressStart2P.ttf"; int FTitle=48, FUI=24; TTF_Font* fTitle=TTF_OpenFont(FONT_PATH,FTitle); TTF_Font* fUI=TTF_OpenFont(FONT_PATH,FUI);
    SDL_Texture* chip=LoadPNGTexture(r,"assets/ui/chip.png");
    Mix_Chunk* sType=Mix_LoadWAV("assets/audio/type.wav"); Mix_Chunk* sReady=Mix_LoadWAV("assets/audio/ready.wav");

    TraceDef* traces; int tCount; GPoint* pads; int pCount; build_scene(&traces,&tCount,&pads,&pCount);
    Poly* polys=(Poly*)malloc(sizeof(Poly)*tCount);
    SDL_Texture* board=build_board(r,traces,tCount,pads,pCount,polys);

    SDL_Texture* electronTex=LoadPNGTexture(r,"assets/ui/electron.png"); if(!electronTex) electronTex=make_default_electron(r);

    const char* TITLE="ASSEMBLY ARCHITECT"; Uint32 lastType=SDL_GetTicks(); size_t ti=0; bool typed=false; SDL_Color white={220,255,220,255}, grey={170,200,170,255}; TextTex titleTex={0}; TextTex prompt=RenderText(r,fUI,"PRESS SPACE",grey);

    SDL_Texture* trails=SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WINDOW_W, WINDOW_H); SDL_SetTextureBlendMode(trails, SDL_BLENDMODE_BLEND);

    Electron* E=(Electron*)malloc(sizeof(Electron)*ELECTRON_COUNT); for(int i=0;i<ELECTRON_COUNT;i++){ electron_reset(&E[i], tCount); }

    bool running=true; Uint32 start=SDL_GetTicks(), prev=SDL_GetTicks();
    while(running){
        SDL_Event ev; while(SDL_PollEvent(&ev)){
            if(ev.type==SDL_QUIT) running=false;
            if(ev.type==SDL_KEYDOWN){ if(ev.key.keysym.sym==SDLK_ESCAPE) running=false; if(ev.key.keysym.sym==SDLK_SPACE||ev.key.keysym.sym==SDLK_RETURN) running=false; }
        }
        Uint32 now=SDL_GetTicks(); float dt=(now-prev)/1000.0f; prev=now;

        // Title typing
        if(!typed && now-lastType>=TYPE_DELAY_MS){ lastType=now; size_t N=strlen(TITLE); if(ti<N){ ti++; char buf[128]; size_t n=(ti<sizeof(buf)-1?ti:sizeof(buf)-1); memcpy(buf,TITLE,n); buf[n]='\0'; if(titleTex.tex){ SDL_DestroyTexture(titleTex.tex); titleTex.tex=NULL; } titleTex=RenderText(r,fTitle,buf,white); if(sType && buf[n-1]!=' ') Mix_PlayChannel(-1,sType,0);} else { typed=true; if(sReady) Mix_PlayChannel(-1,sReady,0);} }

        // Update electrons (sub-steps for smoother trails)
        int sub=(dt>0.016f)? 2:1; float sdt=dt/sub; for(int step=0; step<sub; ++step){
            for(int i=0;i<ELECTRON_COUNT;i++){
                Electron* e=&E[i]; const Poly* P=&polys[e->polyIdx]; if(P->count<2){ electron_reset(e,tCount); continue; }
                if(e->dwell>0){ e->dwell-=sdt; if(e->dwell<0) e->dwell=0; }
                else {
                    e->dist += e->speed * sdt;
                    if(e->dist>=P->total){ e->polyIdx=rand()%tCount; P=&polys[e->polyIdx]; e->dist=fmodf(e->dist,P->total); }
                    if(poly_is_vertex(P,e->dist,2.0f)) e->dwell = PAD_DWELL_MIN + frand()*(PAD_DWELL_MAX-PAD_DWELL_MIN);
                }
            }
        }

        // Fade trails and stamp electrons
        SDL_SetRenderTarget(r,trails);
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r,0,10,0,TRAIL_FADE_ALPHA);
        SDL_Rect fr={0,0,WINDOW_W,WINDOW_H}; SDL_RenderFillRect(r,&fr);
        for(int i=0;i<ELECTRON_COUNT;i++){
            const Poly* P=&polys[E[i].polyIdx]; Pt pos=poly_sample(P, fminf(fmaxf(E[i].dist,0),P->total)); int s=(E[i].dwell>0? DOT_SIZE+2: DOT_SIZE);
            SDL_SetTextureColorMod(electronTex, E[i].color.r, E[i].color.g, E[i].color.b);
            SDL_FRect dst={ pos.x - s*0.5f, pos.y - s*0.5f, (float)s, (float)s };
            SDL_RenderCopyF(r, electronTex, NULL, &dst);
        }
        SDL_SetRenderTarget(r,NULL);

        // Compose frame
        SDL_SetRenderDrawColor(r,0,0,0,255); SDL_RenderClear(r);
        if(board) SDL_RenderCopy(r,board,NULL,NULL);
        if(trails){ SDL_SetTextureBlendMode(trails,SDL_BLENDMODE_ADD); SDL_SetTextureAlphaMod(trails,255); SDL_RenderCopy(r,trails,NULL,NULL); }
        if(titleTex.tex) DrawTextCenter(r,&titleTex,WINDOW_W/2,WINDOW_H/5,255);
        if(prompt.tex){ float ph=fmodf((float)(now-start),1500.0f)/1500.0f; float af=(ph<0.5f)?(ph*2.0f):((1.0f-ph)*2.0f); Uint8 pa=(Uint8)(af*255.0f); DrawTextCenter(r,&prompt, WINDOW_W/2, WINDOW_H - WINDOW_H/6, pa);}        
        SDL_RenderPresent(r);
    }

    // Cleanup
    if(board) SDL_DestroyTexture(board);
    if(trails) SDL_DestroyTexture(trails);
    if(electronTex) SDL_DestroyTexture(electronTex);
    if(titleTex.tex) SDL_DestroyTexture(titleTex.tex);
    if(prompt.tex) SDL_DestroyTexture(prompt.tex);
    if(fTitle) TTF_CloseFont(fTitle); if(fUI) TTF_CloseFont(fUI);
    SDL_DestroyRenderer(r); SDL_DestroyWindow(win);
    Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit();
    return 0;
}
