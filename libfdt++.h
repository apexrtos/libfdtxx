/*
 * fdt++ - flattened device tree C++ library.
 *
 * Copyright 2020 Patrick Oppenlander <patrick.oppenlander@gmail.com>
 *
 * SPDX-License-Identifier: 0BSD
 */
#pragma once

#include <memory>
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
 * piece - a piece of the devicetree structure block.
 *
 * This can be a property or a node.
 */
class piece {
public:
	piece() = default;
	piece(node &parent, std::string_view name);
	virtual ~piece() = 0;

	/*
	 * name() - get name of piece.
	 *
	 * For a property, this is the property name.
	 * For a node this is the node name including unit address.
	 */
	std::string_view name() const;

	/*
	 * parent() - get node containing piece.
	 *
	 * Root node has no parent.
	 */
	std::optional<std::reference_wrapper<node>> parent();
	std::optional<std::reference_wrapper<const node>> parent() const;

private:
	std::optional<std::reference_wrapper<node>> parent_;
	std::string name_;
};

/*
 * is_*(piece &) - test if piece is property or node.
 */
bool is_property(const piece &);
bool is_node(const piece &);

/*
 * as_*(piece &) - convert piece to property or node.
 *
 * Throws std::bad_cast if the piece cannot be converted.
 */
property& as_property(piece &);
const property& as_property(const piece &);
node& as_node(piece &);
const node& as_node(const piece &);

/*
 * property - a devicetree property.
 */
class property : public piece {
public:
	using container = std::vector<std::byte>;

	property(node &parent, std::string_view name);

	/*
	 * get() - get property value.
	 */
	std::span<const std::byte> get() const;

	/*
	 * set() - set property value.
	 */
	void set(container &&);
	void set(std::span<const std::byte>);

private:
	container value_;
};

/*
 * set(property &, *) - set property value.
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
 * encoded in the FDT. For example, is_u32, is_string and is_stringlist will
 * all return true for the property [0x70 0x6f 0x6f 0x00].
 */
bool is_empty(const property &);
bool is_u32(const property &);
bool is_u64(const property &);
bool is_string(const property &);
bool is_stringlist(const property &);

/*
 * as_*(property &) - convert property to type
 *
 * Throws std::invalid_argument if the property can not be converted.
 *
 * Strings and bytes are returned as references to the property value.
 */
uint32_t as_u32(const property &);
uint64_t as_u64(const property &);
std::string_view as_string(const property &);
std::vector<std::string_view> as_stringlist(const property &);
std::span<const std::byte> as_bytes(const property &);

/*
 * node - a devicetree node.
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

	/*
	 * children - get node children.
	 *
	 * Returns an iterable container of piece references.
	 */
	auto children();
	auto children() const;

	/*
	 * add - add a property or subnode to node.
	 */
	template<class T, class ...A>
	T& add(std::string_view name, A &&...);

private:
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
 * fdt
 */
class fdt {
public:
	/* TODO(incomplete): memory reservation block */
	/* TODO(incomplete): boot cpuid */

	node& root();
	const node& root() const;

private:
	node root_;
};

/*
 * load - load a flattened devicetree blob
 */
fdt load(std::span<const std::byte>);

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
	for (auto &p : children_)
		t.push_back(std::ref(*p));
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

template<class ...T>
property &
add_property(node &n, std::string_view name, T &&...value)
{
	auto &p = add_property(n, name);
	set(p, std::forward<T>(value)...);
	return p;
}

};
