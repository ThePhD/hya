#pragma once

#include "real.h"
#include <type_traits>

template <typename T, typename THint>
struct normalization_limits {
	const static T max( ) {
		return std::is_integral<T>::value ? std::numeric_limits<T>::max( ) : static_cast<T>( 1 );
	}

	const static T min( ) {
		return std::is_integral<T>::value ? std::numeric_limits<T>::min( ) : static_cast<T>( std::is_signed<THint>::value ? -1 : 0 );
	}
};

template <typename T, typename TValue, typename TIntermediate = double>
T normalize( TValue value, TValue valuemax, TValue valuemin, T max, T min ) {
	TIntermediate valuerange = static_cast<TIntermediate>( valuemax - valuemin );
	TIntermediate valuepercentage = static_cast<TIntermediate>( value - valuemin ) / static_cast<TIntermediate>( valuerange );
	TIntermediate range = static_cast<TIntermediate>( max - min );
	TIntermediate shiftedvalue = valuepercentage * range;
	return static_cast<T>( shiftedvalue + static_cast<TIntermediate>( min ) );
}

template <typename T, typename TValue, typename TIntermediate = double>
T normalize( T value, T valuemax, T valuemin, T max, T min ) {
	return value;
}

template <typename T, typename TValue, typename TIntermediate = double>
T normalize( TValue value ) {
	const static auto valuemax = normalization_limits<TValue, T>::max( );
	const static auto valuemin = normalization_limits<TValue, T>::min( );
	const static auto max = normalization_limits<T, TValue>::max( );
	const static auto min = normalization_limits<T, TValue>::min( );
	const static auto intdmin = static_cast<TIntermediate>( min );
	const static auto valuerange = static_cast<TIntermediate>( valuemax - valuemin );
	const static auto range = static_cast<TIntermediate>( max - min );

	TIntermediate valuepercentage = static_cast<TIntermediate>( value - valuemin ) / static_cast<TIntermediate>( valuerange );
	TIntermediate shiftedvalue = valuepercentage * range;
	return static_cast<T>( shiftedvalue + intdmin ); 
}
