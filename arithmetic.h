#pragma once

#include <functional>

typedef std::plus<> add;
typedef std::minus<> subtract;
typedef std::multiplies<> multiply;
typedef std::divides<> divide;

/*struct adder {
	template <typename Tl, typename Tr>
	auto operator()( Tl&& left, Tr&& right )
		-> decltype( left + right ) {
		return left + right;
	}
};

struct subtracter {
	template <typename Tl, typename Tr>
	auto operator()( Tl&& left, Tr&& right )
		-> decltype( left - right ) {
		return left - right;
	}
};

struct multiplier {
	template <typename Tl, typename Tr>
	auto operator()( Tl&& left, Tr&& right )
		-> decltype( left * right ) {
		return left * right;
	}
};

struct divider {
	template <typename Tl, typename Tr>
	auto operator()( Tl&& left, Tr&& right )
		-> decltype( left / right ) {
		return left / right;
	}
};*/

struct modulus {
	template <typename Tl, typename Tr>
	auto operator()( Tl&& left, Tr&& right )
		-> decltype( left % right ) {
		return left % right;
	}
};
