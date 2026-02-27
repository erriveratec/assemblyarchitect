
// src/title_screen.c
// SDL2 title screen with typewriter text, typing SFX, chip fade+pulse, and blinking "PRESS SPACE".
// Requires: SDL2, SDL2_ttf, SDL2_mixer

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define WINDOW_W 1280
#define WINDOW_H 720

// ---------- Utility: load BMP as texture (avoid SDL_image) ----------
static SDL_Texture* LoadBMPTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surf = SDL_LoadBMP(path);
    if (!surf) {
        SDL_Log("SDL_LoadBMP failed for '%s': %s", path, SDL_GetError());
        return NULL;
    }
    // If your BMP uses a magenta colorkey for transparency, enable this:
    // SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, 255, 0, 255));

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!tex) {
        SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
    }
    return tex;
}

// ---------- Utility: render TTF text to a texture ----------
typedef struct {
    SDL_Texture* tex;
    int w;
    int h;
} TextTex;

static TextTex RenderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color) {
    TextTex out = {0};
    if (!text || !*text) return out;

    SDL_Surface* s = TTF_RenderUTF8_Blended(font, text, color);
    if (!s) {
        SDL_Log("TTF_RenderUTF8_Blended failed: %s", TTF_GetError());
        return out;
    }
    out.tex = SDL_CreateTextureFromSurface(renderer, s);
    out.w = s->w;
    out.h = s->h;
    SDL_FreeSurface(s);

    if (!out.tex) {
        SDL_Log("SDL_CreateTextureFromSurface (text) failed: %s", SDL_GetError());
    }
    return out;
}

static void DestroyTextTex(TextTex* t) {
    if (t->tex) SDL_DestroyTexture(t->tex);
    t->tex = NULL; t->w = t->h = 0;
}

// ---------- Centered draw helpers ----------
static void DrawTextureCenter(SDL_Renderer* r, SDL_Texture* tex, int cx, int cy, float scale, Uint8 alpha) {
    if (!tex) return;
    int w, h; SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    int dw = (int)(w * scale);
    int dh = (int)(h * scale);
    SDL_Rect dst = { cx - dw/2, cy - dh/2, dw, dh };
    SDL_SetTextureAlphaMod(tex, alpha);
    SDL_RenderCopy(r, tex, NULL, &dst);
}

static void DrawTextCenter(SDL_Renderer* r, TextTex* t, int cx, int cy, Uint8 alpha) {
    if (!t || !t->tex) return;
    SDL_Rect dst = { cx - t->w/2, cy - t->h/2, t->w, t->h };
    SDL_SetTextureAlphaMod(t->tex, alpha);
    SDL_RenderCopy(r, t->tex, NULL, &dst);
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    // Init SDL (video + audio)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        SDL_Log("Mix_OpenAudio failed: %s", Mix_GetError());
        // Continue without sound if device isn't available.
    }

    SDL_Window* window = SDL_CreateWindow(
        "Assembly Architect - Title",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        Mix_CloseAudio(); TTF_Quit(); SDL_Quit(); return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        Mix_CloseAudio(); TTF_Quit(); SDL_Quit(); return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // --- Assets ---
    const char* FONT_PATH = "DOSVGA437.ttf"; // change to your pixel font
    const int   FONT_SIZE_TITLE = 48;   // adjust for your look
    const int   FONT_SIZE_UI    = 24;

    TTF_Font* fontTitle = TTF_OpenFont(FONT_PATH, FONT_SIZE_TITLE);
    TTF_Font* fontUI    = TTF_OpenFont(FONT_PATH, FONT_SIZE_UI);
    if (!fontTitle || !fontUI) {
        SDL_Log("TTF_OpenFont failed: %s", TTF_GetError());
        if (fontTitle) TTF_CloseFont(fontTitle);
        if (fontUI)    TTF_CloseFont(fontUI);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
        Mix_CloseAudio(); TTF_Quit(); SDL_Quit(); return 1;
    }

    SDL_Texture* chipTex = LoadBMPTexture(renderer, "assets/ui/chip.bmp");
    if (!chipTex) {
        SDL_Log("Warning: chip.bmp not found; will draw without chip.");
    }

    Mix_Chunk* sfxType  = Mix_LoadWAV("key_press.wav");
    if (!sfxType) SDL_Log("Warning: failed to load type.wav: %s", Mix_GetError());
    Mix_Chunk* sfxReady = Mix_LoadWAV("assets/audio/ready.wav"); // optional
    if (!sfxReady) SDL_Log("Note: ready.wav not found (optional).");

    // --- State ---
    const char* FULL_TITLE = "ASSEMBLY ARCHITECT";
    const Uint32 TYPE_DELAY_MS = 90;  // time between chars
    Uint32 lastTypeMs = SDL_GetTicks();
    size_t typeIndex = 0;

    bool titleDone = false;
    Uint8 chipAlpha = 0;              // fade-in after titleDone
    const Uint32 chipFadeMs = 1000;   // ~1s fade
    Uint32 chipStartMs = 0;

    // Pre-render dynamic text textures (we’ll rebuild when typing advances)
    SDL_Color white = { 230, 230, 230, 255 };
    SDL_Color grey  = { 200, 200, 200, 255 };

    TextTex titleTex = (TextTex){0};
    TextTex pressTex = RenderText(renderer, fontUI, "PRESS SPACE", grey);

    // Main loop
    bool running = true;
    Uint32 startMs = SDL_GetTicks();

    // For blink: 1.5s cycle, fade in/out
    const float blinkPeriodMs = 1500.0f;

    while (running) {
        // --- Events ---
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_SPACE) {
                    // Continue to next scene
                    running = false;
                }
            }
        }

        Uint32 now = SDL_GetTicks();

        // --- Typewriter logic ---
        if (!titleDone && now - lastTypeMs >= TYPE_DELAY_MS) {
            lastTypeMs = now;

            size_t fullLen = strlen(FULL_TITLE);
            if (typeIndex < fullLen) {
                typeIndex++;
                // Build the current substring
                char buf[256];
                size_t n = (typeIndex < sizeof(buf)-1 ? typeIndex : sizeof(buf)-1);
                memcpy(buf, FULL_TITLE, n);
                buf[n] = '\0';

                DestroyTextTex(&titleTex);
                titleTex = RenderText(renderer, fontTitle, buf, white);

                // Play type SFX (skip spaces so it feels like keystrokes)
                if (sfxType && buf[n-1] != ' ') {
                    Mix_PlayChannel(-1, sfxType, 0);
                }
            } else {
                titleDone = true;
                chipStartMs = now;
                if (sfxReady) Mix_PlayChannel(-1, sfxReady, 0);
            }
        }

        // --- Chip fade-in + subtle pulse (once title is done) ---
        float chipScale = 1.0f;
        if (titleDone) {
            Uint32 elapsed = now - chipStartMs;
            if (elapsed < chipFadeMs) {
                // ease-in alpha
                float t = (float)elapsed / (float)chipFadeMs;
                if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
                chipAlpha = (Uint8)(t * 255.0f);
            } else {
                chipAlpha = 255;
            }
            // Pulse scale: ±2% over 2s
            float pulse = sinf((now - chipStartMs) * (2.0f * (float)M_PI / 2000.0f));
            chipScale = 1.0f + 0.02f * pulse;
        }

        // --- Blink "PRESS SPACE" ---
        float blinkPhase = fmodf((float)(now - startMs), blinkPeriodMs) / blinkPeriodMs; // 0..1
        float blinkAlphaF = (blinkPhase < 0.5f) ? (blinkPhase * 2.0f) : ((1.0f - blinkPhase) * 2.0f);
        Uint8 pressAlpha = (Uint8)(blinkAlphaF * 255.0f);

        // --- Render ---
        SDL_SetRenderDrawColor(renderer, 8, 8, 8, 255);
        SDL_RenderClear(renderer);

        // (Optional) draw subtle scanlines
        for (int y = 0; y < WINDOW_H; y += 4) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 18); // very subtle
            SDL_RenderDrawLine(renderer, 0, y, WINDOW_W, y);
        }

        // Title at top center
        if (titleTex.tex) {
            DrawTextCenter(renderer, &titleTex, WINDOW_W/2, WINDOW_H/5, 255);
        }

        // Chip center
        if (chipTex) {
            DrawTextureCenter(renderer, chipTex, WINDOW_W/2, WINDOW_H/2, chipScale, chipAlpha);
        }

        // "PRESS SPACE" near bottom
        if (pressTex.tex) {
            DrawTextCenter(renderer, &pressTex, WINDOW_W/2, WINDOW_H - WINDOW_H/6, pressAlpha);
        }

        SDL_RenderPresent(renderer);

        // Cap ~60 FPS if vsync is off
        SDL_Delay(1);
    }

    // Cleanup
    DestroyTextTex(&titleTex);
    DestroyTextTex(&pressTex);
    if (chipTex) SDL_DestroyTexture(chipTex);
    if (sfxType)  Mix_FreeChunk(sfxType);
    if (sfxReady) Mix_FreeChunk(sfxReady);
    TTF_CloseFont(fontTitle);
    TTF_CloseFont(fontUI);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
