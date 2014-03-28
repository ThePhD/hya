#pragma once

#include "real.h"

struct DSP {
	virtual real operator ()( real sample ) = 0;
	virtual ~DSP( );
};
