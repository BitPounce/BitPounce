#pragma once
#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>
#include <BitPounce/Core/FileSystem.h>

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

		VersionInfo() = default;

		VersionInfo(Version version, int build, std::string commit, std::string date)
		{
			this->version = version;
			this->build = build;
			this->commit = commit;
			this->date = date;
		}

		VersionInfo(const std::filesystem::path& filepath)
		{
			BufferBase buffer = FileSystem::LoadFile(filepath);
			std::string jsonStr((char*)buffer.Data, buffer.Size);
			nlohmann::json json = nlohmann::json::parse(jsonStr);

			if (json.contains("version"))
			{
				std::string v = json["version"];
				int maj = 0, min = 0, pat = 0;
				std::sscanf(v.c_str(), "%d.%d.%d", &maj, &min, &pat);
				version = Version(maj, min, pat);
			}
		
			if (json.contains("build"))
				build = json["build"].get<int>();
			else
				build = 0;
		
			if (json.contains("commit"))
				commit = json["commit"].get<std::string>();
			else
				commit = "";
		
			if (json.contains("date"))
				date = json["date"].get<std::string>();
			else
				date = "";
		}
	};
}