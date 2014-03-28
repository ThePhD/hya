#include <Furrovine++/Stopwatch.h>
#include <Furrovine++/pass.h>
#include <Furrovine++/make_buffer_view.h>
#include "normalization.h"
#include "AudioDSPDevice.h"
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

void AudioDSPDevice::SubmitBuffer( const Furrovine::Audio::AudioBuffer& buffer ) {
	Furrovine::Stopwatch stopwatch;
	const Furrovine::Audio::PCMAudioDescription& pcmaudio = audiodevice->PCMDescription( );
	const std::size_t maxsubmitsize = static_cast<std::size_t>( timechunk.count( ) * pcmaudio.SampleRate * pcmaudio.ChannelCount );
	const std::size_t maxsubmitbytesize = pcmaudio.BytesPerSample( ) * maxsubmitsize;

	stopwatch.Start( );
	std::unique_ptr<Furrovine::byte []> data( new Furrovine::byte[ buffer.data.size( ) ] );
	std::cout << "new byte[n] - " << stopwatch.ElapsedMilliseconds( ) << "ms | ( n = " << buffer.data.size() << " )\n";
	
	stopwatch.Restart( );
	std::memcpy( data.get( ), buffer.data.data( ), buffer.data.size( ) );
	std::cout << "std::memcpy - " << stopwatch.ElapsedMilliseconds( ) << "ms\n";
	
	/*for ( std::size_t submit = 0; submit < buffer.data.size( ); submit += maxsubmitbytesize ) {
	*/	//bool last = submit + maxsubmitbytesize > buffer.data.size( );
		//std::size_t databuffersize = last ?  
		Furrovine::buffer_view<Furrovine::byte> databuffer( data.get( ), buffer.data.size( ) );
		stopwatch.Restart( );
		Process( databuffer );
		std::cout << "process(  ) - " << stopwatch.ElapsedSeconds( ) << "s" << "\n";
		stopwatch.Restart( );
		Furrovine::Audio::AudioBuffer audiobuffer( buffer );
		audiobuffer.data = Furrovine::buffer_view<Furrovine::byte>( data.get( ), buffer.data.size( ) );
		
		//if ( last )
			audiodevice->SubmitBuffer( audiobuffer, Furrovine::Audio::AudioBufferCallbacks( DSPDelete( data.release( ) ) ) );
		//else
		//	audiodevice->SubmitBuffer( audiobuffer );
	//}
}

const Furrovine::Audio::PCMAudioDescription& AudioDSPDevice::PCMDescription( ) const {
	return audiodevice->PCMDescription( );
}

AudioDSPDevice::AudioDSPDevice( Furrovine::Audio::AudioDevice& device, std::chrono::duration<real, std::milli> timelimit ) : audiodevice( std::addressof( device ) ), timechunk( timelimit ) {
	milliseconds heuristic( 600000 );
	auto reservecount = static_cast<std::size_t>( heuristic.count( ) / timechunk.count( ) );
	chunks.reserve( reservecount );
}

void AudioDSPDevice::Process( Furrovine::buffer_view<Furrovine::byte> rawdata ) {
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

void AudioDSPDevice::Add( std::unique_ptr<DSP> dsp ) {
	dsps.push_back( std::move( dsp ) );
}
