/*
 * fdt++ - flattened device tree C++ library
 *
 * Copyright 2020 Patrick Oppenlander <patrick.oppenlander@gmail.com>
 *
 * SPDX-License-Identifier: 0BSD
 */
#pragma once

#include <bit>
#include <cassert>
#include <filesystem>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>
#include <version>

/*
 * Fall back to vector if ranges isn't available
 */
#ifdef __cpp_lib_ranges
#include <ranges>
#endif

namespace fdt {

class node;
class property;

/*
 * piece - a piece of the devicetree structure block
 *
 * This can be a property or a node.
 */
class piece {
public:
	piece() = default;
	piece(node &parent, std::string_view name);

	piece(piece &&) = delete;
	piece(const piece &) = delete;
	piece &operator=(piece &&) = delete;
	piece &operator=(const piece &) = delete;
	virtual ~piece() = 0;

	std::string_view name() const;
	std::optional<std::reference_wrapper<node>> parent();
	std::optional<std::reference_wrapper<const node>> parent() const;

private:
	virtual bool v_equal(const piece &) const = 0;

	const std::optional<std::reference_wrapper<node>> parent_{std::nullopt};
	const std::string name_;

	friend bool operator==(const piece &, const piece &);
};

bool operator==(const piece &, const piece &);

/*
 * name(piece &) - get name of a piece
 *
 * For a property, this is the property name.
 * For a node this is the node name including unit address.
 */
std::string_view name(const piece &);

/*
 * path(piece &) - get path from root of tree to piece
 */
std::string path(const piece &);

/*
 * parent(piece &) - get node containing piece
 *
 * Root node has no parent.
 */
std::optional<std::reference_wrapper<node>> parent(piece &);
std::optional<std::reference_wrapper<const node>> parent(const piece &);

/*
 * root(piece &) - get root node of flattened device tree
 */
node &root(piece &);
const node &root(const piece &);

/*
 * is_*(piece &) - test if piece is property or node
 */
bool is_property(const piece &);
bool is_node(const piece &);

/*
 * as_*(piece &) - convert piece to property or node
 *
 * Throws std::bad_cast if the piece cannot be converted.
 */
property& as_property(piece &);
const property& as_property(const piece &);
node& as_node(piece &);
const node& as_node(const piece &);

/*
 * property - a devicetree property
 */
class property : public piece {
public:
	using container = std::vector<std::byte>;

	property(node &parent, std::string_view name);

	std::span<const std::byte> get() const;
	void set(container &&);
	void set(std::span<const std::byte>);

private:
	virtual bool v_equal(const piece &) const override;

	container value_;
};

/*
 * set(property &, *) - set property value
 */
void set(property &, uint32_t);
void set(property &, uint64_t);
void set(property &, std::string_view);
void set(property &, const std::vector<std::string_view> &);
void set(property &, property::container &&);
void set(property &, std::span<const std::byte>);

/*
 * is_*(property &) - test if property value can be converted to type
 *
 * These tests are not mutually exclusive as the type of a property is not
 * encoded in the FDT. For example, is<uint32_t>, is_string and is_stringlist
 * will all return true for the property [0x70 0x6f 0x6f 0x00].
 *
 * is<T> supports integral types, types for which std::tuple_size is defined
 * and composite types thereof.
 */
bool is_empty(const property &);
bool is_string(const property &);
bool is_stringlist(const property &);
template<class T> bool is(const property &);
template<class T> bool is_array(const property &);

/*
 * as_*(property &) - convert property to type
 *
 * Throws std::invalid_argument if the property can not be converted.
 *
 * Strings and bytes are returned as references to the property value.
 *
 * as<T> and as_array<T> support integral types, types for which
 * std::tuple_size is defined and composite types thereof.
 */
std::string_view as_string(const property &);
std::vector<std::string_view> as_stringlist(const property &);
template<class T> T as(const property &);
template<class T> auto as_array(const property &);
std::span<const std::byte> as_bytes(const property &);

/*
 * node - a devicetree node
 */
class node : public piece {
	using piece_p = std::unique_ptr<piece>;

	struct set_compare {
		using is_transparent = void;

		template<class Key>
		bool operator()(const Key &, const piece_p &) const;
		template<class Key>
		bool operator()(const piece_p &, const Key &) const;
		bool operator()(const piece_p &, const piece_p &) const;
	};
	using piece_set = std::set<piece_p, set_compare>;

public:
	node() = default;
	node(node &parent, std::string_view name);

	auto children();
	auto children() const;

	template<class T, class ...A>
	T& add(std::string_view name, A &&...);

private:
	virtual bool v_equal(const piece &) const override;

	piece_set children_;
};

/*
 * node_name - get node-name part of node name
 */
std::string_view node_name(const node &);

/*
 * unit_address - get unit-address part of node name
 */
std::optional<std::string_view> unit_address(const node &);

/*
 * children - get node children
 *
 * Returns an iterable container of piece references.
 */
template<class Node>
auto children(Node &);

/*
 * properties - get node properties
 *
 * Returns an iterable container of property references.
 */
template<class Node>
auto properties(Node &);

/*
 * subnodes - get node subnodes
 *
 * Returns an iterable container of node references.
 */
template<class Node>
auto subnodes(Node &);

/*
 * add_node - add a subnode to a node
 *
 * Throws std::invalid_argument if the name already exists.
 */
node& add_node(node &, std::string_view name);

/*
 * add_property - add a property to a node
 *
 * Throws std::invalid_argument if the name already exists.
 */
property& add_property(node &, std::string_view name);
template<class ...T>
property& add_property(node &, std::string_view name, T &&...value);

/*
 * contains - test if node contains path
 *
 * This function takes a path relative to the node.
 *
 * Throws std::invalid_argument if the path format is invalid.
 */
bool contains(const node &, std::string_view path);

/*
 * find - find child of node by path
 *
 * This function takes a path relative to the node.
 *
 * Throws std::invalid_argument if the path format is invalid.
 */
std::optional<std::reference_wrapper<const piece>>
find(const node &, std::string_view path);

std::optional<std::reference_wrapper<piece>>
find(node &, std::string_view path);

/*
 * get_node - get a node by path
 *
 * Throws
 *	std::invalid_argument if the path format is invalid.
 *	std::bad_optional_access if the path does not exist.
 *	std::bad_cast if the path does not refer to a node.
 */
node& get_node(node &, std::string_view path);
const node& get_node(const node &, std::string_view path);

/*
 * get_property - get a property by path
 *
 * Throws
 *	std::invalid_argument if the path format is invalid.
 *	std::bad_optional_access if the path does not exist.
 *	std::bad_cast if the path does not refer to a property.
 */
property& get_property(node &, std::string_view path);
const property& get_property(const node &, std::string_view path);

/*
 * fdt
 */
class fdt {
public:
	fdt();

	fdt(fdt &&) = default;
	fdt(const fdt &) = delete;
	fdt &operator=(fdt &&) = default;
	fdt &operator=(const fdt &) = delete;
	~fdt() = default;

	/* TODO(incomplete): memory reservation block */
	/* TODO(incomplete): boot cpuid */

	node& root();
	const node& root() const;

private:
	std::unique_ptr<node> root_;
};

bool operator==(const fdt &, const fdt &);

/*
 * root(fdt &) - get root node of flattened device tree
 */
node& root(fdt &);
const node& root(const fdt &);

/*
 * load - load a flattened devicetree blob
 * load_keep - load a flattened devicetree blob and return loaded bytes
 *
 * Throws exceptions.
 */
fdt load(std::span<const std::byte>);
fdt load(int fd);
fdt load(const std::filesystem::path &);
std::pair<fdt, std::vector<std::byte>> load_keep(int fd);
std::pair<fdt, std::vector<std::byte>> load_keep(const std::filesystem::path &);

/*
 * save - save a flattened devicetree blob
 *
 * Throws exceptions.
 */
std::vector<std::byte> save(const fdt &);

/*
 * contains - test if fdt contains path
 *
 * Throws std::invalid_argument if the path format is invalid.
 */
bool contains(const fdt &, std::string_view path);

/*
 * find - find piece of fdt by path
 *
 * Throws std::invalid_argument if the path format is invalid.
 */
std::optional<std::reference_wrapper<const piece>>
find(const fdt &, std::string_view path);

std::optional<std::reference_wrapper<piece>>
find(fdt &, std::string_view path);

/*
 * get_node - get a node by path
 *
 * Throws
 *	std::invalid_argument if the path format is invalid.
 *	std::bad_optional_access if the path does not exist.
 *	std::bad_cast if the path does not refer to a node.
 */
node& get_node(fdt &, std::string_view path);
const node& get_node(const fdt &, std::string_view path);

/*
 * get_property - get a property by path
 *
 * Throws
 *	std::invalid_argument if the path format is invalid.
 *	std::bad_optional_access if the path does not exist.
 *	std::bad_cast if the path does not refer to a property.
 */
property& get_property(fdt &, std::string_view path);
const property& get_property(const fdt &, std::string_view path);

/*
 * implementation details
 */
namespace dtl {

template<class...> inline constexpr bool false_v = false;

template<typename T>
constexpr size_t
byte_size()
{
	if constexpr (std::is_integral_v<T>)
		return sizeof(T);
	else {
		/* REVISIT: can we do this without constructing a T? */
		return std::apply([](const auto &...v) {
			return (byte_size<std::decay_t<decltype(v)>>() + ...);
		}, T{});
	}
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>, T>
byteswap(T v)
{
	if constexpr (std::endian::native == std::endian::big)
		return v;
	else if constexpr (sizeof(T) == 1)
		return v;
#ifdef _MSC_VER
	else if constexpr (sizeof(T) == sizeof(unsigned short))
		return _byteswap_ushort(v);
	else if constexpr (sizeof(T) == sizeof(unsigned long))
		return _byteswap_ulong(v);
	else if constexpr (sizeof(T) == sizeof(uint64_t))
		return _byteswap_uint64(v);
#else
	else if constexpr (sizeof(T) == 2)
		return __builtin_bswap16(v);
	else if constexpr (sizeof(T) == 4)
		return __builtin_bswap32(v);
	else if constexpr (sizeof(T) == 8)
		return __builtin_bswap64(v);
#endif
	else
		static_assert(false_v<T>, "can't byteswap type");
}

template<typename T>
T
read_advance(std::span<const std::byte> &d)
{
	assert(d.size() >= byte_size<T>());
	T t;
	if constexpr (std::is_integral_v<T>) {
		std::copy_n(data(d), sizeof(T), reinterpret_cast<std::byte *>(&t));
		t = byteswap(t);
		d = d.subspan(sizeof(t));
	} else {
		std::apply([&d](auto &...v) {
			((v = read_advance<std::decay_t<decltype(v)>>(d)), ...);
		}, t);
	}
	return t;
}

template<typename T>
T
read(std::span<const std::byte> d)
{
	return read_advance<T>(d);
}

}

template<class T>
bool
is(const property &p)
{
	return size(as_bytes(p)) == dtl::byte_size<T>();
}

template<class T>
bool
is_array(const property &p)
{
	if (is_empty(p))
		return false;
	return size(as_bytes(p)) % dtl::byte_size<T>() == 0;
}

template<class T>
T
as(const property &p)
{
	if (!is<T>(p))
		throw std::invalid_argument{"incompatible type"};
	return dtl::read<T>(as_bytes(p));
}

template<class T>
auto
as_array(const property &p)
{
	if (!is_array<T>(p))
		throw std::invalid_argument{"incompatible type"};
	const auto sz{dtl::byte_size<T>()};
	const auto n{size(as_bytes(p)) / sz};
#ifdef __cpp_lib_ranges
	return std::views::iota(decltype(n){0}, n) | std::views::transform(
		[&p, sz](auto i) {
			return dtl::read<T>(as_bytes(p).subspan(i * sz));
	});
#else
	std::vector<T> t;
	t.reserve(n);
	for (size_t i{0}; i != n; ++i)
		t.push_back(dtl::read<T>(as_bytes(p).subspan(i * sz)));
	return t;
#endif
}

template<class Key>
bool
node::set_compare::operator()(const Key &l, const piece_p &r) const
{
	return l < r->name();
}

template<class Key>
bool
node::set_compare::operator()(const piece_p &l, const Key &r) const
{
	return l->name() < r;
}

inline auto
node::children()
{
	/* REVISIT: can we deduplicate const & non-const overloads? */
#ifdef __cpp_lib_ranges
	return children_ | std::views::transform(
		[](auto &p) -> piece & {
			return *p;
	});
#else
	std::vector<std::reference_wrapper<piece>> t;
	t.reserve(children_.size());
	for (auto &p : children_)
		t.push_back(std::ref(*p));
	return t;
#endif
}

inline auto
node::children() const
{
	/* REVISIT: can we deduplicate const & non-const overloads? */
#ifdef __cpp_lib_ranges
	return children_ | std::views::transform(
		[](auto &p) -> const piece & {
			return *p;
	});
#else
	std::vector<std::reference_wrapper<const piece>> t;
	t.reserve(children_.size());
	for (const auto &p : children_)
		t.push_back(std::cref(*p));
	return t;
#endif
}

template<class T, class ...A>
T &
node::add(std::string_view name, A &&...a)
{
	auto r = children_.emplace(std::make_unique<T>(*this, name,
						       std::forward<A>(a)...));
	if (!r.second)
		throw std::invalid_argument{"name exists"};
	return static_cast<T &>(**r.first);
}

template<class Node>
auto
children(Node &n)
{
	return n.children();
}

template<class Node>
auto
properties(Node &n)
{
#ifdef __cpp_lib_ranges
	/* REVISIT: can we avoid using a lambda here? */
	return children(n) | std::views::filter(is_property) |
	    std::views::transform([](auto &c) -> decltype((as_property(c))) {
		return as_property(c);
	});
#else
	auto c{children(n)};
	std::vector<std::reference_wrapper<std::remove_reference_t<decltype((as_property(c.front().get())))>>> t;
	for (auto &p : c) {
		if (!is_property(p))
			continue;
		t.push_back(std::ref(as_property(p)));
	}
	return t;
#endif
}

template<class Node>
auto
subnodes(Node &n)
{
#ifdef __cpp_lib_ranges
	/* REVISIT: can we avoid using a lambda here? */
	return children(n) | std::views::filter(is_node) |
	    std::views::transform([](auto &c) -> decltype((as_node(c))) {
		return as_node(c);
	});
#else
	auto c{children(n)};
	std::vector<std::reference_wrapper<std::remove_reference_t<decltype((as_node(c.front().get())))>>> t;
	for (auto &cn : c) {
		if (!is_node(cn))
			continue;
		t.push_back(std::ref(as_node(cn)));
	}
	return t;
#endif
}

template<class ...T>
property &
add_property(node &n, std::string_view name, T &&...value)
{
	auto &p = add_property(n, name);
	set(p, std::forward<T>(value)...);
	return p;
}

}
