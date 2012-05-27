#ifndef BPPLAYER_H
#define BPPLAYER_H

#include "../core/Amiga.h"
#include "../core/AmigaRow.h"
#include "../core/AmigaPlayer.h"
#include "BPVoice.h"
#include "BPStep.h"
#include "BPSample.h"

#define BPPLAYER_MAX_TRACKS 4
#define BPPLAYER_MAX_PATTERNS 4

//fixed
#define BPPLAYER_MAX_SAMPLES 16
#define BPPLAYER_MAX_BUFFER 128
#define BPPLAYER_MAX_VOICES 4

enum BPPlayerVersion {
      BPSOUNDMON_V1 = 1,
      BPSOUNDMON_V2 = 2,
      BPSOUNDMON_V3 = 3,
};

struct BPPlayer {
	struct AmigaPlayer super;
	//tracks      : Vector.<BPStep>,
	struct BPStep tracks[BPPLAYER_MAX_TRACKS];
	//patterns    : Vector.<AmigaRow>,
	struct AmigaRow patterns[BPPLAYER_MAX_PATTERNS];	
	//samples     : Vector.<BPSample>,
	struct BPSample samples[BPPLAYER_MAX_SAMPLES];
	int length;
	//buffer      : Vector.<int>,
	int buffer[BPPLAYER_MAX_BUFFER];
	//voices      : Vector.<BPVoice>,
	struct BPVoice voices[BPPLAYER_MAX_VOICES];
	int trackPos;
	int patternPos;
	int nextPos;
	int jumpFlag;
	int repeatCtr;
	int arpeggioCtr;
	int vibratoPos;
};

void BPPlayer_defaults(struct BPPlayer* self);
void BPPlayer_ctor(struct BPPlayer* self, struct Amiga *amiga);
struct BPPlayer* BPPlayer_new(struct Amiga *amiga);

#endif
