#ifndef SDL_H

#define SDL_H

// SDL main header (dummy file) for compatibility

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define LOAD_GIF
#define LOAD_PCX
#define LOAD_XPM
#define LOAD_TGA

#include "SDL_types.h"
#include "SDL_error.h"
#include "SDL_rwops.h"
#include "SDL_video.h"

#endif
