#pragma once
#include "C++Utilities/CppUtils.hpp"

template<class Type>
concept Integral = is_integral<Type>::value;

template<class Type>
concept NotIntegral = !is_integral<Type>::value;

template <class T>
concept Callable = requires(T a)
{
    { a() };
};

template <class T>
concept NotCallable = !Callable<T>;

template <class... Types>
struct has_callable;

template <class Type, class... Types>
struct has_callable<Type,Types...>{
	static constexpr bool value = Callable<Type> || has_callable<Types...>::value;
};

template <>
struct has_callable<>{
	static constexpr bool value = false;
};

template <class... Types>
concept CallablePack = has_callable<Types...>::value;

template <class... Types>
concept NotCallablePack = !CallablePack<Types...>;

template <class T>
concept Container = requires(T a, const T b)
{
    requires std::regular<T>;
    requires std::swappable<T>;
    requires std::same_as<typename T::reference, typename T::value_type &>;
    requires std::same_as<typename T::const_reference, const typename T::value_type &>;
    requires std::forward_iterator<typename T::iterator>;
    requires std::forward_iterator<typename T::const_iterator>;
    requires std::signed_integral<typename T::difference_type>;
    requires std::same_as<typename T::difference_type, typename std::iterator_traits<typename T::iterator>::difference_type>;
    requires std::same_as<typename T::difference_type, typename std::iterator_traits<typename T::const_iterator>::difference_type>;
    { a.begin() } -> std::same_as<typename T::iterator>;
    { a.end() } -> std::same_as<typename T::iterator>;
    { b.begin() } -> std::same_as<typename T::const_iterator>;
    { b.end() } -> std::same_as<typename T::const_iterator>;
    { a.cbegin() } -> std::same_as<typename T::const_iterator>;
    { a.cend() } -> std::same_as<typename T::const_iterator>;
    { a.size() } -> std::same_as<typename T::size_type>;
    { a.max_size() } -> std::same_as<typename T::size_type>;
    { a.empty() } -> std::convertible_to<bool>;
};

template <class T>
concept NotContainer = !Container<T>;

template <class T, class... U>
concept PackDerivesFrom = (std::derived_from<T, U> && ...);


template<class... Types> struct has_container;

template<class Type, class... Types>
struct has_container<Type, Types...>{
public:
    static constexpr bool value = Container<Type> || has_container<Types...>::value;
};

template<>
struct has_container<>{
public:
    static constexpr bool value = false;
};

template<class... Types> struct total_sizeof;

template<class Type, class... Types>
struct total_sizeof<Type,Types...>{
public:
    static constexpr size_t value = sizeof(Type) + total_sizeof<Types...>::value;
};

template<>
struct total_sizeof<>{
public:
    static constexpr size_t value = 0;
};

template<class Type>
concept Array = std::is_array<Type>::value;

template<class Type>
concept NotArray = not Array<Type>;

template<class Type>
class FollowingUint;

template<>
class FollowingUint<uint8_t>{
public:
	using Value = uint16_t;
};

template<>
class FollowingUint<uint16_t>{
public:
	using Value = uint32_t;
};

template<>
class FollowingUint<uint32_t>{
public:
	using Value = uint64_t;
};
