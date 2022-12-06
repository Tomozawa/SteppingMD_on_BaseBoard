#pragma once

#include <cstddef>
#include <cstdint>

namespace CRSLib
{
	namespace IntegerTypes
	{
#define Stew_define_standard_integer_type(bit_size)\
		using i##bit_size = std::int##bit_size##_t;\
		using u##bit_size = std::uint##bit_size##_t

		Stew_define_standard_integer_type(8);
		Stew_define_standard_integer_type(16);
		Stew_define_standard_integer_type(32);
		Stew_define_standard_integer_type(64);

#undef Stew_define_standard_integer_type

		using i_ptr = std::intptr_t;
		using ptrdiff_t = std::ptrdiff_t;
		using size_t = std::size_t;
		using nullptr_t = std::nullptr_t;
		using max_align_t = std::max_align_t;
		using byte = std::byte;
	}

	using namespace IntegerTypes;

	namespace IntegerLiterals
	{
#define Stew_define_integer_literals(type)\
		inline constexpr type operator"" _##type(unsigned long long x) noexcept\
		{\
			return x;\
		}

		Stew_define_integer_literals(i8)
		Stew_define_integer_literals(u8)
		Stew_define_integer_literals(i16)
		Stew_define_integer_literals(u16)
		Stew_define_integer_literals(i32)
		Stew_define_integer_literals(u32)
		Stew_define_integer_literals(i64)
		Stew_define_integer_literals(u64)
		Stew_define_integer_literals(i_ptr)
		Stew_define_integer_literals(ptrdiff_t)
		Stew_define_integer_literals(size_t)

#undef Stew_define_integer_literals
	}
}
