#pragma once
#include "BitPounce/Core/Base.h"
#include <mutex>

namespace BitPounce
{
	class RefObj
	{
	public:
		RefObj();
		RefObj(void* ptr);

		RefObj(const RefObj& other);
		RefObj& operator=(const RefObj& other);
		RefObj(RefObj&& other) noexcept;
		RefObj& operator=(RefObj&& other) noexcept;

		virtual ~RefObj();

		void* Get();
		void Set(void* ptr);

	protected:
		virtual void Free() = 0;

	private:
		void* m_ptr = nullptr;
	};
}
