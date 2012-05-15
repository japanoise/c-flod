/*
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

#include "CorePlayer.h"
#include "../flod_internal.h"
#include "Amiga.h"
#include "Soundblaster.h"

void CorePlayer_defaults(struct CorePlayer* self) {
	CLASS_DEF_INIT();
	// static initializers go here
	self->encoding = ENCODING_US_ASCII;
	self->title = "";
	self->soundPos = 0.0;
}

void CorePlayer_ctor(struct CorePlayer* self, struct CoreMixer *hardware) {
	CLASS_CTOR_DEF(CorePlayer);
	// original constructor code goes here
	hardware->player = self;
	self->hardware = hardware;
	
	//add vtable
	self->process = CorePlayer_process;
	self->fast = CorePlayer_fast;
	self->accurate = CorePlayer_accurate;
	self->setup = CorePlayer_setup;
	self->set_ntsc = CorePlayer_set_ntsc;
	self->set_stereo = CorePlayer_set_stereo;
	self->set_volume = CorePlayer_set_volume;
	self->toggle = CorePlayer_toggle;
	self->reset = CorePlayer_reset;
	self->loader = CorePlayer_loader;
}

struct CorePlayer* CorePlayer_new(struct CoreMixer *hardware) {
	CLASS_NEW_BODY(CorePlayer, hardware);
}

void CorePlayer_set_force(struct CorePlayer* self, int value) {
	self->version = 0;
}

/* stubs */
void CorePlayer_process(struct CorePlayer* self) {}

void CorePlayer_setup(struct CorePlayer* self) {}

void CorePlayer_set_ntsc(struct CorePlayer* self, int value) { }

void CorePlayer_set_stereo(struct CorePlayer* self, Number value) { }

void CorePlayer_set_volume(struct CorePlayer* self, Number value) { }

void CorePlayer_toggle(struct CorePlayer* self, int index) {}

void CorePlayer_reset(struct CorePlayer* self) { }

void CorePlayer_loader(struct CorePlayer* self, struct ByteArray *stream) { }

/* callback function for EVENT_SAMPLE_DATA */
void CorePlayer_fast(struct CorePlayer* self) {}
void CorePlayer_accurate(struct CorePlayer* self) {}


struct ByteArray *CorePlayer_get_waveform(struct CorePlayer* self) {
	return CoreMixer_waveform(self->hardware);
}

int CorePlayer_load(struct CorePlayer* self, struct ByteArray *stream) {
	CoreMixer_reset(self->hardware);
	ByteArray_set_position(stream, 0);

	self->version  = 0;
	self->playSong = 0;
	self->lastSong = 0;
#ifdef SUPPORT_COMPRESSION
	struct ZipFile* zip;	
	if (stream->readUnsignedInt(stream) == 67324752) {
		zip = ZipFile_new(stream);
		stream = zip->uncompress(zip->entries[0]);
	}
#endif

	if (stream) {
		stream->endian = self->endian;
		ByteArray_set_position(stream, 0);
		self->loader(self, stream);
		if (self->version) self->setup(self);
	}
	return self->version;
}

/* processor default : NULL */
int CorePlayer_play(struct CorePlayer* self, struct Sound *processor) {
	if (!self->version) return 0;
	if (self->soundPos == 0.0) {
		//self->initialize();
		CorePlayer_initialize(self);
	}
	self->sound = processor ? processor : Sound_new();

	//if (self->quality && (self->hardware->type == CM_SOUNDBLASTER)) {
	if (self->quality) {
		EventDispatcher_addEventListener((struct EventDispatcher*) self->sound, EVENT_SAMPLE_DATA, (EventHandlerFunc) self->hardware->accurate);
	} else {
		EventDispatcher_addEventListener((struct EventDispatcher*) self->sound, EVENT_SAMPLE_DATA, (EventHandlerFunc) self->hardware->fast);
	}

	self->soundChan = Sound_play(self->sound, self->soundPos);
	EventDispatcher_addEventListener((struct EventDispatcher*) self->soundChan, EVENT_SOUND_COMPLETE, (EventHandlerFunc) CorePlayer_completeHandler);
	self->soundPos = 0.0;
	return 1;
}

void CorePlayer_pause(struct CorePlayer* self) {
	if (!self->version || !self->soundChan) return;
	self->soundPos = SoundChannel_get_position(self->soundChan);
	EventDispatcher_removeEvents((struct EventDispatcher*) self);
}

void CorePlayer_stop(struct CorePlayer* self) {
	if (!self->version) return;
	if (self->soundChan) EventDispatcher_removeEvents((struct EventDispatcher*) self);
	self->soundPos = 0.0;
	self->reset(self);
}

    //js function reset
void CorePlayer_initialize(struct CorePlayer* self) {
	self->tick = 0;
	CoreMixer_initialize(self->hardware);
	if(self->hardware->type == CM_AMIGA)
		Amiga_initialize((struct Amiga*) self->hardware);
	else if(self->hardware->type == CM_SOUNDBLASTER)
		Soundblaster_initialize((struct Soundblaster*) self->hardware);
	else
		abort();
	//self->hardware->initialize();
	self->hardware->samplesTick = 110250 / self->tempo;
}

/* callback function for EVENT_SOUND_COMPLETE */
void CorePlayer_completeHandler(struct CorePlayer* self, struct Event *e) {
	CorePlayer_stop(self);
	EventDispatcher_dispatchEvent((struct EventDispatcher*) self, e);
}

void CorePlayer_removeEvents(struct CorePlayer* self) {
	SoundChannel_stop(self->soundChan);
	EventDispatcher_removeEventListener((struct EventDispatcher*) self->soundChan, EVENT_SOUND_COMPLETE, (EventHandlerFunc) CorePlayer_completeHandler);
	EventDispatcher_dispatchEvent((struct EventDispatcher*) self->soundChan, Event_new(EVENT_SOUND_COMPLETE));
	//self->soundChan->dispatchEvent(new Event(Event->SOUND_COMPLETE));

	if (self->quality) {
		EventDispatcher_removeEventListener((struct EventDispatcher*) self->sound, EVENT_SAMPLE_DATA, (EventHandlerFunc) self->hardware->accurate);
	} else {
		EventDispatcher_removeEventListener((struct EventDispatcher*) self->sound, EVENT_SAMPLE_DATA, (EventHandlerFunc) self->hardware->fast);
	}
}

