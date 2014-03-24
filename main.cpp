#include <Furrovine++/Graphics/Window.h>
#include <Furrovine++/WindowDriver.h>
#include <Furrovine++/Audio/AudioDevice.h>
#include <Furrovine++/Pipeline/FlacAudioLoader.h>
#include <Furrovine++/Pipeline/WavAudioLoader.h>
#include <Furrovine++/Stopwatch.h>
#include <Furrovine++/pass.h>
#include <chrono>
#include <queue>

using namespace Furrovine;
using namespace Furrovine::Graphics;
using namespace Furrovine::Audio;
using namespace Furrovine::Pipeline;

struct DSP {
	virtual void operator ()( fixed_vector<float, 16>& sample ) = 0;
	virtual ~DSP( ) {}
};

class Delay : public DSP {
private:
	std::queue<float> samplebacklog;
	std::size_t samplebacklogcount;
	float amplitude;
	std::chrono::seconds delay;

public:

	Delay( PCMAudio pcmaudio, std::chrono::seconds timedelay, float amp = 0.5f ) : amplitude( amp ), delay( timedelay ) {
		samplebacklogcount = static_cast<std::size_t>( pcmaudio.SampleRate * delay.count( ) );
	}

	virtual void operator() ( fixed_vector<float, 16>& sample ) override {
		if ( samplebacklog.size( ) < samplebacklogcount ) {
			for ( std::size_t i = 0; i < sample.size( ); ++i ) {
				float& channelsample = sample[ i ];
				samplebacklog.push( channelsample );
			}
			return;
		}
		for ( std::size_t i = 0; i < sample.size( ); ++i ) {
			sample[ i ] += samplebacklog.front( ) * amplitude;
			samplebacklog.pop( );
		}
	}
};

class AudioDSPDevice {
private:
	AudioDevice* audiodevice;

	struct DSPDelete {
		byte* data;

		DSPDelete( byte* data ) : data( data ) {

		}

		void operator () ( ) {
			delete [] data;
		}

	};

public:

	AudioDSPDevice( AudioDevice& device ) : audiodevice( std::addressof( device ) ) {
	
	}

	void SubmitBuffer( const AudioBuffer& buffer ) {
		const PCMAudio& pcmaudio = audiodevice->PCMConfig( );
		std::unique_ptr<byte[]> data( new byte[ buffer.data.size( ) ] );
		std::memcpy( data.get( ), buffer.data.data( ), buffer.data.size( ) );

		AudioBuffer audiobuffer( buffer );
		audiobuffer.data = buffer_view<byte>( data.get( ), buffer.data.size( ) );
		audiodevice->SubmitBuffer( audiobuffer, AudioBufferCallbacks( DSPDelete( data.release() ) ) );
	}

};

int main ( ) {
	
	uint32 samplerate = 48000;
	uint32 bitspersample = 16;
	uint32 channelcount = 2;
	AudioDevice baseaudiodevice( bitspersample, channelcount, samplerate );
	AudioDSPDevice audiodevice( baseaudiodevice );
	
	WavAudioData data = WavAudioLoader( )( "synth.wav" );
	std::vector<uint16> bufferdata( samplerate * 2 );
	AudioBuffer buffer( data.Buffer(), 0, 0, 0, 0, 1 );
	audiodevice.SubmitBuffer( buffer );

	Stopwatch stopwatch;
	stopwatch.Start( );
	while ( true ) {
		float seconds = stopwatch.ElapsedSeconds( );

		if ( seconds < 1 )
			continue;

		//audiodevice.SubmitBuffer( buffer );
		stopwatch.Restart( );
	}

}