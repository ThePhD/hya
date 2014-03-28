#pragma once

#include "DSP.h"
#include "chrono.h"
#include "arithmetic.h"
#include <Furrovine++/Audio/PCMAudioDescription.h>
#include <queue>

template <typename TAction = std::plus<void>>
class TDelay : public DSP {
private:
	std::queue<real> samplebacklog;
	std::size_t delaysamplecount;
	real amplitude;
	seconds delay;
	TAction action;

public:

	TDelay( const Furrovine::Audio::PCMAudioDescription& pcmaudio, seconds timedelay = 1, real amp = 0.5f, TAction act = TAction( ) ) 
	: amplitude( amp ), delay( timedelay ), action( std::move( act ) ) {
		delaysamplecount = static_cast<std::size_t>( pcmaudio.SampleRate * pcmaudio.ChannelCount * delay.count( ) );
	}

	virtual real operator() ( real sample ) override {
		samplebacklog.push( sample );
		if ( samplebacklog.size( ) < delaysamplecount )
			return sample;
		real oldsample = samplebacklog.front( );
		samplebacklog.pop( );

		oldsample *= amplitude;
		return action( sample, oldsample );
	}
};


typedef TDelay<> Delay;
