#include <Furrovine++/Graphics/Window.h>
#include <Furrovine++/WindowDriver.h>
#include <Furrovine++/Audio/AudioDevice.h>
#include <Furrovine++/Stopwatch.h>

int main ( ) {
	using namespace Furrovine;
	using namespace Furrovine::Graphics;
	using namespace Furrovine::Audio;
	
	uint32 samplerate = 44100;
	uint32 bitspersample = 16;
	uint32 channelcount = 2;
	AudioDevice audiodevice( bitspersample, channelcount, samplerate );
	
	std::vector<uint16> bufferdata( samplerate * 2 );
	AudioBuffer buffer( reinterpret_cast<byte*>( bufferdata.data() ), size_as<byte, uint16>( samplerate ), 0, samplerate );
	
	Stopwatch stopwatch;
	stopwatch.Start( );
	while ( true ) {
		double seconds = stopwatch.ElapsedSeconds( );

		if ( seconds < 1 )
			continue;
		audiodevice.SubmitBuffer( buffer );
		stopwatch.Restart( );
	}

}