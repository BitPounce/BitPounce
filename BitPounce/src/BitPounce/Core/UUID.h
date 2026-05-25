#pragma once

#include <functional>

namespace BitPounce {

	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }

		constexpr bool operator==(const UUID& other) const {
			return m_UUID == other.m_UUID;
		}

		constexpr bool operator==(const uint64_t& other) const {
			return m_UUID == other;
		}

		// HACK: too remove some warning
		constexpr bool operator==(const int& other) const {
			return m_UUID == other;
		}
	private:
		uint64_t m_UUID;
	};

}

namespace std {

	template<>
	struct hash<BitPounce::UUID>
	{
		std::size_t operator()(const BitPounce::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};

}