#pragma once

#include "real.h"
#include "chrono.h"
#include "DSP.h"
#include <Furrovine++/Audio/AudioDevice.h>
#include <vector>

class RealtimeAudioDevice {
private:
	Furrovine::Audio::AudioDevice* audiodevice;
	std::vector<std::unique_ptr<DSP>> dsps;
	std::vector<Furrovine::Audio::AudioBuffer> chunks;
	seconds timechunk;

	void Process( const Furrovine::Audio::AudioBuffer& data );
	void Process( Furrovine::buffer_view<Furrovine::byte> rawdata );

public:

	RealtimeAudioDevice( Furrovine::Audio::AudioDevice& device, milliseconds timelimit = milliseconds( 25 ) );

	const Furrovine::Audio::PCMAudioDescription& PCMDescription( ) const;

	void SubmitBuffer( const Furrovine::Audio::AudioBuffer& buffer );

	void Update( );

	void Add( std::unique_ptr<DSP> dsp );

	template <typename T>
	void AddDSP( T&& dsp ) {
		typedef typename std::decay<T>::type TDecay;
		Add( std::make_unique<TDecay>( std::forward<T>( dsp ) ) );
	}

};
