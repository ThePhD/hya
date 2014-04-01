#include "RealtimeAudioDevice.h"
#include <Furrovine++/Stopwatch.h>
#include <Furrovine++/pass.h>
#include <Furrovine++/make_buffer_view.h>
#include "normalization.h"
#include <iostream>

struct DSPDelete {
	Furrovine::byte* data;

	DSPDelete( Furrovine::byte* data ) : data( data ) {

	}

	void operator () ( ) {
		delete [] data;
	}
};

template <typename T>
void ProcessAs( std::vector<std::unique_ptr<DSP>>& dsps, Furrovine::buffer_view<T> data ) {
	for ( std::size_t i = 0; i < data.size( ); ++i ) {
		T& sample = data[ i ];
		real adjusted = normalize<real>( sample );
		for ( std::size_t d = 0; d < dsps.size( ); ++d ) {
			DSP& dsp = *dsps[ d ];
			adjusted = dsp( adjusted );
		}
		sample = normalize<T>( adjusted );
	}
}

void RealtimeAudioDevice::SubmitBuffer( const Furrovine::Audio::AudioBuffer& buffer ) {
	using namespace Furrovine;
	using namespace Furrovine::Audio;

	Furrovine::Stopwatch stopwatch;
	double dspprocess, submitprocess, memcpyprocess, newprocess;
	const Furrovine::Audio::PCMAudioDescription& pcmaudio = audiodevice->PCMDescription( );
	const std::size_t maxsubmitsize = static_cast<std::size_t>( timechunk.count( ) * pcmaudio.SampleRate * pcmaudio.ChannelCount );
	const std::size_t maxsubmitbytesize = maxsubmitsize * pcmaudio.BytesPerSample();
	const Furrovine::uint32 buffersamplelength = buffer.playsamplelength == 0 ? pcmaudio.SampleSize( buffer.data.size() ) : buffer.playsamplelength;
	stopwatch.Start( );
	std::unique_ptr<Furrovine::byte []> data( new Furrovine::byte[ buffer.data.size( ) ] );
	newprocess = stopwatch.ElapsedMilliseconds( );
	stopwatch.Restart( );
	std::memcpy( data.get( ), buffer.data.data( ), buffer.data.size( ) );
	memcpyprocess = stopwatch.ElapsedMilliseconds( );
	Furrovine::buffer_view<Furrovine::byte> databuffer( data.get( ), buffer.data.size( ) );

	std::cout << "======================\n";
	std::cout << "new byte[ " << buffer.data.size( ) << " ] - " << newprocess << "ms\n";
	std::cout << "std::memcpy data - " << memcpyprocess << "ms\n";
	std::cout << "------------\n";
	for ( std::size_t sample = 0; sample < buffersamplelength; sample += maxsubmitsize ) {
		bool last = sample + maxsubmitsize > buffersamplelength;
		std::size_t databufferstartsample = sample;
		std::size_t databuffersamples = last ? buffersamplelength - sample : maxsubmitsize;
		Furrovine::Audio::AudioBuffer audiobuffer( databuffer, databufferstartsample, databuffersamples );
		stopwatch.Restart( );
		Process( audiobuffer );
		dspprocess = stopwatch.ElapsedMilliseconds( );
		stopwatch.Restart( );
		
		if ( last ) {
			audiobuffer.flags = AudioBufferFlags::StreamEnd;
			audiodevice->SubmitBuffer( audiobuffer, Furrovine::Audio::AudioBufferCallbacks( DSPDelete( data.release( ) ) ) );
		}
		else
			audiodevice->SubmitBuffer( audiobuffer );
		submitprocess = stopwatch.ElapsedMilliseconds( );
		
		std::cout << "sound buffer - " << pcmaudio.SampleDuration( databuffersamples ).Milliseconds() << "ms" << "\n";
		std::cout << "buffer dsp process - " << dspprocess << "ms" << "\n";
		std::cout << "submit buffer - " << submitprocess << "ms" << "\n";
	}
	data.release( );
}

const Furrovine::Audio::PCMAudioDescription& RealtimeAudioDevice::PCMDescription( ) const {
	return audiodevice->PCMDescription( );
}

RealtimeAudioDevice::RealtimeAudioDevice( Furrovine::Audio::AudioDevice& device, milliseconds timelimit ) : audiodevice( std::addressof( device ) ), timechunk( timelimit ) {
	seconds heuristic( 60 );
	auto reservecount = static_cast<std::size_t>( heuristic.count( ) / timechunk.count( ) );
	chunks.reserve( reservecount );
}

void RealtimeAudioDevice::Process( const Furrovine::Audio::AudioBuffer& buffer ) {
	const Furrovine::Audio::PCMAudioDescription& pcmaudio = audiodevice->PCMDescription( );
	Furrovine::buffer_view<Furrovine::byte> rawdata( const_cast<Furrovine::byte*>( buffer.data.data() ) + pcmaudio.BytesPerSample() * buffer.playsamplestart, buffer.playsamplelength * pcmaudio.BytesPerSample() );
	Process( rawdata );
}

void RealtimeAudioDevice::Process( Furrovine::buffer_view<Furrovine::byte> rawdata ) {
	switch ( audiodevice->PCMDescription( ).PCMFormat( ) ) {
	case Furrovine::Audio::PCMAudioFormat::UInt8:
		ProcessAs( dsps, rawdata );
		break;
	case Furrovine::Audio::PCMAudioFormat::UInt16:
		ProcessAs( dsps, Furrovine::make_buffer_view_as<Furrovine::uint16>( rawdata ) );
		break;
	case Furrovine::Audio::PCMAudioFormat::UInt32:
		ProcessAs( dsps, Furrovine::make_buffer_view_as<Furrovine::uint32>( rawdata ) );
		break;
	case Furrovine::Audio::PCMAudioFormat::UInt64:
		ProcessAs( dsps, Furrovine::make_buffer_view_as<Furrovine::uint64>( rawdata ) );
		break;
	case Furrovine::Audio::PCMAudioFormat::Int8:
		ProcessAs( dsps, Furrovine::make_buffer_view_as<Furrovine::int8>( rawdata ) );
		break;
	case Furrovine::Audio::PCMAudioFormat::Int16:
		ProcessAs( dsps, Furrovine::make_buffer_view_as<Furrovine::int16>( rawdata ) );
		break;
	case Furrovine::Audio::PCMAudioFormat::Int32:
		ProcessAs( dsps, Furrovine::make_buffer_view_as<Furrovine::int32>( rawdata ) );
		break;
	case Furrovine::Audio::PCMAudioFormat::Int64:
		ProcessAs( dsps, Furrovine::make_buffer_view_as<Furrovine::int64>( rawdata ) );
		break;
	case Furrovine::Audio::PCMAudioFormat::SinglePrecision:
		ProcessAs( dsps, Furrovine::make_buffer_view_as<float>( rawdata ) );
		break;
	case Furrovine::Audio::PCMAudioFormat::DoublePrecision:
		ProcessAs( dsps, Furrovine::make_buffer_view_as<double>( rawdata ) );
		break;
	}
}

void RealtimeAudioDevice::Add( std::unique_ptr<DSP> dsp ) {
	dsps.push_back( std::move( dsp ) );
}
