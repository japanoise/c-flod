#ifndef F2PLAYER_H
#define F2PLAYER_H

#include "../core/Soundblaster.h"
#include "../core/SBPlayer.h"
#include "F2Pattern.h"
#include "F2Instrument.h"
#include "F2Voice.h"

enum F2Player_Flags {
      UPDATE_PERIOD = 1,
      UPDATE_VOLUME = 2,
      UPDATE_PANNING = 4,
      UPDATE_TRIGGER = 8,
      UPDATE_ALL = 15,
      SHORT_RAMP = 32,
};

#define F2PLAYER_MAX_PATTERNS 4
#define F2PLAYER_MAX_INSTRUMENTS 4
#define F2PLAYER_MAX_VOICES 4

/*
inheritance
object
	-> EventDispatcher
		->CorePlayer
			->SBPlayer
				->F2Player
*/
struct F2Player {
	struct SBPlayer super;
	struct F2Pattern patterns[F2PLAYER_MAX_PATTERNS]; // Vector
	struct F2Instrument instruments[F2PLAYER_MAX_INSTRUMENTS]; // Vector
	int linear;
	struct F2Voice voices[F2PLAYER_MAX_VOICES];// Vector
	int order;
	int position;
	int nextOrder;
	int nextPosition;
	struct F2Pattern *pattern;
	int patternDelay;
	int patternOffset;
	int complete;
};

void F2Player_defaults(struct F2Player* self);
void F2Player_ctor(struct F2Player* self, struct Soundblaster *mixer);
struct F2Player* F2Player_new(struct Soundblaster *mixer);

#endif
