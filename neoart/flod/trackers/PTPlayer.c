﻿/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 4.1 - 2012/04/16

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/

#include "PTPlayer.h"
#include "../flod_internal.h"

const unsigned short PERIODS[] = {
        856,808,762,720,678,640,604,570,538,508,480,453,
        428,404,381,360,339,320,302,285,269,254,240,226,
        214,202,190,180,170,160,151,143,135,127,120,113,0,
        850,802,757,715,674,637,601,567,535,505,477,450,
        425,401,379,357,337,318,300,284,268,253,239,225,
        213,201,189,179,169,159,150,142,134,126,119,113,0,
        844,796,752,709,670,632,597,563,532,502,474,447,
        422,398,376,355,335,316,298,282,266,251,237,224,
        211,199,188,177,167,158,149,141,133,125,118,112,0,
        838,791,746,704,665,628,592,559,528,498,470,444,
        419,395,373,352,332,314,296,280,264,249,235,222,
        209,198,187,176,166,157,148,140,132,125,118,111,0,
        832,785,741,699,660,623,588,555,524,495,467,441,
        416,392,370,350,330,312,294,278,262,247,233,220,
        208,196,185,175,165,156,147,139,131,124,117,110,0,
        826,779,736,694,655,619,584,551,520,491,463,437,
        413,390,368,347,328,309,292,276,260,245,232,219,
        206,195,184,174,164,155,146,138,130,123,116,109,0,
        820,774,730,689,651,614,580,547,516,487,460,434,
        410,387,365,345,325,307,290,274,258,244,230,217,
        205,193,183,172,163,154,145,137,129,122,115,109,0,
        814,768,725,684,646,610,575,543,513,484,457,431,
        407,384,363,342,323,305,288,272,256,242,228,216,
        204,192,181,171,161,152,144,136,128,121,114,108,0,
        907,856,808,762,720,678,640,604,570,538,508,480,
        453,428,404,381,360,339,320,302,285,269,254,240,
        226,214,202,190,180,170,160,151,143,135,127,120,0,
        900,850,802,757,715,675,636,601,567,535,505,477,
        450,425,401,379,357,337,318,300,284,268,253,238,
        225,212,200,189,179,169,159,150,142,134,126,119,0,
        894,844,796,752,709,670,632,597,563,532,502,474,
        447,422,398,376,355,335,316,298,282,266,251,237,
        223,211,199,188,177,167,158,149,141,133,125,118,0,
        887,838,791,746,704,665,628,592,559,528,498,470,
        444,419,395,373,352,332,314,296,280,264,249,235,
        222,209,198,187,176,166,157,148,140,132,125,118,0,
        881,832,785,741,699,660,623,588,555,524,494,467,
        441,416,392,370,350,330,312,294,278,262,247,233,
        220,208,196,185,175,165,156,147,139,131,123,117,0,
        875,826,779,736,694,655,619,584,551,520,491,463,
        437,413,390,368,347,328,309,292,276,260,245,232,
        219,206,195,184,174,164,155,146,138,130,123,116,0,
        868,820,774,730,689,651,614,580,547,516,487,460,
        434,410,387,365,345,325,307,290,274,258,244,230,
        217,205,193,183,172,163,154,145,137,129,122,115,0,
        862,814,768,725,684,646,610,575,543,513,484,457,
        431,407,384,363,342,323,305,288,272,256,242,228,
        216,203,192,181,171,161,152,144,136,128,121,114,0
};

const unsigned char VIBRATO[] = {
          0, 24, 49, 74, 97,120,141,161,180,197,212,224,
        235,244,250,253,255,253,250,244,235,224,212,197,
        180,161,141,120, 97, 74, 49, 24
};

const unsigned char FUNKREP[] = {
        0,5,6,7,8,10,11,13,16,19,22,26,32,43,64,128
};


void PTPlayer_defaults(struct PTPlayer* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

//amiga def. value = NULL
void PTPlayer_ctor(struct PTPlayer* self, struct Amiga *amiga) {
	CLASS_CTOR_DEF(PTPlayer);
	// original constructor code goes here
	AmigaPlayer_ctor(&self->super, amiga);
	
	unsigned i;
	for (i = 0; i < PTPLAYER_MAX_VOICES; i++) {
		PTVoice_ctor(&self->voices[i], i);
		if(i) self->voices[i].next = &self->voices[i];
	}
	
	//vtable
	self->super.super.set_force = PTPlayer_set_force;
	self->super.super.process = PTPlayer_process;
	self->super.super.loader = PTPlayer_loader;
	self->super.super.initialize = PTPlayer_initialize;
}

struct PTPlayer* PTPlayer_new(struct Amiga *amiga) {
	CLASS_NEW_BODY(PTPlayer, amiga);
}

//override
void PTPlayer_set_force(struct PTPlayer* self, int value) {
	if (value < PROTRACKER_10)
		value = PROTRACKER_10;
	else if (value > PROTRACKER_12)
		value = PROTRACKER_12;

	self->version = value;

	if (value < PROTRACKER_11) self->vibratoDepth = 6;
		else self->vibratoDepth = 7;
}

//override
void PTPlayer_process(struct PTPlayer* self) {
	struct AmigaChannel *chan = NULL;
	int i = 0; 
	int pattern = 0;
	struct PTRow *row = NULL;
	struct PTSample *sample = NULL;
	int value = 0;
	struct PTVoice *voice = self->voices[0];

	if (!self->tick) {
		if (self->patternDelay) {
			effects();
		} else {
			pattern = self->track[self->trackPos] + self->patternPos;

			while (voice) {
				chan = voice->channel;
				voice->enabled = 0;

				if (!voice->step) chan->period = voice->period;

				row = self->patterns[pattern + voice->index];
				voice->step   = row->step;
				voice->effect = row->effect;
				voice->param  = row->param;

				if (row->sample) {
					sample = voice->sample = self->samples[row->sample];

					voice->pointer  = sample->pointer;
					voice->length   = sample->length;
					voice->loopPtr  = voice->funkWave = sample->loopPtr;
					voice->repeat   = sample->repeat;
					voice->finetune = sample->finetune;

					chan->volume = voice->volume = sample->volume;
				} else {
					sample = voice->sample;
				}

				if (!row->note) {
					moreEffects(voice);
					voice = voice->next;
					continue;
				} else {
					if ((voice->step & 0x0ff0) == 0x0e50) {
						voice->finetune = (voice->param & 0x0f) * 37;
					} else if (voice->effect == 3 || voice->effect == 5) {
						if (row->note == voice->period) {
							voice->portaPeriod = 0;
						} else {
							i = voice->finetune;
							value = i + 37;

							for (i; i < value; ++i)
								if (row->note >= PERIODS[i]) break;

							if (i == value) value--;

							if (i > 0) {
								value = (voice->finetune / 37) & 8;
								if (value) i--;
							}

							voice->portaPeriod = PERIODS[i];
							voice->portaDir = row->note > voice->portaPeriod ? 0 : 1;
						}
					} else if (voice->effect == 9) {
						moreEffects(voice);
					}
				}

				for (i = 0; i < 37; ++i)
					if (row->note >= PERIODS[i]) break;

				voice->period = PERIODS[int(voice->finetune + i)];

				if ((voice->step & 0x0ff0) == 0x0ed0) {
					if (voice->funkSpeed) updateFunk(voice);
					extended(voice);
					voice = voice->next;
					continue;
				}

				if (voice->vibratoWave < 4) voice->vibratoPos = 0;
				if (voice->tremoloWave < 4) voice->tremoloPos = 0;

				chan->enabled = 0;
				chan->pointer = voice->pointer;
				chan->length  = voice->length;
				chan->period  = voice->period;

				voice->enabled = 1;
				moreEffects(voice);
				voice = voice->next;
			}
			voice = self->voices[0];

			while (voice) {
				chan = voice->channel;
				if (voice->enabled) chan->enabled = 1;

				chan->pointer = voice->loopPtr;
				chan->length  = voice->repeat;

				voice = voice->next;
			}
		}
	} else {
		effects();
	}

	if (++(self->tick) == self->speed) {
		self->tick = 0;
		self->patternPos += 4;

		if (self->patternDelay)
			if (--self->patternDelay) self->patternPos -= 4;

		if (self->patternBreak) {
			self->patternBreak = 0;
			self->patternPos = self->breakPos;
			self->breakPos = 0;
		}

		if (self->patternPos == 256 || self->jumpFlag) {
			self->patternPos = self->breakPos;
			self->breakPos = 0;
			self->jumpFlag = 0;

			if (++(self->trackPos) == length) {
				self->trackPos = 0;
				self->amiga->complete = 1;
			}
		}
	}
}

//override
void PTPlayer_initialize(struct PTPlayer* self) {
	struct PTVoice *voice = voices[0];

	self->tempo        = 125;
	self->speed        = 6;
	self->trackPos     = 0;
	self->patternPos   = 0;
	self->patternBreak = 0;
	self->patternDelay = 0;
	self->breakPos     = 0;
	self->jumpFlag     = 0;

	self->super->initialize();
	self->force = version;

	while (voice) {
		voice->initialize();
		voice->channel = self->amiga->channels[voice->index];
		voice->sample  = self->samples[0];
		voice = voice->next;
	}
}

//override
void PTPlayer_loader(struct PTPlayer* self, struct ByteArray *stream) {
	int higher = 0; 
	int i = 0; 
	char *id; //String;
	int j = 0;
	struct PTRow *row = NULL;
	struct PTSample *sample = NULL;
	int size = 0;
	int value = 0;
	
	if (stream->length < 2106) return;

	stream->position = 1080;
	id = stream->readMultiByte(4, ENCODING);
	if (id != "M->K." && id != "M!K!") return;

	stream->position = 0;
	self->title = stream->readMultiByte(20, ENCODING);
	self->version = PROTRACKER_10;
	stream->position += 22;

	for (i = 1; i < 32; ++i) {
		value = stream->readUnsignedShort();

		if (!value) {
			samples[i] = null;
			stream->position += 28;
			continue;
		}

		sample = new PTSample();
		stream->position -= 24;

		sample->name = stream->readMultiByte(22, ENCODING);
		sample->length = sample->realLen = value << 1;
		stream->position += 2;

		sample->finetune = stream->readUnsignedByte() * 37;
		sample->volume   = stream->readUnsignedByte();
		sample->loop     = stream->readUnsignedShort() << 1;
		sample->repeat   = stream->readUnsignedShort() << 1;

		stream->position += 22;
		sample->pointer = size;
		size += sample->length;
		self->samples[i] = sample;
	}

	stream->position = 950;
	self->length = stream->readUnsignedByte();
	stream->position++;

	for (i = 0; i < 128; ++i) {
		value = stream->readUnsignedByte() << 8;
		self->track[i] = value;
		if (value > higher) higher = value;
	}

	stream->position = 1084;
	higher += 256;
	patterns = new Vector.<PTRow>(higher, true);

	for (i = 0; i < higher; ++i) {
		row = new PTRow();
		row->step = value = stream->readUnsignedInt();

		row->note   = (value >> 16) & 0x0fff;
		row->effect = (value >>  8) & 0x0f;
		row->sample = (value >> 24) & 0xf0 | (value >> 12) & 0x0f;
		row->param  = value & 0xff;

		self->patterns[i] = row;

		if (row->sample > 31 || !samples[row->sample]) row->sample = 0;

		if (row->effect == 15 && row->param > 31)
		self->version = PROTRACKER_11;

		if (row->effect == 8)
		self->version = PROTRACKER_12;
	}

	self->amiga->store(stream, size);

	for (i = 1; i < 32; ++i) {
		sample = samples[i];
		if (!sample) continue;

		if (sample->loop || sample->repeat > 4) {
			sample->loopPtr = sample->pointer + sample->loop;
			sample->length  = sample->loop + sample->repeat;
		} else {
			sample->loopPtr = amiga->memory->length;
			sample->repeat  = 2;
		}

		size = sample->pointer + 2;
		for (j = sample->pointer; j < size; ++j) 
			amiga->memory[j] = 0;
	}

	sample = new PTSample();
	sample->pointer = sample->loopPtr = amiga->memory->length;
	sample->length  = sample->repeat  = 2;
	self->samples[0] = sample;
}

void PTPlayer_effects(struct PTPlayer* self) {
	struct AmigaChannel *chan;
	int i = 0;
	int position = 0;
	int slide = 0;
	int value = 0;
	struct PTVoice *voice = self->voices[0];
	int wave = 0;

	while (voice) {
		chan = voice->channel;
		if (voice->funkSpeed) updateFunk(voice);

		if ((voice->step & 0x0fff) == 0) {
			chan->period = voice->period;
			voice = voice->next;
			continue;
		}

		switch (voice->effect) {
			case 0:   //arpeggio
				value = tick % 3;

				if (!value) {
					chan->period = voice->period;
					voice = voice->next;
					continue;
				}

				if (value == 1) value = voice->param >> 4;
				else value = voice->param & 0x0f;

				i = voice->finetune;
				position = i + 37;

				for (i; i < position; ++i)
					if (voice->period >= PERIODS[i]) {
						chan->period = PERIODS[int(i + value)];
						break;
					}
				break;
			case 1:   //portamento up
				voice->period -= voice->param;
				if (voice->period < 113) voice->period = 113;
				chan->period = voice->period;
				break;
			case 2:   //portamento down
				voice->period += voice->param;
				if (voice->period > 856) voice->period = 856;
				chan->period = voice->period;
				break;
			case 3:   //tone portamento
			case 5:   //tone portamento + volume slide
				if (voice->effect == 5) {
					slide = 1;
				} else {
					voice->portaSpeed = voice->param;
					voice->param = 0;
				}

				if (voice->portaPeriod) {
					if (voice->portaDir) {
						voice->period -= voice->portaSpeed;

						if (voice->period <= voice->portaPeriod) {
							voice->period = voice->portaPeriod;
							voice->portaPeriod = 0;
						}
					} else {
						voice->period += voice->portaSpeed;

						if (voice->period >= voice->portaPeriod) {
							voice->period = voice->portaPeriod;
							voice->portaPeriod = 0;
						}
					}

					if (voice->glissando) {
						i = voice->finetune;
						value = i + 37;

						for (i; i < value; ++i)
							if (voice->period >= PERIODS[i]) break;

						if (i == value) i--;
						chan->period = PERIODS[i];
					} else {
						chan->period = voice->period;
					}
				}
				break;
			case 4:   //vibrato
			case 6:   //vibrato + volume slide
				if (voice->effect == 6) {
					slide = 1;
				} else if (voice->param) {
					value = voice->param & 0x0f;
					if (value) voice->vibratoParam = (voice->vibratoParam & 0xf0) | value;
					value = voice->param & 0xf0;
					if (value) voice->vibratoParam = (voice->vibratoParam & 0x0f) | value;
				}

				position = (voice->vibratoPos >> 2) & 31;
				wave = voice->vibratoWave & 3;

				if (wave) {
					value = 255;
					position <<= 3;

					if (wave == 1) {
						if (voice->vibratoPos > 127) value -= position;
						else value = position;
					}
				} else {
					value = VIBRATO[position];
				}

				value = ((voice->vibratoParam & 0x0f) * value) >> vibratoDepth;

				if (voice->vibratoPos > 127) chan->period = voice->period - value;
				else chan->period = voice->period + value;

				value = (voice->vibratoParam >> 2) & 60;
				voice->vibratoPos = (voice->vibratoPos + value) & 255;
				break;
			case 7:   //tremolo
				chan->period = voice->period;

				if (voice->param) {
					value = voice->param & 0x0f;
					if (value) voice->tremoloParam = (voice->tremoloParam & 0xf0) | value;
					value = voice->param & 0xf0;
					if (value) voice->tremoloParam = (voice->tremoloParam & 0x0f) | value;
				}

				position = (voice->tremoloPos >> 2) & 31;
				wave = voice->tremoloWave & 3;

				if (wave) {
					value = 255;
					position <<= 3;

					if (wave == 1) {
						if (voice->tremoloPos > 127) value -= position;
						else value = position;
					}
				} else {
					value = VIBRATO[position];
				}

				value = ((voice->tremoloParam & 0x0f) * value) >> 6;

				if (voice->tremoloPos > 127) chan->volume = voice->volume - value;
				else chan->volume = voice->volume + value;

				value = (voice->tremoloParam >> 2) & 60;
				voice->tremoloPos = (voice->tremoloPos + value) & 255;
				break;
			case 10:  //volume slide
				slide = 1;
				break;
			case 14:  //extended effects
				extended(voice);
				break;
			default:
				break;
		}

		if (slide) {
			slide = 0;
			value = voice->param >> 4;

			if (value) voice->volume += value;
			else voice->volume -= voice->param & 0x0f;

			if (voice->volume < 0) voice->volume = 0;
			else if (voice->volume > 64) voice->volume = 64;

			chan->volume = voice->volume;
		}
		voice = voice->next;
	}
}

void PTPlayer_moreEffects(struct PTPlayer* self, struct PTVoice *voice) {
	struct AmigaChannel *chan = voice->channel;
	int value = 0;
	
	if (voice->funkSpeed) updateFunk(voice);

	switch (voice->effect) {
		case 9:   //sample offset
			if (voice->param) voice->offset = voice->param;
			value = voice->offset << 8;

			if (value >= voice->length) {
				voice->length = 2;
			} else {
				voice->pointer += value;
				voice->length  -= value;
			}
			break;
		case 11:  //position jump
			self->trackPos = voice->param - 1;
			self->breakPos = 0;
			self->jumpFlag = 1;
			break;
		case 12:  //set volume
			voice->volume = voice->param;
			if (voice->volume > 64) voice->volume = 64;
			chan->volume = voice->volume;
			break;
		case 13:  //pattern break
			self->breakPos = ((voice->param >> 4) * 10) + (voice->param & 0x0f);

			if (self->breakPos > 63) self->breakPos = 0;
			else self->breakPos <<= 2;

			self->jumpFlag = 1;
			break;
		case 14:  //extended effects
			extended(voice);
			break;
		case 15:  //set speed
			if (!voice->param) return;

			if (voice->param < 32) speed = voice->param;
			else self->amiga->samplesTick = 110250 / voice->param;

			self->tick = 0;
			break;
		default:
			break;
	}
}

void PTPlayer_extended(struct PTPlayer* self, struct PTVoice *voice) {
	struct AmigaChannel *chan = voice->channel;
	int effect = voice->param >> 4;
	int i = 0;
	int len = 0;
	memory:Vector.<int>;
	int param = voice->param & 0x0f;

	switch (effect) {
		case 0:   //set filter
			self->amiga->filter->active = param;
			break;
		case 1:   //fine portamento up
			if (self->tick) return;
			voice->period -= param;
			if (voice->period < 113) voice->period = 113;
			chan->period = voice->period;
			break;
		case 2:   //fine portamento down
			if (self->tick) return;
			voice->period += param;
			if (voice->period > 856) voice->period = 856;
			chan->period = voice->period;
			break;
		case 3:   //glissando control
			voice->glissando = param;
			break;
		case 4:   //vibrato control
			voice->vibratoWave = param;
			break;
		case 5:   //set finetune
			voice->finetune = param * 37;
			break;
		case 6:   //pattern loop
			if (self->tick) return;

			if (param) {
				if (voice->loopCtr) voice->loopCtr--;
				else voice->loopCtr = param;

				if (voice->loopCtr) {
					self->breakPos = voice->loopPos << 2;
					self->patternBreak = 1;
				}
			} else {
				voice->loopPos = self->patternPos >> 2;
			}
			break;
		case 7:   //tremolo control
			voice->tremoloWave = param;
			break;
		case 8:   //karplus strong
			len = voice->length - 2;
			memory = self->amiga->memory;

			for (i = voice->loopPtr; i < len;)
				memory[i] = (memory[i] + memory[++i]) * 0.5;

			memory[++i] = (memory[i] + memory[0]) * 0.5;
			break;
		case 9:   //retrig note
			if (self->tick || !param || !voice->period) return;
			if (self->tick % param) return;

			chan->enabled = 0;
			chan->pointer = voice->pointer;
			chan->length  = voice->length;
			chan->delay   = 30;

			chan->enabled = 1;
			chan->pointer = voice->loopPtr;
			chan->length  = voice->repeat;
			chan->period  = voice->period;
			break;
		case 10:  //fine volume up
			if (self->tick) return;
			voice->volume += param;
			if (voice->volume > 64) voice->volume = 64;
			chan->volume = voice->volume;
			break;
		case 11:  //fine volume down
			if (self->tick) return;
			voice->volume -= param;
			if (voice->volume < 0) voice->volume = 0;
			chan->volume = voice->volume;
			break;
		case 12:  //note cut
			if (self->tick == param) chan->volume = voice->volume = 0;
			break;
		case 13:  //note delay
			if (self->tick != param || !voice->period) return;

			chan->enabled = 0;
			chan->pointer = voice->pointer;
			chan->length  = voice->length;
			chan->delay   = 30;

			chan->enabled = 1;
			chan->pointer = voice->loopPtr;
			chan->length  = voice->repeat;
			chan->period  = voice->period;
			break;
		case 14:  //pattern delay
			if (self->tick || self->patternDelay) return;
			self->patternDelay = ++param;
			break;
		case 15:  //funk repeat or invert loop
			if (self->tick) return;
			voice->funkSpeed = param;
			if (param) updateFunk(voice);
			break;
		default:
			break;
	}
}

void PTPlayer_updateFunk(struct PTPlayer* self, struct PTVoice *voice) {
	struct AmigaChannel *chan = voice->channel;
	int p1 = 0; 
	int p2 = 0; 
	int value = FUNKREP[voice->funkSpeed];

	voice->funkPos += value;
	if (voice->funkPos < 128) return;
	voice->funkPos = 0;

	if (self->version == PROTRACKER_10) {
		p1 = voice->pointer + voice->sample->realLen - voice->repeat;
		p2 = voice->funkWave + voice->repeat;

		if (p2 > p1) {
			p2 = voice->loopPtr;
			chan->length = voice->repeat;
		}
		chan->pointer = voice->funkWave = p2;
	} else {
		p1 = voice->loopPtr + voice->repeat;
		p2 = voice->funkWave + 1;

		if (p2 >= p1) p2 = voice->loopPtr;

		self->amiga->memory[p2] = -self->amiga->memory[p2];
	}
}

