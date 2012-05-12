﻿/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 4.1 - 2012/04/09

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 	LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/

#include "Amiga.h"

void Amiga_defaults(struct Amiga* self) {
	self->model = MODEL_A1200;
	self->loopLen = 4;
	self->clock = 0.0;
	self->master = 0.00390625;
}

struct Amiga* Amiga_new(void) {
	struct Amiga *self = malloc(sizeof(Amiga));
	if(self) {
		Amiga_defaults(self);
		Amiga_ctor(self);
	}
	return self;
}

void Amiga_ctor(struct Amiga* self) {
      super();
      self->bufferSize = 8192;
      self->filter = AmigaFilter_new();
      self->channels = new Vector.<AmigaChannel>(4, true);

      self->channels[0] = AmigaChannel_new(0);
      self->channels[0].next = self->channels[1] = AmigaChannel_new(1);
      self->channels[1].next = self->channels[2] = AmigaChannel_new(2);
      self->channels[2].next = self->channels[3] = AmigaChannel_new(3);
}

void Amiga_set_volume(struct Amiga* self, int value) {
	if (value > 0) {
		if (value > 64) value = 64;
		self->master = ((float) value / 64) * 0.00390625;
	} else {
		self->master = 0.0;
	}
}

// pointer default value: -1
int Amiga_store(struct Amiga* self, struct ByteArray *stream, int len, int pointer) {
	int add = 0; 
	int i = 0;
	int pos = stream->position;
	int start = self->memory->length;
	int total = 0;

	if (pointer > -1) ByteArray_set_position(stream, pointer);
	total = stream->position + len;

	if (total >= stream->length) {
		add = total - stream->length;
		len = stream->length - stream->position;
	}

	for (i = start, len += start; i < len; ++i)
		self->memory[i] = stream->readByte();

	self->memory->length += add;
	if (pointer > -1) ByteArray_set_position(stream, pos);
	return start;
}

//override
void Amiga_initialize(struct Amiga* self) {
	self->super->initialize();
	self->wave->clear();
	self->filter->initialize();

	if (!self->memory->fixed) {
		self->loopPtr = self->memory->length;
		self->memory->length += self->loopLen;
		self->memory->fixed = true;
	}

	self->channels[0]->initialize();
	self->channels[1]->initialize();
	self->channels[2]->initialize();
	self->channels[3]->initialize();
}

//override
void Amiga_reset(struct Amiga* self) {
      self->memory = new Vector.<int>();
}

    //override
void Amiga_fast(struct Amiga* self, e:SampleDataEvent) {
	struct AmigaChannel *chan = NULL;
	struct ByteArray *data = e->data;
	int i = 0;
	number lvol = NAN;
	int mixed = 0;
	int mixLen = 0;
	int mixPos = 0;
	number rvol = NAN;
	struct Sample *sample = NULL;
	int size = self->bufferSize;
	number speed = NAN;
	int toMix = 0;
	number value = NAN;

	if (self->completed) {
		if (!self->remains) return;
		size = self->remains;
	}

	while (mixed < size) {
		if (!self->samplesLeft) {
			self->player->process();
			self->samplesLeft = self->samplesTick;

			if (self->completed) {
				size = mixed + self->samplesTick;

				if (size > self->bufferSize) {
					self->remains = size - self->bufferSize;
					size = self->bufferSize;
				}
			}
		}

		toMix = self->samplesLeft;
		if ((mixed + toMix) >= size) toMix = size - mixed;
		mixLen = mixPos + toMix;
		chan = self->channels[0];

		while (chan) {
			sample = self->buffer[mixPos];

			if (chan->audena && chan->audper > 60) {
				if (chan->mute) {
					chan->ldata = 0.0;
					chan->rdata = 0.0;
				}

				speed = chan->audper / self->clock;

				value = chan->audvol * self->master;
				lvol = value * (1 - chan->level);
				rvol = value * (1 + chan->level);

				for (i = mixPos; i < mixLen; ++i) {
					if (chan->delay) {
						chan->delay--;
					} else if (--chan->timer < 1.0) { 
						if (!chan->mute) {
							value = self->memory[chan->audloc] * 0.0078125;
							chan->ldata = value * lvol;
							chan->rdata = value * rvol;
						}

						chan->audloc++;
						chan->timer += speed;

						if (chan->audloc >= chan->audcnt) {
							chan->audloc = chan->pointer;
							chan->audcnt = chan->pointer + chan->length;
						}
					}

					sample->l += chan->ldata;
					sample->r += chan->rdata;
					sample = sample->next;
				}
			} else {
				for (i = mixPos; i < mixLen; ++i) {
					sample->l += chan->ldata;
					sample->r += chan->rdata;
					sample = sample->next;
				}
			}
			chan = chan->next;
		}

		mixPos = mixLen;
		mixed += toMix;
		self->samplesLeft -= toMix;
	}

	sample = self->buffer[0];

	if (self->player->record) {
		for (i = 0; i < size; ++i) {
			self->filter->process(model, sample);

			self->wave->writeShort(int(sample->l * (sample->l < 0 ? 32768 : 32767)));
			self->wave->writeShort(int(sample->r * (sample->r < 0 ? 32768 : 32767)));

			data->writeFloat(sample->l);
			data->writeFloat(sample->r);

			sample->l = sample->r = 0.0;
			sample = sample->next;
		}
	} else {
		for (i = 0; i < size; ++i) {
		self->filter->process(model, sample);

		data->writeFloat(sample->l);
		data->writeFloat(sample->r);

		sample->l = sample->r = 0.0;
		sample = sample->next;
		}
	}
}

