#ifndef AMIGA_H
#define AMIGA_H

#include "../flod.h"
#include "CoreMixer.h"
#include "AmigaFilter.h"
#include "AmigaChannel.h"

// tune this value to the biggest filesize
// of mods you want to support. a 85 KB file needs approx 80KB amiga mem.
// TODO: eventually there's a way to point to the original file buffer
// to make this unnecessary.
// for a desktop player it shouldnt be a problem to set this to something
// huge like 512 kb
// maybe make this so the caller can provide a buffer.
#define AMIGA_MAX_MEMORY_KB 86
#define AMIGA_MAX_MEMORY ((AMIGA_MAX_MEMORY_KB * 1024) / sizeof(char))

// this is hardwired.
#define AMIGA_MAX_CHANNELS 4

enum AmigaModel {
      MODEL_A500 = 0,
      MODEL_A1200 = 1,
};
//public final class Amiga extends CoreMixer {
	  
struct Amiga {
	struct CoreMixer super;
	struct AmigaFilter* filter;
	enum AmigaModel model;
	//int *memory; // Vector
	signed char memory[AMIGA_MAX_MEMORY];
	unsigned vector_count_memory;
	//struct AmigaChannel *channels; //Vector
	struct AmigaChannel channels[AMIGA_MAX_CHANNELS];
	int loopPtr;
	int loopLen;
	Number clock;
	Number master;
	int memory_fixed;
};

void Amiga_defaults(struct Amiga* self);
void Amiga_ctor(struct Amiga* self);
struct Amiga* Amiga_new(void);
void Amiga_set_volume(struct Amiga* self, int value);
int Amiga_store(struct Amiga* self, struct ByteArray *stream, int len, int pointer);
void Amiga_initialize(struct Amiga* self);
void Amiga_reset(struct Amiga* self);
void Amiga_fast(struct Amiga* self, struct SampleDataEvent *e);

#endif
