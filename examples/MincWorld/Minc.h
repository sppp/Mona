/*
    Copyright (c) 2012 Tom Portegys (portegys@gmail.com). All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are
    permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY TOM PORTEGYS "AS IS" AND ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
    FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// A minc is a creature with a hybrid brain: a LENS neural network governing instinctive
// behavior augmented by a mona neural network that learns from experience.

#ifndef MINC_HPP
#define MINC_HPP

#ifdef _WIN32
	#ifndef WIN32
		#define WIN32
	#endif
#endif

#include "Tmaze.p"
#ifdef __cplusplus
extern "C" {
//#include "../../lens/Src/lens.h"
}
#endif

#include <Mona/Mona.h>

class Minc {
public:

	// Identifier.
	int id;

	// LENS neural network.
	static int  HIDDEN_UNITS;
	static real LEARNING_RATE;
	int         numMarks;
	int         intervals;
	char*        lensNet;

	// Mona neural network.
	static RESPONSE_POTENTIAL MIN_RESPONSE_POTENTIAL;
	Mona* mona;

	// Constructors.
	Minc(int numMarks, int intervals, int random_seed);
	Minc();

	// Destructor.
	~Minc();

	// Generalization learning.
	void generalize(char* example, int numUpdates);

	// Discrimination learning.
	// Return true for success.
	bool discriminate(char* example, Tmaze* maze);

	// Result testing.
	// Return true for success.
	bool test(char* example, Tmaze* maze, bool verbose = true);

	// Lens output callback.
	static void lensCallback();

	static int  startTick;
	static int  tickCount;
	static Minc* activeMinc;
	static bool result;
	static bool printCallback;
	Tmaze*       maze;

	// Load minc.
	void Load(String filename);
	void Load(FILE*);

	// Save minc.
	void Store(String filename);
	void Store(FILE*);

	// Random numbers.
	int random_seed;
	Random* randomizer;

	// Identifier dispenser.
	static int id_dispenser;
};
#endif
