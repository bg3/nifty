#include <SDL2/SDL.h>
#include <stdio.h>

#define HOR 16
#define VER 16
#define PAD 2
#define SZ (HOR * VER * 16)

int WIDTH = 2 * 8 * HOR + 8 * PAD * 2.5;
int HEIGHT = 8 * (VER + 2) + 8 * PAD * 2;
int FPS = 30, GUIDES = 1, BIGPIXEL = 0, ZOOM = 2;

SDL_Window *gWindow;
SDL_Renderer *gRenderer;
SDL_Texture *gTexture;
Uint32 *pixels;

Uint32 theme[] = {
  0xdddddd,
  0x333333,
  0x666666,
  0x999999
};

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
  0xdddddd
/*  0xafaab9*/
};

typedef struct Brush {
  int x, y;
  int left, right, lshift, rshift;
  Uint8 chr;
  int palette[4];
} Brush;

Brush brush;

Uint8 chrset[16 * 16 * 8 * 2];
Uint8 chrmap[HOR * VER];
int chrclrs[HOR * VER * 4];

void randmap() {
	int x, y, r;
	for (y = 0; y < VER; ++y) {
		for (x = 0; x < HOR; ++x) {
			r = rand() % 3;
			if (r == 0) {
				r = rand() % 256;
				chrmap[x + y * HOR] = r;
			} else {
				chrmap[x + y * HOR] = 0;
			}
		}
	}
}

int pal_index_at(int x, int y, int channel) {
  int n = x * 4 + y * HOR * 4 + channel;
  return chrclrs[n];
}

Uint32 clr(int x, int y, int channel) {
  return palette[pal_index_at(x, y, channel)];
}

void setclr(int x, int y, int channel, int pal_index) {
  chrclrs[x * 4 + y * HOR * 4 + channel] = pal_index;
}

int error(char *msg, const char *err) {
	printf("Error %s: %s\n", msg, err);
	return 0;
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

void clear(Uint32 *dst) {
	int v, h;
	for(v = 0; v < HEIGHT; v++)
		for(h = 0; h < WIDTH; h++)
			dst[v * WIDTH + h] = theme[0];
}

void liftchr() {
   brush.chr = chrmap[brush.x + brush.y * HOR];
}

void dropchr() {
   chrmap[brush.x + brush.y * HOR] = brush.chr;
}

void liftclr() {
  int index = brush.x * 4 + brush.y * HOR * 4;
  int ch;
  for (ch = 0; ch < 4; ++ch)
    brush.palette[ch] = chrclrs[index + ch];
}

void dropclr() {
  int index = brush.x * 4 + brush.y * HOR * 4;
  int ch;
  for (ch = 0; ch < 4; ++ch)
    chrclrs[index + ch] = brush.palette[ch];
}

void putpixel(Uint32 *dst, int x, int y, Uint32 color) {
	if(x >= 0 && x < WIDTH - 8 && y >= 0 && y < HEIGHT - 8)
		dst[(y + PAD * 8) * WIDTH + (x + PAD * 8)] = color; 
}

void drawchr(Uint32 *dst, int x, int y, int id) {
	int v, h, offset = id * 16;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int px = (x * 8) + (8 - h);
			int py = (y * 8) + v;
			int ch1 = chrset[offset + v];
			int ch2 = chrset[offset + v + 8];
			int channel = ((ch1 >> h) & 0x1) + (((ch2 >> h) & 0x1) << 1);
      Uint32 color = clr(x - HOR - 1, y, channel);
			putpixel(dst, px - 1, py, color);
		}
}

void drawchr_ui(Uint32 *dst, int x, int y, int id) {
	int v, h, offset = id * 16;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int px = (x * 8) + (8 - h);
			int py = (y * 8) + v;
			int ch1 = chrset[offset + v];
			int ch2 = chrset[offset + v + 8];
			int clr_index = ((ch1 >> h) & 0x1) + (((ch2 >> h) & 0x1) << 1);
      Uint32 color = theme[clr_index];
			putpixel(dst, px - 1, py, color);
		}
}

void drawchr_ui_pal(Uint32 *dst, int x, int y, int id, int c0, int c1, int c2, int c3) {
	int v, h, offset = id * 16;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int px = (x * 8) + (8 - h);
			int py = (y * 8) + v;
			int ch1 = chrset[offset + v];
			int ch2 = chrset[offset + v + 8];
			int clr_index = ((ch1 >> h) & 0x1) + (((ch2 >> h) & 0x1) << 1);
      clr_index = clr_index == 0 ? c0 : clr_index == 1 ? c1 : clr_index == 2 ? c2 : c3;
      Uint32 color = palette[clr_index];
			putpixel(dst, px - 1, py, color);
		}
}

void drawchr_ui_theme(Uint32 *dst, int x, int y, int id, int c0, int c1, int c2, int c3) {
	int v, h, offset = id * 16;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int px = (x * 8) + (8 - h);
			int py = (y * 8) + v;
			int ch1 = chrset[offset + v];
			int ch2 = chrset[offset + v + 8];
			int clr_index = ((ch1 >> h) & 0x1) + (((ch2 >> h) & 0x1) << 1);
      clr_index = clr_index == 0 ? c0 : clr_index == 1 ? c1 : clr_index == 2 ? c2 : c3;
      Uint32 color = theme[clr_index];
			putpixel(dst, px - 1, py, color);
		}
}

void drawui(Uint32 *dst) {
  drawchr_ui_pal(dst, 17, VER + 1, brush.chr, brush.palette[0], brush.palette[1], brush.palette[2], brush.palette[3]); 

  drawchr_ui_pal(dst, 0, VER + 1, 219, 0, 0, 0, 0);
  drawchr_ui_pal(dst, 1, VER + 1, 219, 0, 1, 0, 0);
  drawchr_ui_pal(dst, 2, VER + 1, 219, 0, 2, 0, 0);
  drawchr_ui_pal(dst, 3, VER + 1, 219, 0, 3, 0, 0);
  drawchr_ui_pal(dst, 4, VER + 1, 219, 0, 4, 0, 0);
  drawchr_ui_pal(dst, 5, VER + 1, 219, 0, 5, 0, 0);
  drawchr_ui_pal(dst, 6, VER + 1, 219, 0, 6, 0, 0);
  drawchr_ui_pal(dst, 7, VER + 1, 219, 0, 7, 0, 0);
  drawchr_ui_pal(dst, 8, VER + 1, 219, 0, 8, 0, 0);
  drawchr_ui_pal(dst, 9, VER + 1, 219, 0, 9, 0, 0);
  drawchr_ui_pal(dst, 10, VER + 1, 219, 0, 10, 0, 0);
  drawchr_ui_pal(dst, 11, VER + 1, 219, 0, 11, 0, 0);
  drawchr_ui_pal(dst, 12, VER + 1, 219, 0, 12, 0, 0);
  drawchr_ui_pal(dst, 13, VER + 1, 219, 0, 13, 0, 0);
  drawchr_ui_pal(dst, 14, VER + 1, 219, 0, 14, 0, 0);
  drawchr_ui_pal(dst, 15, VER + 1, 219, 0, 15, 0, 0);
 
  drawchr_ui_theme(dst, brush.palette[0], VER + 2, 49, 0, 3, 0, 0);
  drawchr_ui_theme(dst, brush.palette[1], VER + 2, 50, 1, 0, 0, 0);
  drawchr_ui_theme(dst, brush.palette[2], VER + 2, 51, 2, 0, 0, 0);
  drawchr_ui_theme(dst, brush.palette[3], VER + 2, 52, 3, 0, 0, 0);
}

void redraw(Uint32 *dst) {
	int x, y;
	clear(dst);
  drawui(dst);

	for(y = 0; y < VER; ++y)
		for(x = 0; x < HOR; ++x)
			drawchr_ui(dst, x, y, x + y * HOR);

	for(y = 0; y < VER; ++y)
		for(x = 0; x < HOR; ++x)
			drawchr(dst, HOR + x + 1, y, chrmap[x + y * HOR]);

	SDL_UpdateTexture(gTexture, NULL, dst, WIDTH * sizeof(Uint32));
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
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
    /* could convert these into fixed values earlier*/
    if (event->motion.x > (8 * PAD) * ZOOM &&
        event->motion.x < (8 * PAD + HOR * 8) * ZOOM &&
        event->motion.y > (8 * PAD) * ZOOM &&
        event->motion.y < (8 * PAD + VER * 8) * ZOOM) { 
      /* chr */
      brush.x = (event->motion.x / ZOOM - (8 * PAD)) / 8;
      brush.y = (event->motion.y / ZOOM - (8 * PAD)) / 8;
      if (brush.left) {
        brush.chr = (brush.x + brush.y * HOR);
      }
    } else if (event->motion.x > (8 * PAD * 1.5 + HOR * 8) * ZOOM &&
               event->motion.x < (8 * PAD * 1.5 + 2 * HOR * 8) * ZOOM &&
               event->motion.y > (8 * PAD) * ZOOM &&
               event->motion.y < (8 * PAD + VER * 8) * ZOOM) {
     /* map */
       brush.x = (event->motion.x / ZOOM - (8 * PAD * 1.5 + HOR * 8)) / 8;
       brush.y = (event->motion.y / ZOOM - (8 * PAD)) / 8;
       if (brush.left) {
         dropchr();
         dropclr();
         /* name these drop chr, drop clr and pick chr, pick clr */
       }
       if (brush.right) {
         liftchr();
         liftclr();
       }
    } else if (event->motion.x > (8 * PAD) * ZOOM &&
               event->motion.x < (8 * PAD + HOR * 8) * ZOOM &&
               event->motion.y > (8 * PAD * 1.5 + VER * 8) * ZOOM &&
               event->motion.y < (8 * PAD * 1.5 + (VER + 1) * 8) * ZOOM) {
        int hover_colour = (event->motion.x / ZOOM - (8 * PAD)) / 8; 
        if (brush.lshift || brush.rshift) {
          if (brush.left) brush.palette[2] = hover_colour;
          if (brush.right) brush.palette[3] = hover_colour;
        } else {
          if (brush.left) brush.palette[1] = hover_colour;
          if (brush.right) brush.palette[0] = hover_colour;
        }
    } else {
      /* outside either */
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
		SDL_WINDOW_SHOWN);
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

int main(int argc, char **argv) {
	FILE *fp;
	fp = fopen("spectrum-pi.chr", "r");
	fread(chrset, sizeof(chrset), 1, fp);

	int ticknext = 0;

	if (!init()) {
		return error("Init", "Failure");
	}

  brush.chr = 3;
  brush.palette[0] = 0;
  brush.palette[1] = 9;
  brush.palette[2] = 2;
  brush.palette[3] = 3;

	while (1) {
		int tick = SDL_GetTicks();
		SDL_Event event;
		if (tick < ticknext)
			SDL_Delay(ticknext - tick);
		ticknext = tick + (1000 / FPS) ;
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
        case SDL_SCANCODE_LSHIFT:
          brush.lshift = 0;
          break;
        case SDL_SCANCODE_RSHIFT:
          brush.rshift = 0;
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
 *
 * switch left/right mouse select.
 * improve region detection in mouse events.
 * clean up tile/colour interaction, there are a bunch of different functions for drawing in the nametable, out of it, with palette, with theme colours, etc.
 * show tile that will be painted when hovering over the canvas
 * highlight selected tile in chrbuf area
 */
