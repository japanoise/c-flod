/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 3.0 - 2012/02/08

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/

#include "SBSample.h"
#include "../flod_internal.h"

void SBSample_defaults(struct SBSample* self) {
	CLASS_DEF_INIT();
	// static initializers go here
	self->name = "";
	self->bits = 8;
}

void SBSample_ctor(struct SBSample* self) {
	CLASS_CTOR_DEF(SBSample);
	// original constructor code goes here
}

struct SBSample* SBSample_new(void) {
	CLASS_NEW_BODY(SBSample);
}

void SBSample_store(struct SBSample* self, struct ByteArray* stream) {
	int delta = 0;
	int i = 0;
	int len = self->length;
	int pos = 0;
	Number sample = NAN;
	int total = 0;
	int value = 0;
	if (!self->loopLen) self->loopMode = 0;
	pos = stream->position;

	if (self->loopMode) {
		len = self->loopStart + self->loopLen;
		data = new Vector.<Number>(len + 1, true);
	} else {
		data = new Vector.<Number>(self->length + 1, true);
	}

	if (self->bits == 8) {
		total = pos + len;

		if (total > stream->length)
		len = stream->length - pos;

		for (i = 0; i < len; ++i) {
			value = stream->readByte() + delta;
	
			if (value < -128) value += 256;
			else if (value > 127) value -= 256;

			self->data[i] = value * 0.0078125;
			delta = value;
		}
	} else {
		total = pos + (len << 1);

		if (total > stream->length)
		len = (stream->length - pos) >> 1;

		for (i = 0; i < len; ++i) {
			value = stream->readShort() + delta;

			if (value < -32768) value += 65536;
			else if (value > 32767) value -= 65536;

			self->data[i] = value * 0.00003051758;
			delta = value;
		}
	}

	total = pos + self->length;

	if (!self->loopMode) {
		self->data[self->length] = 0.0;
	} else {
		self->length = self->loopStart + self->loopLen;

		if (self->loopMode == 1) {
			self->data[len] = self->data[self->loopStart];
		} else {
			self->data[len] = self->data[int(len - 1)];
		}
	}

	if (len != self->length) {
		sample = self->data[len - 1];
		for (i = len; i < self->length; ++i) self->data[i] = sample;
	}

	if (total < stream->length) stream->position = total;
	else stream->position = stream->length - 1;
}
