#pragma once
#include <vector>
#include <functional>
#include <type_traits>
#include <utility>

namespace BitPounce
{
	template<typename>
	class FunctionArray;

	template<typename Ret, typename... Args>
	class FunctionArray<Ret(Args...)>
	{
	public:
		using result_type = Ret;
		using function_type = std::function<Ret(Args...)>;

		FunctionArray() noexcept = default;
		FunctionArray(std::nullptr_t) noexcept {}
		FunctionArray(const FunctionArray&) = default;
		FunctionArray(FunctionArray&&) = default;
		FunctionArray& operator=(const FunctionArray&) = default;
		FunctionArray& operator=(FunctionArray&&) = default;
		FunctionArray& operator=(std::nullptr_t) noexcept { clear(); return *this; }

		// Construct from any callable
		template<typename Callable, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Callable>, FunctionArray> && std::is_invocable_r_v<Ret, Callable, Args...>>>
		FunctionArray(Callable&& callable)
		{
			add(std::forward<Callable>(callable));
		}

		template<typename Callable, typename = std::enable_if_t<std::is_invocable_r_v<Ret, Callable, Args...>>>
		FunctionArray& add(Callable&& callable)
		{
			functions.emplace_back(std::forward<Callable>(callable));
			return *this;
		}

		template<typename Callable, typename = std::enable_if_t<std::is_invocable_r_v<Ret, Callable, Args...>>>
		FunctionArray& operator+=(Callable&& callable)
		{
			return add(std::forward<Callable>(callable));
		}

		Ret operator()(Args... args) const
		{
			if constexpr (std::is_same_v<Ret, void>)
			{
				for (const auto& f : functions)
					if (f) f(args...);
			}
			else
			{
				Ret last{};
				for (const auto& f : functions)
					if (f) last = f(args...);
				return last;
			}
		}

		explicit operator bool() const noexcept { return !functions.empty(); }
		bool empty() const noexcept { return functions.empty(); }
		size_t size() const noexcept { return functions.size(); }

		const std::type_info& target_type() const noexcept { return typeid(void); }
		template<typename T> T* target() noexcept { return nullptr; }
		template<typename T> const T* target() const noexcept { return nullptr; }

		void swap(FunctionArray& other) noexcept { functions.swap(other.functions); }
		void clear() noexcept { functions.clear(); }

		const std::vector<function_type>& get_functions() const { return functions; }

	private:
		std::vector<function_type> functions;
	};

	template<typename Ret, typename... Args>
	void swap(FunctionArray<Ret(Args...)>& a, FunctionArray<Ret(Args...)>& b) noexcept
	{
		a.swap(b);
	}
}