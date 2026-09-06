/* Minimal shim of miniaudio API to allow compilation and local testing.
   Replace this file with the official miniaudio.h when available to use
   the full featureset. This shim implements only the functions used by
   the project's AudioEngine wrapper and delegates playback to the
   platform fallback implemented in the project. */

#pragma once

#include <stdint.h>
#include <stddef.h>

typedef int ma_result;
typedef struct ma_engine ma_engine;
typedef struct ma_sound ma_sound;

#define MA_SUCCESS 0

struct ma_engine { int dummy; };
struct ma_sound { char dummy; };

// Simplified API used by the engine wrapper
ma_result ma_engine_init(ma_engine* pEngine, void* config);
void ma_engine_uninit(ma_engine* pEngine);

ma_result ma_sound_init_from_file(ma_engine* pEngine, const char* filename, ma_sound** ppSound);
void ma_sound_uninit(ma_sound* pSound);
ma_result ma_engine_play_sound(ma_engine* pEngine, ma_sound* pSound, ma_sound** ppSound);
void ma_sound_start(ma_sound* pSound);
void ma_sound_stop(ma_sound* pSound);
ma_result ma_sound_set_volume(ma_sound* pSound, float volume);
ma_result ma_sound_set_pitch(ma_sound* pSound, float pitch);
