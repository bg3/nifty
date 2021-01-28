#include <SDL2/SDL.h>
#include <stdio.h>

#define SET_HOR 16
#define SET_VER 16
#define GUTTER 1
#define PAD 2
#define CHR_SIZE 8
#define MAP_HOR 48
#define MAP_VER 32
/* 11 x 26 map = 512x512 window
 * 48 x 32 */

int WIDTH = (PAD + SET_HOR + GUTTER + MAP_HOR + PAD) * CHR_SIZE;
int HEIGHT = (PAD + MAP_VER + PAD + PAD) * CHR_SIZE;
int FPS = 30, ZOOM = 2;

SDL_Window *gWindow;
SDL_Renderer *gRenderer;
SDL_Texture *gTexture;
Uint32 *pixels;

Uint32 palette[] = {
  0xf5f4eb,
  0x000000,
  0x191028,
  0x46AF45,
  0xa1d685,
  0x453e78,
  0x7664fe,
  0x833129,
  0x9ec2e8,
  0xdc534b,
  0xe18d79,
  0xd6b97b,
  0xe9d8a1,
  0x216c4b,
  0xd365c8,
  0xafaab9,
  /* system palette from here */
  0xdddddd,
  0x333333,
  0x666666,
  0x999999
};

typedef struct Brush {
  int x, y;
  int left, right, lshift, rshift;
  int canvas_hover;
  int colour_chrset;
  Uint8 chr;
  int pal[4];
} Brush;

Brush brush;

Uint8 chrset[SET_HOR * SET_VER * CHR_SIZE * 2]; /* 4 colour tile represented by 16 bits */
Uint8 chrmap[MAP_HOR * MAP_VER];
int   chrclr[MAP_HOR * MAP_VER * 4];

int error(char *msg, const char *err) {
	printf("Error %s: %s\n", msg, err);
	return 0;
}

void clear(Uint32 *dst) {
	int v, h;
	for(v = 0; v < HEIGHT; v++)
		for(h = 0; h < WIDTH; h++)
			dst[v * WIDTH + h] = palette[16];
}

void putpixel(Uint32 *dst, int x, int y, Uint32 colour) {
	if(x >= 0 && x < WIDTH - 8 && y >= 0 && y < HEIGHT - 8)
		dst[(y + PAD * CHR_SIZE) * WIDTH + (x + PAD * CHR_SIZE)] = colour;
}

int pal_index_at(int x, int y, int channel) {
  int n = x * 4 + y * MAP_HOR * 4 + channel;
  return chrclr[n];
}

Uint32 clr(int x, int y, int channel) {
  return palette[pal_index_at(x, y, channel)];
}

void drawchr_window(Uint32 *dst, int x, int y, int id, int colmode) {
  /* colmode: 0 = use chrclr[], 1 = use current brush palette, 2 = use system colours */
	int v, h, offset = id * 16;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int px = (x * 8) + (8 - h);
			int py = (y * 8) + v;
			int ch1 = chrset[offset + v];
			int ch2 = chrset[offset + v + 8];
			int channel = ((ch1 >> h) & 0x1) + (((ch2 >> h) & 0x1) << 1);
      Uint32 colour;
      switch (colmode) {
      case 0:
        colour = clr(x - SET_HOR - GUTTER, y, channel);
        break;
      case 1:
        colour = palette[brush.pal[channel]];
        break;
      case 2:
        colour = palette[16 + channel];
        break;
      }
			putpixel(dst, px - 1, py, colour);
		}
}

void drawchr_canvas(Uint32 *dst, int x, int y, int id, int colmode) {
  int mv_x;
  mv_x = SET_HOR + GUTTER + x;
  drawchr_window(dst, mv_x, y, id, colmode);
}

void drawpal_window(Uint32 *dst, int x, int y, int id, int c0, int c1, int c2, int c3) {
	int v, h, offset = id * 16;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int px = (x * 8) + (8 - h);
			int py = (y * 8) + v;
			int ch1 = chrset[offset + v];
			int ch2 = chrset[offset + v + 8];
			int clr_index = ((ch1 >> h) & 0x1) + (((ch2 >> h) & 0x1) << 1);
      clr_index = clr_index == 0 ? c0 : clr_index == 1 ? c1 : clr_index == 2 ? c2 : c3;
      Uint32 colour = palette[clr_index];
			putpixel(dst, px - 1, py, colour);
		}
}

void drawui(Uint32 *dst) {
  /* draw current character + colour */
  drawchr_window(dst, SET_HOR + GUTTER, MAP_VER + 1, brush.chr, 1);

  /* draw palette */
  drawpal_window(dst, 0, SET_VER + 1, 219, 0, 0, 0, 0);
  drawpal_window(dst, 1, SET_VER + 1, 219, 0, 1, 0, 0);
  drawpal_window(dst, 2, SET_VER + 1, 219, 0, 2, 0, 0);
  drawpal_window(dst, 3, SET_VER + 1, 219, 0, 3, 0, 0);
  drawpal_window(dst, 4, SET_VER + 1, 219, 0, 4, 0, 0);
  drawpal_window(dst, 5, SET_VER + 1, 219, 0, 5, 0, 0);
  drawpal_window(dst, 6, SET_VER + 1, 219, 0, 6, 0, 0);
  drawpal_window(dst, 7, SET_VER + 1, 219, 0, 7, 0, 0);
  drawpal_window(dst, 8, SET_VER + 1, 219, 0, 8, 0, 0);
  drawpal_window(dst, 9, SET_VER + 1, 219, 0, 9, 0, 0);
  drawpal_window(dst, 10, SET_VER + 1, 219, 0, 10, 0, 0);
  drawpal_window(dst, 11, SET_VER + 1, 219, 0, 11, 0, 0);
  drawpal_window(dst, 12, SET_VER + 1, 219, 0, 12, 0, 0);
  drawpal_window(dst, 13, SET_VER + 1, 219, 0, 13, 0, 0);
  drawpal_window(dst, 14, SET_VER + 1, 219, 0, 14, 0, 0);
  drawpal_window(dst, 15, SET_VER + 1, 219, 0, 15, 0, 0);

  /* draw numbers 1-4 under palette */
  drawchr_window(dst, brush.pal[0], SET_VER + 2, 49, 2);
  drawchr_window(dst, brush.pal[1], SET_VER + 2, 50, 2);
  drawchr_window(dst, brush.pal[2], SET_VER + 2, 51, 2);
  drawchr_window(dst, brush.pal[3], SET_VER + 2, 52, 2);

  /* draw brush */
  if (brush.canvas_hover) drawchr_canvas(pixels, brush.x, brush.y, brush.chr, 1);
}

void redraw(Uint32 *dst) {
	int x, y;
	clear(dst);

	for(y = 0; y < SET_VER; ++y)
		for(x = 0; x < SET_HOR; ++x)
			drawchr_window(dst, x, y, x + y * SET_HOR, brush.colour_chrset ? 1 : 2);

	for(y = 0; y < MAP_VER; ++y)
		for(x = 0; x < MAP_HOR; ++x)
			drawchr_canvas(dst, x, y, chrmap[x + y * MAP_HOR], 0);

  drawui(dst);

	SDL_UpdateTexture(gTexture, NULL, dst, WIDTH * sizeof(Uint32));
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
}

int save_rect(void) {
  int w, h;
  SDL_GetRendererOutputSize(gRenderer, &w, &h);
  w /= ZOOM;
  h /= ZOOM;
  SDL_Surface* whole = SDL_CreateRGBSurfaceFrom(pixels, w, h, 8 * 4, w * 4, 0,0,0,0);

  int part_w = MAP_HOR * CHR_SIZE;
  int part_h = MAP_VER * CHR_SIZE;
  int map_x = (PAD + SET_HOR + GUTTER) * CHR_SIZE;
  int map_y = PAD * CHR_SIZE;

  SDL_Surface* part = SDL_CreateRGBSurface(0, part_w, part_h, 32, 0, 0, 0, 0);
  SDL_Rect src_rect = {map_x, map_y, part_w, part_h};
  SDL_Rect dst_rect = {0, 0, part_w, part_h};
  SDL_BlitSurface(whole, &src_rect, part, &dst_rect);
  SDL_SaveBMP(part, "nifty.bmp");
  SDL_FreeSurface(whole);
  SDL_FreeSurface(part);
  return 1;
}

int save_window(void) {
  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 rmask = 0xff000000;
    Uint32 gmask = 0x00ff0000;
    Uint32 bmask = 0x0000ff00;
    Uint32 amask = 0x000000ff;
  #else
    Uint32 rmask = 0x000000ff;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x00ff0000;
    Uint32 amask = 0xff000000;
  #endif

  int w, h;
  SDL_GetRendererOutputSize(gRenderer, &w, &h);
  w /= ZOOM;
  h /= ZOOM;
  SDL_Surface *sshot = SDL_CreateRGBSurfaceFrom(pixels, w, h, 8 * 4, w * 4, rmask, gmask, bmask, amask);
  SDL_SaveBMP(sshot, "screenshot.bmp");
  SDL_FreeSurface(sshot);
  return 1;
  /* 1. SDL_GetWindowSurface(gWindow) == NULL
        https://wiki.libsdl.org/SDL_GetWindowSurface says:
        "You may not combine this with 3D or the rendering API on this window."
     2. NULL parameter in RenderReadPixels, does not seem to work on macOS?
     3. Gave up using RenderReadPixels entirely.*/
}

  void setclr(int x, int y, int channel, int pal_index) {
    chrclr[x * 4 + y * MAP_HOR * 4 + channel] = pal_index;
  }

  void quit(void) {
    free(pixels);
    SDL_DestroyTexture(gTexture);
    gTexture = NULL;
    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    SDL_Quit();
    exit(0);
  }

  void clearmap() {
    int x, y;
    for (y = 0; y < MAP_VER; ++y) {
      for (x = 0; x < MAP_HOR; ++x) {
        chrmap[x + y * MAP_HOR] = 0;
        int ch;
        for (ch = 0; ch < 4; ++ch) {
          setclr(x, y, ch, 0);
        }
      }
    }
  }

  void liftchr() {
     brush.chr = chrmap[brush.x + brush.y * MAP_HOR];
  }

  void dropchr(int chr) {
     chrmap[brush.x + brush.y * MAP_HOR] = chr;
  }

  void liftclr() {
    int index = brush.x * 4 + brush.y * MAP_HOR * 4;
    int ch;
    for (ch = 0; ch < 4; ++ch)
      brush.pal[ch] = chrclr[index + ch];
  }

  void dropclr() {
    int index = brush.x * 4 + brush.y * MAP_HOR * 4;
    int ch;
    for (ch = 0; ch < 4; ++ch)
      chrclr[index + ch] = brush.pal[ch];
  }



  void domouse(SDL_Event* event) {
    switch(event->type) {
    case SDL_MOUSEBUTTONUP:
        if (event->button.button == SDL_BUTTON_RIGHT) brush.right = 0;
        if (event->button.button == SDL_BUTTON_LEFT) brush.left = 0;
        break;
    case SDL_MOUSEBUTTONDOWN:
        if (event->button.button == SDL_BUTTON_RIGHT) brush.right = 1;
        if (event->button.button == SDL_BUTTON_LEFT) brush.left = 1;
    case SDL_MOUSEMOTION:
      if (event->motion.x > PAD * CHR_SIZE * ZOOM &&
          event->motion.x < (PAD + SET_HOR) * CHR_SIZE * ZOOM &&
          event->motion.y > PAD * CHR_SIZE * ZOOM &&
          event->motion.y < (PAD + SET_VER) * CHR_SIZE * ZOOM) {
        /* chr */
        brush.canvas_hover = 0;
        brush.x = (event->motion.x / ZOOM - (PAD * CHR_SIZE)) / CHR_SIZE;
        brush.y = (event->motion.y / ZOOM - (PAD * CHR_SIZE)) / CHR_SIZE;
        if (brush.left) {
          brush.chr = (brush.x + brush.y * SET_HOR);
      }
    } else if (event->motion.x > (PAD + SET_HOR + GUTTER) * CHR_SIZE * ZOOM &&
               event->motion.x < (PAD + SET_HOR + GUTTER + MAP_HOR) * CHR_SIZE * ZOOM &&
               event->motion.y > PAD * CHR_SIZE * ZOOM &&
               event->motion.y < (PAD + MAP_VER) * CHR_SIZE * ZOOM) {
      brush.canvas_hover = 1;
     /* map */
       brush.x = (event->motion.x / ZOOM - (PAD + SET_HOR + GUTTER) * CHR_SIZE) / CHR_SIZE;
       brush.y = (event->motion.y / ZOOM - PAD * CHR_SIZE) / CHR_SIZE;
       if (brush.left) {
         dropchr(brush.chr);
         dropclr();
       }
       if (brush.right) {
         liftchr();
         liftclr();
       }
    } else if (event->motion.x > PAD * CHR_SIZE * ZOOM &&
               event->motion.x < (PAD + SET_HOR) * CHR_SIZE * ZOOM &&
               event->motion.y > (PAD + 1 + SET_VER) * CHR_SIZE * ZOOM &&
               event->motion.y < (PAD + 2 + SET_VER) * CHR_SIZE * ZOOM) {
        brush.canvas_hover = 0;
        int hover_colour = (event->motion.x / ZOOM - PAD * CHR_SIZE) / CHR_SIZE;
        if (brush.lshift || brush.rshift) {
          if (brush.left) brush.pal[2] = hover_colour;
          if (brush.right) brush.pal[3] = hover_colour;
        } else {
          if (brush.left) brush.pal[1] = hover_colour;
          if (brush.right) brush.pal[0] = hover_colour;
        }
    } else {
      /* outside char set, palette and canvas */
      brush.canvas_hover = 0;
    }
  }
}

int init(void) {
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return error("Init", SDL_GetError());
	gWindow = SDL_CreateWindow("Nifty",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WIDTH * ZOOM,
		HEIGHT * ZOOM,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS);
	if(gWindow == NULL)
		return error("Window", SDL_GetError());
	gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
	if(gRenderer == NULL)
		return error("Renderer", SDL_GetError());
	gTexture = SDL_CreateTexture(gRenderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STATIC,
		WIDTH,
		HEIGHT);
	if(gTexture == NULL)
		return error("Texture", SDL_GetError());
	pixels = (Uint32 *)malloc(WIDTH * HEIGHT * sizeof(Uint32));
	if(pixels == NULL)
		return error("Pixels", "Failed to allocate memory");
	clear(pixels);
	srand(time(NULL));
	return 1;
}

void test() {
}

int resize_watcher(void* data, SDL_Event* event) {
  if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
/*        printf("RESIZE!");*/
  }
  return 0;
}


int main(int argc, char **argv) {
	FILE *fp;
	fp = fopen("spectrum-pi.chr", "r");
	fread(chrset, sizeof(chrset), 1, fp);

	int ticknext = 0;

	if (!init()) {
		return error("Init", "Failure");
	}

  brush.chr = 3;
  brush.pal[0] = 0;
  brush.pal[1] = 9;
  brush.pal[2] = 2;
  brush.pal[3] = 3;

	while (1) {
		int tick = SDL_GetTicks();
		SDL_Event event;
		if (tick < ticknext)
			SDL_Delay(ticknext - tick);
		ticknext = tick + (1000 / FPS) ;
    SDL_AddEventWatch(resize_watcher, gWindow);
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_QUIT: quit(); break;
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEMOTION:
          domouse(&event);
          redraw(pixels);
          break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_LSHIFT:
          brush.lshift = 1;
          break;
        case SDL_SCANCODE_RSHIFT:
          brush.rshift = 1;
          break;
				default:
					break;
				}
        break;
			case SDL_KEYUP:
				switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_P:
          brush.colour_chrset = !brush.colour_chrset;
          redraw(pixels);
          break;
        case SDL_SCANCODE_S:
          save_rect();
          break;
        case SDL_SCANCODE_X:
        {
          int tmp = brush.pal[0];
          brush.pal[0] = brush.pal[1];
          brush.pal[1] = tmp;
          break;
        }
        case SDL_SCANCODE_N:
          clearmap();
          redraw(pixels);
          break;
        case SDL_SCANCODE_LSHIFT:
          brush.lshift = 0;
          break;
        case SDL_SCANCODE_RSHIFT:
          brush.rshift = 0;
          break;
        case SDL_SCANCODE_T:
          test();
          break;
				default:
					break;
				}
        break;
      case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
				redraw(pixels);
				break;
			}
		}
	}
	quit();
	return 0;
}

/* TODO
 * DONE switch left/right mouse select.
 * DONE show tile that will be painted when hovering over the canvas
 * DONE improve region detection in mouse events.
 * fill area
 *   rect selection
 *   matching tiles (contiguous/non)
 *   noncontiguous is the same as replace
 * undo
 * select + drag, cut/paste
 * allow resizing
 * handle when canvas is smaller than charset
 * clean up tile/colour interaction, there are a bunch of different functions for drawing in the nametable, out of it, with palette, with theme colours, etc.
 * show hover tile with current brush colours
 * highlight selected tile in chrbuf area
 * confirm erase, or make it modifier+key
 * add system char set
 * reduce memory use for clrmap
 * import char set
 * export image
 * export char map (editable)
 * add character editing mode?
 */
