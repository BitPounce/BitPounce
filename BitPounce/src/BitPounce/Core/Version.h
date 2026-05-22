#pragma once
#include <cstdint>
#include <string>

namespace BitPounce
{
	struct Version
	{
		uint16_t major = 0;
		uint16_t minor = 0;
		uint16_t patch = 0;

		Version() = default;

		Version(uint16_t maj, uint16_t min, uint16_t pat)
			: major(maj), minor(min), patch(pat) {}

		std::string toString() const
		{
			return std::to_string(major) + "." +
				   std::to_string(minor) + "." +
				   std::to_string(patch);
		}

		bool operator==(const Version& other) const
		{
			return major == other.major &&
				   minor == other.minor &&
				   patch == other.patch;
		}

		bool operator<(const Version& other) const
		{
			if (major != other.major) return major < other.major;
			if (minor != other.minor) return minor < other.minor;
			return patch < other.patch;
		}

		bool operator!=(const Version& other) const { return !(*this == other); }
		bool operator>(const Version& other) const { return other < *this; }
		bool operator<=(const Version& other) const { return !(other < *this); }
		bool operator>=(const Version& other) const { return !(*this < other); }
	};

	struct VersionInfo
	{
		Version version;
		int build;
		std::string commit;
		std::string date;
	};
}