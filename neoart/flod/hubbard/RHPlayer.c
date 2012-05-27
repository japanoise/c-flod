﻿/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 4.0 - 2012/03/09

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/

#include "RHPlayer.h"
#include "../flod_internal.h"

void RHPlayer_loader(struct RHPlayer* self, struct ByteArray *stream);
void RHPlayer_initialize(struct RHPlayer* self);
void RHPlayer_process(struct RHPlayer* self);

void RHPlayer_defaults(struct RHPlayer* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

void RHPlayer_ctor(struct RHPlayer* self, struct Amiga *amiga) {
	CLASS_CTOR_DEF(RHPlayer);
	// original constructor code goes here
	//super(amiga);
	AmigaPlayer_ctor(&self->super, amiga);
	
	/*voices = new Vector.<RHVoice>(4, true);
	voices[3] = new RHVoice(3,8);
	voices[3].next = voices[2] = new RHVoice(2,4);
	voices[2].next = voices[1] = new RHVoice(1,2);
	voices[1].next = voices[0] = new RHVoice(0,1); */
	
	unsigned i = RHPLAYER_MAX_VOICES;
	while(1) {
		RHVoice_ctor(&self->voices[i], i, 1 << i);
		if(i) self->voices[i].next = &self->voices[i - 1];
		else break;
		i--;
	}
	
	
	//vtable
	self->super.super.loader = RHPlayer_loader;
	self->super.super.initialize = RHPlayer_initialize;
	self->super.super.process = RHPlayer_process;
	
	self->super.super.min_filesize = 8;
}

struct RHPlayer* RHPlayer_new(struct Amiga *amiga) {
	CLASS_NEW_BODY(RHPlayer, amiga);
}


//override
void RHPlayer_process(struct RHPlayer* self) {
	struct AmigaChannel *chan = 0;
	int loop = 0;
	struct RHSample *sample = 0;
	int value = 0; 
	struct RHVoice *voice = &self->voices[3];

	while (voice) {
		chan = voice->channel;
		stream->position = voice->patternPos;
		sample = voice->sample;

		if (!voice->busy) {
			voice->busy = 1;

			if (sample->loopPtr == 0) {
				chan->pointer = amiga->loopPtr;
				chan->length  = amiga->loopLen;
			} else if (sample->loopPtr > 0) {
				chan->pointer = sample->pointer + sample->loopPtr;
				chan->length  = sample->length  - sample->loopPtr;
			}
		}

		if (--voice->tick == 0) {
			voice->flags = 0;
			loop = 1;

			while (loop) {
				value = stream->readByte();

				if (value < 0) {
					switch (value) {
						case -121:
							if (variant == 3) voice->volume = stream->readUnsignedByte();
							break;
						case -122:
							if (variant == 4) voice->volume = stream->readUnsignedByte();
							break;
						case -123:
							if (variant > 1) amiga->complete = 1;
							break;
						case -124:
							stream->position = voice->trackPtr + voice->trackPos;
							value = stream->readUnsignedInt();
							voice->trackPos += 4;

							if (!value) {
								stream->position = voice->trackPtr;
								value = stream->readUnsignedInt();
								voice->trackPos = 4;

								if (!loopSong) {
									complete &= ~(voice->bitFlag);
									if (!complete) amiga->complete = 1;
								}
							}

							stream->position = value;
							break;
						case -125:
							if (variant == 4) voice->flags |= 4;
							break;
						case -126:
							voice->tick = song->speed * stream->readByte();
							voice->patternPos = stream->position;

							chan->pointer = amiga->loopPtr;
							chan->length  = amiga->loopLen;
							loop = 0;
							break;
						case -127:
							voice->portaSpeed = stream->readByte();
							voice->flags |= 1;
							break;
						case -128:
							value = stream->readByte();
							if (value < 0) value = 0;
							voice->sample = sample = samples[value];
							voice->vibratoPtr = vibrato + sample->vibrato;
							voice->vibratoPos = voice->vibratoPtr;
							break;
						default:
							break;
					}
				} else {
					voice->tick = song->speed * value;
					voice->note = stream->readByte();
					voice->patternPos = stream->position;

					voice->synthPos = sample->loPos;
					voice->vibratoPos = voice->vibratoPtr;

					chan->pointer = sample->pointer;
					chan->length  = sample->length;
					chan->volume  = (voice->volume) ? voice->volume : sample->volume;

					stream->position = periods + (voice->note << 1);
					value = stream->readUnsignedShort() * sample->relative;
					chan->period = voice->period = (value >> 10);

					chan->enabled = 1;
					voice->busy = loop = 0;
				}
			}
		} else {
			if (voice->tick == 1) {
				if (variant != 4 || !(voice->flags & 4))
				chan->enabled = 0;
			}

			if (voice->flags & 1)
				chan->period = (voice->period += voice->portaSpeed);

			if (sample->divider) {
				stream->position = voice->vibratoPos;
				value = stream->readByte();

				if (value == -124) {
					stream->position = voice->vibratoPtr;
					value = stream->readByte();
				}

				voice->vibratoPos = stream->position;
				value = (voice->period / sample->divider) * value;
				chan->period = voice->period + value;
			}
		}

		if (sample->hiPos) {
			value = 0;

			if (voice->flags & 2) {
				voice->synthPos--;

				if (voice->synthPos <= sample->loPos) {
				voice->flags &= -3;
				value = 60;
				}
			} else {
				voice->synthPos++;

				if (voice->synthPos > sample->hiPos) {
					voice->flags |= 2;
					value = 60;
				}
			}

			amiga->memory[int(sample->pointer + voice->synthPos)] = value;
		}

		voice = voice->next;
	}
}

//override
void RHPlayer_initialize(struct RHPlayer* self) {
	int i = 0; 
	int j = 0; 
	struct RHSample *sample = 0;
	struct RHVoice *voice = &self->voices[3];
	
	super->initialize();

	song = songs[playSong];
	complete = 15;

	for (i = 0; i < samples->length; ++i) {
		sample = samples[i];

		if (sample->wave) {
			for (j = 0; j < sample->length; ++j)
				amiga->memory[int(sample->pointer + j)] = sample->wave[j];
		}
	}

	while (voice) {
		voice->initialize();
		voice->channel = amiga->channels[voice->index];

		voice->trackPtr = song->tracks[voice->index];
		voice->trackPos = 4;

		stream->position = voice->trackPtr;
		voice->patternPos = stream->readUnsignedInt();

		voice = voice->next;
	}
}

//override
void RHPlayer_loader(struct RHPlayer* self, struct ByteArray *stream) {
	int i = 0; 
	int j = 0; 
	int len = 0; 
	int pos = 0; 
	struct RHSample *sample = 0;
	int samplesData = 0; 
	int samplesHeaders = 0; 
	int samplesLen = 0; 
	struct RHSong *song = 0;
	int songsHeaders = 0; 
	int wavesHeaders = 0; 
	int wavesPointers = 0; 
	int value = 0;
	
	stream->position = 44;

	while (stream->position < 1024) {
		value = stream->readUnsignedShort();

		if (value == 0x7e10 || value == 0x7e20) {                               //moveq #16,d7 || moveq #32,d7
			value = stream->readUnsignedShort();

			if (value == 0x41fa) {                                                //lea $x,a0
				i = stream->position + stream->readUnsignedShort();
				value = stream->readUnsignedShort();

				if (value == 0xd1fc) {                                              //adda->l
					samplesData = i + stream->readUnsignedInt();
					amiga->loopLen = 64;
					stream->position += 2;
				} else {
					samplesData = i;
					amiga->loopLen = 512;
				}

				samplesHeaders = stream->position + stream->readUnsignedShort();
				value = stream->readUnsignedByte();
				if (value == 0x72) samplesLen = stream->readUnsignedByte();           //moveq #x,d1
			}
		} else if (value == 0x51c9) {                                           //dbf d1,x
			stream->position += 2;
			value = stream->readUnsignedShort();

			if (value == 0x45fa) {                                                //lea $x,a2
				wavesPointers = stream->position + stream->readUnsignedShort();
				stream->position += 2;

				while (1) {
					value = stream->readUnsignedShort();

					if (value == 0x4bfa) {                                            //lea $x,a5
						wavesHeaders = stream->position + stream->readUnsignedShort();
						break;
					}
				}
			}
		} else if (value == 0xc0fc) {                                           //mulu->w #x,d0
			stream->position += 2;
			value = stream->readUnsignedShort();

			if (value == 0x41eb)                                                  //lea $x(a3),a0
				songsHeaders = stream->readUnsignedShort();
		} else if (value == 0x346d) {                                           //movea->w x(a5),a2
			stream->position += 2;
			value = stream->readUnsignedShort();

			if (value == 0x49fa)                                                  //lea $x,a4
			vibrato = stream->position + stream->readUnsignedShort();
		} else if (value == 0x4240) {                                           //clr->w d0
			value = stream->readUnsignedShort();

			if (value == 0x45fa) {                                                //lea $x,a2
				periods = stream->position + stream->readUnsignedShort();
				break;
			}
		}
	}

	if (!samplesHeaders || !samplesData || !samplesLen || !songsHeaders) return;

	stream->position = samplesData;
	samples = new Vector.<RHSample>();
	samplesLen++;

	for (i = 0; i < samplesLen; ++i) {
		sample = new RHSample();
		sample->length   = stream->readUnsignedInt();
		sample->relative = 3579545 / stream->readUnsignedShort();
		sample->pointer  = amiga->store(stream, sample->length);
		samples[i] = sample;
	}

	stream->position = samplesHeaders;

	for (i = 0; i < samplesLen; ++i) {
		sample = samples[i];
		stream->position += 4;
		sample->loopPtr = stream->readInt();
		stream->position += 6;
		sample->volume = stream->readUnsignedShort();

		if (wavesHeaders) {
		sample->divider = stream->readUnsignedShort();
		sample->vibrato = stream->readUnsignedShort();
		sample->hiPos   = stream->readUnsignedShort();
		sample->loPos   = stream->readUnsignedShort();
		stream->position += 8;
		}
	}

	if (wavesHeaders) {
		stream->position = wavesHeaders;
		i = (wavesHeaders - samplesHeaders) >> 5;
		len = i + 3;
		variant = 1;

		if (i >= samplesLen) {
			for (j = samplesLen; j < i; ++j)
				samples[j] = new RHSample();
		}

		for (; i < len; ++i) {
			sample = new RHSample();
			stream->position += 4;
			sample->loopPtr   = stream->readInt();
			sample->length    = stream->readUnsignedShort();
			sample->relative  = stream->readUnsignedShort();

			stream->position += 2;
			sample->volume  = stream->readUnsignedShort();
			sample->divider = stream->readUnsignedShort();
			sample->vibrato = stream->readUnsignedShort();
			sample->hiPos   = stream->readUnsignedShort();
			sample->loPos   = stream->readUnsignedShort();

			pos = stream->position;
			stream->position = wavesPointers;
			stream->position = stream->readInt();

			sample->pointer = amiga->memory->length;
			amiga->memory->length += sample->length;
			sample->wave = new Vector.<int>(sample->length, true);

			for (j = 0; j < sample->length; ++j)
			sample->wave[j] = stream->readByte();

			samples[i] = sample;
			wavesPointers += 4;
			stream->position = pos;
		}
	}

	samples->fixed = true;

	stream->position = songsHeaders;
	songs = new Vector.<RHSong>();
	value = 65536;

	while (1) {
		song = new RHSong();
		stream->position++;
		song->tracks = new Vector.<int>(4, true);
		song->speed = stream->readUnsignedByte();

		for (i = 0; i < 4; ++i) {
			j = stream->readUnsignedInt();
			if (j < value) value = j;
			song->tracks[i] = j;
		}

		songs->push(song);
		if ((value - stream->position) < 18) break;
	}

	songs->fixed = true;
	lastSong = songs->length - 1;

	stream->length = samplesData;
	stream->position = 0x160;

	while (stream->position < 0x200) {
		value = stream->readUnsignedShort();

		if (value == 0xb03c) {                                                  //cmp->b #x,d0
			value = stream->readUnsignedShort();

			if (value == 0x0085) {                                                //-123
				variant = 2;
			} else if (value == 0x0086) {                                         //-122
				variant = 4;
			} else if (value == 0x0087) {                                         //-121
				variant = 3;
			}
		}
	}

	self->stream = stream;
	version = 1;
}