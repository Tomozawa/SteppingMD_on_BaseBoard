// For user
// memcpyできる8バイト以内の型を、
// DataField型に書き込んだり読み出したりするための関数がまとまっている。



#pragma once

#include <cstring>
#include <bit>
#include <type_traits>
#include <utility>

#include "utility.hpp"

namespace CRSLib::Can
{
	template<class T>
	concept BeAbleToPackC = sizeof(T) <= can_mtu && std::is_trivially_copyable_v<T>;

	// DataField型へ変換
	template<std::endian endian, BeAbleToPackC T>
	inline void pack(DataField& buffer, const T& value) noexcept
	{
		std::memcpy(buffer.data(), &value, sizeof(T));

		if constexpr(std::endian::native != endian)
		{
			for(unsigned int i = 0; i < sizeof(T) / 2; ++i)
			{
				std::swap(buffer[i], buffer[sizeof(T) - 1 - i]);
			}
		}
	}

	// DataField型から逆変換
	template<std::endian endian, BeAbleToPackC T>
	inline T unpack(const DataField& buffer) noexcept
	{
		if constexpr(std::endian::native != endian)
		{
			char tmp[can_mtu];
			for(unsigned int i = 0; i < sizeof(T); ++i)
			{
				tmp[i] = buffer[sizeof(T) - 1 - i];
			}

			// 確か大丈夫だったはず(char *からは任意の型にアクセスできるんじゃないっけ？)
			return *reinterpret_cast<const T *>(tmp);
		}
		else
		{
			return *reinterpret_cast<const T *>(buffer.data());
		}
	}


}
