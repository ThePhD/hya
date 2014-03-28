//#include <Furrovine++/Graphics/Window.h>
//#include <Furrovine++/WindowDriver.h>
#include <Furrovine++/Pipeline/WavAudioLoader.h>
#include <Furrovine++/Stopwatch.h>

#include "AudioDSPDevice.h"
#include "DSPs.h"

int main ( ) {
	using namespace Furrovine;
	using namespace Furrovine::Audio;
	using namespace Furrovine::Pipeline;
	//using namespace Furrovine::Graphics;

	const static uint32 samplerate = 48000;
	const static uint32 bitspersample = 16;
	const static uint32 channelcount = 2;

	WavAudioData data = WavAudioLoader( )( "synth.wav" );

	AudioDevice baseaudiodevice( bitspersample, channelcount, samplerate );
	AudioDSPDevice audiodevice( baseaudiodevice, milliseconds( 50 ) );
	audiodevice.AddDSP( PerChannelDelay( audiodevice.PCMDescription( ), { seconds( 0.513 ), seconds( 0.490 ) }, real( 0.25 ) ) );
	
	AudioBuffer buffer( data.Buffer(), 0, 0, 0, 0, 1 );
	audiodevice.SubmitBuffer( buffer );

	Stopwatch stopwatch;
	stopwatch.Start( );
	while ( true ) {
		double seconds = stopwatch.ElapsedSeconds( );

		if ( seconds < 1 )
			continue;

		//audiodevice.SubmitBuffer( buffer );
		stopwatch.Restart( );
	}

}