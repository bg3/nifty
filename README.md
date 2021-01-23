# nifty
Charset tilemap/art editor

# Nifty

Nifty is a tiny paint tool for tile art. It uses NES/Nasu .chr files as tile data.

## Getting Started

Nifty requires [SDL2](https://wiki.libsdl.org/).

To compile:
```
cc nifty.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -L/usr/local/lib -lSDL2 -o nifty
```

## To do list

* Loading different character sets
* Saving canvas as editable tile data
* Exporting canvas as image
* Highlight currently selected character
* Preview current tile/colours under mouse in canvas area
* Keyboard commands:
  * Erase
  * Switch fg/bg colour

## Contributions

Feedback welcome. The code needs a clean up, but I'll work on that once I've figured out the feature set.

## Controls

### In character set (left)
* Left click to select character to paint with

### On canvas area (right)
* Left click to paint character
* Right click to lift (eyedropper) character and colours

### In colour palette
* Left click - colour 2 (fg)
* Right click - colour 1 (bg)
* Shift-left click - colour 4
* Shift-right click - colour 3

## Acknowledgments

* Nifty is based on [Nasu](https://git.sr.ht/~rabbits/nasu/) by [Hundred Rabbits](https://100r.co/site/home.html)
* The included character set, Spectrum PI is by me and based on the ZX Spectrum character set.
