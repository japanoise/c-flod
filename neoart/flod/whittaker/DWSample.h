#ifndef DWSAMPLE_H
#define DWSAMPLE_H

#include "../core/AmigaSample.h"

// extends AmigaSample
struct DWSample {
	struct AmigaSample super;
	int relative;
	int finetune;
};

void DWSample_defaults(struct DWSample* self);

void DWSample_ctor(struct DWSample* self);

struct DWSample* DWSample_new(void);

#endif
