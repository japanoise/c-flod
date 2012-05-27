#ifndef RHPLAYER_H
#define RHPLAYER_H

#include "../core/Amiga.h"
#include "../core/AmigaPlayer.h"
#include "RHSong.h"
#include "RHVoice.h"
#include "RHSample.h"

#define RHPLAYER_MAX_SONGS 4
#define RHPLAYER_MAX_SAMPLES 4

//fixed
#define RHPLAYER_MAX_VOICES 4

struct RHPlayer {
	struct AmigaPlayer super;
	struct RHSong songs[RHPLAYER_MAX_SONGS];//    : Vector.<RHSong>,
	struct RHSample samples[RHPLAYER_MAX_SAMPLES];//  : Vector.<RHSample>,
	struct RHSong *song;
	int periods;
	int vibrato;
	struct RHVoice voices[RHPLAYER_MAX_VOICES]; //   : Vector.<RHVoice>,
	struct ByteArray *stream;
	int complete;
};

void RHPlayer_defaults(struct RHPlayer* self);
void RHPlayer_ctor(struct RHPlayer* self, struct Amiga *amiga);
struct RHPlayer* RHPlayer_new(struct Amiga *amiga);


#endif
