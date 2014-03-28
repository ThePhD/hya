#pragma once

#include "DSP.h"
#include "chrono.h"
#include "arithmetic.h"
#include <Furrovine++/Audio/PCMAudioDescription.h>
#include <queue>

template <typename TAction = std::plus<void>>
class TPerChannelDelay : public DSP {
private:
	struct DelayChannel {
		seconds delay;
		std::size_t samplecount;
		std::queue<real> samplebacklog;
	};
	
	Furrovine::Audio::PCMAudioDescription pcmdesc;
	real amplitude;
	TAction action;
	std::size_t currentchannel;
	std::vector<DelayChannel> delays;

public:

	TPerChannelDelay( const Furrovine::Audio::PCMAudioDescription& pcmaudio, std::vector<seconds> timedelay, real ampl = 0.5f, TAction act = TAction( ) )
	: pcmdesc( pcmaudio ), amplitude( ampl ), action( std::move( act ) ), currentchannel( 0 ){
		seconds basevalue = timedelay.size( ) > 0 ? timedelay[ 0 ] : seconds( 0.5 );
		timedelay.resize( pcmdesc.ChannelCount, basevalue );

		for ( std::size_t t = 0; t < pcmdesc.ChannelCount; ++t ) {
			delays.emplace_back( );
			delays[ t ].delay = timedelay[ t ];
			delays[ t ].samplecount = static_cast<std::size_t>( pcmaudio.SampleRate * delays[ t ].delay.count( ) );
		}
	}

	virtual real operator() ( real sample ) override {
		DelayChannel& channel = delays[ currentchannel ];
		auto& delaysamplebacklog = channel.samplebacklog;
		auto& delaysamplecount = channel.samplecount;
		currentchannel = ( currentchannel + 1 ) % pcmdesc.ChannelCount;
		
		channel.samplebacklog.push( sample );
		if ( delaysamplebacklog.size( ) < delaysamplecount )
			return sample;
		real oldsample = delaysamplebacklog.front( );
		delaysamplebacklog.pop( );

		oldsample *= amplitude;
		return action( sample, oldsample );
	}
};

const seconds Channel0Delay = seconds( 0.513 );
const seconds Channel1Delay = seconds( 0.490 );

typedef TPerChannelDelay<> PerChannelDelay;
