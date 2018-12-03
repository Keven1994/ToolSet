#pragma once

namespace kevDev {
	namespace details {
		template<typename T>
		struct checkPointerPointer {
			static inline constexpr bool value = false;
		};

		template<typename T>
		struct checkPointerPointer<T**> {
			static inline constexpr bool value = true;
		};

		template<typename T, typename... Ts>
		constexpr bool contains()
		{
			return std::disjunction<std::is_same<T, Ts>...>::value;
		}
	}

	namespace vector_settings {
		namespace maxCount {
			struct _8BIT {};
			struct _16BIT {};
			struct _32BIT {};
			struct _64BIT {};
		}
		struct useInitialized {};
		//consider to use this only when you dont know your bounds @ compile time and you are safe to dont get out of bounds
		struct noSubscriptCheck {};

		template<bool isArray>
		struct deepDelete {
			static inline constexpr bool value = false;
		};

		template<>
		struct deepDelete<true> {
			static inline constexpr bool value = true;
		};

		struct optimized {};
	}


	template<typename... Args>
	struct Vector_Setting {
		using size_c = typename std::conditional<details::contains<vector_settings::maxCount::_8BIT, Args...>(), uint8_t,
			typename std::conditional<details::contains<vector_settings::maxCount::_16BIT, Args...>(), uint16_t,
			typename std::conditional<details::contains<vector_settings::maxCount::_32BIT, Args...>(), uint32_t, uint64_t>::type>::type>::type;
		static inline constexpr bool useInitialized = details::contains<vector_settings::useInitialized, Args...>();
		static inline constexpr bool deepDelete = details::contains<vector_settings::deepDelete<true>, Args...>() ||
			details::contains<vector_settings::deepDelete<false>, Args...>();
		static inline constexpr bool isArray = details::contains<vector_settings::deepDelete<true>, Args...>();
		static inline constexpr bool optimized = details::contains<vector_settings::optimized, Args...>();
		static inline constexpr bool noSubscriptCheck = details::contains<vector_settings::noSubscriptCheck, Args...>();
	};

	template<typename T, typename setting,
		typename setting::size_c(*resizeFactorFunction)(typename setting::size_c size) noexcept = vector_algorithms::defaultResAcq,
		typename T*(*searchAlgorithm)(const T& elem, T* data, typename setting::size_c size) noexcept = vector_algorithms::defaultSearchAlgo,
		void(*sortAlgorithm)(T data[], typename setting::size_c) noexcept = vector_algorithms::MergeSort>
		struct Vector_Algorithms {
		static inline constexpr auto ResFctPtr = resizeFactorFunction;
		static inline constexpr auto searchAlgo = searchAlgorithm;
		static inline constexpr auto sortAlgo = sortAlgorithm;
	};
}