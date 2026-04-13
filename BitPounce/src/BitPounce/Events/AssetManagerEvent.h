#pragma once

#include "Event.h"
#include "BitPounce/Scene/Scene.h"
#include "BitPounce/Asset/AssetManager.h"
#include <sstream>

namespace BitPounce
{
    #define ALL_EVENT_CLASSS_USING_AssetMetad_AND_AssetHandle(type, category) \
        class  type##Event : public Event \
        {\
        public:\
        type##Event(AssetMetadata& metadata, const AssetHandle& handle): m_Metadata(metadata), m_Handle(handle) {}\
        EVENT_GETER(AssetMetadata, m_Metadata, GetMetadata);\
        EVENT_GETER(AssetHandle, m_Handle, GetHandle);\
\
		std::string ToString() const override\
		{\
			std::stringstream ss;\
			ss << #type << ": " << m_Handle;\
			return ss.str();\
		} \
\
		EVENT_CLASS_TYPE(type) \
		EVENT_CLASS_CATEGORY(category) \
	private: \
		AssetMetadata& m_Metadata; \
        AssetHandle m_Handle; \
        };    \
        \

    ALL_EVENT_CLASSS_USING_AssetMetad_AND_AssetHandle(AssetPreLoaded, EventCategoryAssetManager)
    ALL_EVENT_CLASSS_USING_AssetMetad_AND_AssetHandle(AssetLoaded, EventCategoryAssetManager)
}