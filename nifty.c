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
	0xDDDDDD,
	0x333333,
	0x72DEC2,
	0x666666,
	0x222222};

Uint8 icons[][8] = {
	{0x38, 0x7c, 0xfe, 0xfe, 0xfe, 0x7c, 0x38, 0x00},
	{0x38, 0x44, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00},
	{0x02, 0x02, 0x04, 0x38, 0x40, 0x80, 0x80, 0x00},
	{0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00},
	{0xaa, 0x54, 0xaa, 0x54, 0xaa, 0x54, 0xaa, 0x00},
	{0x38, 0x7c, 0xee, 0xd6, 0xee, 0x7c, 0x38, 0x00},
	{0x44, 0xba, 0x44, 0x44, 0x44, 0xba, 0x44, 0x00},
	{0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00},
	{0xee, 0xaa, 0xee, 0x00, 0xee, 0xaa, 0xee, 0x00},
	{0x00, 0x00, 0x00, 0x82, 0x44, 0x38, 0x00, 0x00}, /* eye open */
	{0x00, 0x38, 0x44, 0x92, 0x28, 0x10, 0x00, 0x00}  /* eye closed */
};

Uint8 chrbuf[16 * 16 * 8 * 2];
Uint8 nametable[HOR * VER];


void randnametable() {
	int x, y, r;
	for (y = 0; y < VER; ++y) {
		for (x = 0; x < HOR; ++x) {
			r = rand() % 3;
			if (r == 0) {
				r = rand() % 256;
				nametable[x + y * HOR] = r;
			} else {
				nametable[x + y * HOR] = 0;
			}
		}
	}
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

void putpixel(Uint32 *dst, int x, int y, int color) {
	if(x >= 0 && x < WIDTH - 8 && y >= 0 && y < HEIGHT - 8)
		dst[(y + PAD * 8) * WIDTH + (x + PAD * 8)] = theme[color];
}

void drawchr(Uint32 *dst, int x, int y, int id, int palette) {
	int v, h, offset = id * 16;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int px = (x * 8) + (8 - h);
			int py = (y * 8) + v;
			int ch1 = chrbuf[offset + v];
			int ch2 = chrbuf[offset + v + 8];
			int clr = ((ch1 >> h) & 0x1) + (((ch2 >> h) & 0x1) << 1);
			putpixel(dst, px - 1, py, clr);
		}
}

void redraw(Uint32 *dst) {
	int x, y;
	clear(dst);
	for(y = 0; y < VER; ++y)
		for(x = 0; x < HOR; ++x)
			drawchr(dst, x, y, x + y * HOR, 0);
	for(y = 0; y < VER; ++y)
		for(x = 0; x < HOR; ++x)
			drawchr(dst, HOR + x + 1, y, nametable[x + y * HOR], 0);
	SDL_UpdateTexture(gTexture, NULL, dst, WIDTH * sizeof(Uint32));
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
}

void domouse(SDL_Event* event) {
  switch(event->type) {
	case SDL_MOUSEBUTTONUP:
  
  case SDL_MOUSEBUTTONDOWN:

  case SDL_MOUSEMOTION:
    /* convert event.x, y into window x, y
    // replace the character at this position.*/
    if (event->motion.x > (8 * PAD) * ZOOM &&
        event->motion.x < (8 * PAD + HOR * 8) * ZOOM &&
        event->motion.y > (8 * PAD) * ZOOM &&
        event->motion.y < (8 * PAD + VER * 8) * ZOOM) {
      /* chr */
      theme[1] = 0x009900;
    } else if (event->motion.x > (8 * PAD * 1.5 + HOR * 8) * ZOOM &&
               event->motion.x < (8 * PAD * 1.5 + 2 * HOR * 8) * ZOOM &&
               event->motion.y > (8 * PAD) * ZOOM &&
               event->motion.y < (8 * PAD + VER * 8) * ZOOM) {
      /* map */
      theme[1] = 0xFF0000; 
    } else {
      /* outside either */
      theme[1] = 0x333333;
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
	fread(chrbuf, sizeof(chrbuf), 1, fp);

	int ticknext = 0;

	if (!init()) {
		return error("Init", "Failure");
	}

	randnametable();

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
				case SDL_SCANCODE_R:
					randnametable();
					redraw(pixels);
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
