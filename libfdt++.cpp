/*
 * fdt++ - flattened device tree C++ library.
 *
 * Copyright 2020 Patrick Oppenlander <patrick.oppenlander@gmail.com>
 *
 * SPDX-License-Identifier: 0BSD
 */
#include "libfdt++.h"

#include <algorithm>
#include <type_traits>

extern "C" {
#include <libfdt.h>
}

namespace fdt {

namespace {

/*
 * _byte - create a byte literal
 */
constexpr std::byte operator""_byte(unsigned long long v)
{
	return static_cast<std::byte>(v);
}

/*
 * is_0_byte - test if byte is equal to 0_byte
 */
constexpr auto is_0_byte = [](std::byte v) {
	return v == 0_byte;
};

/*
 * load - load FDT node from d starting at node_offset into n
 */
void
load(std::span<const std::byte> d, const int node_offset, node &n)
{
	const void *p = d.data();
	int off;

	fdt_for_each_property_offset(off, p, node_offset) {
		const char *name;
		int len;
		const void *val = fdt_getprop_by_offset(p, off, &name, &len);
		if (!val)
			throw std::invalid_argument{fdt_strerror(len)};
		std::span<const std::byte> value(
			reinterpret_cast<const std::byte *>(val), len);
		add_property(n, name, std::move(value));
	}
	if (off < 0 && off != -FDT_ERR_NOTFOUND)
		throw std::invalid_argument{fdt_strerror(off)};

	fdt_for_each_subnode(off, p, node_offset) {
		int len;
		const char *name = fdt_get_name(p, off, &len);
		if (!name)
			throw std::invalid_argument{fdt_strerror(len)};
		load(d, off, add_node(n, name));
	}
	if (off < 0 && off != -FDT_ERR_NOTFOUND)
		throw std::invalid_argument{fdt_strerror(off)};
}

/*
 * find_impl - find a piece of the FDT by path
 */
template<class T>
std::optional<std::reference_wrapper<
	std::conditional_t<std::is_const_v<T>, const piece, piece>>>
find_impl(T &n, std::string_view path)
{
	auto sep = path.find('/');
	auto nn = path.substr(0, sep);
	if (empty(nn))
		throw std::invalid_argument{"bad path"};
	auto c = n.children();
	auto it = lower_bound(c.begin(), c.end(), nn, [](auto &l, auto &r) {
		return l.name() < r;
	});
	if (it == c.end())
		return {};
	/* unit address is optional in node name */
	/* WTF: transform_view::iterator has no operator-> */
	if ((*it).name() != nn && (!is_node(*it) || node_name(as_node(*it)) != nn))
		return {};
	/* REVISIT: check for ambiguous path? */
	if (sep == std::string_view::npos)
		return std::ref(*it);
	if (!is_node(*it))
		return {};
	return find_impl(as_node(*it), path.substr(sep + 1));
}

/*
 * valid_node_char - test if character is allowed in node name or unit address.
 *
 * See table 2.1 in the devicetree specification.
 */
bool
valid_node_char(char c)
{
	switch (c) {
	case '0'...'9':
	case 'a'...'z':
	case 'A'...'Z':
	case ',':
	case '.':
	case '_':
	case '+':
	case '-':
		return true;
	}
	return false;
}

/*
 * valid_property_char - test if character is allowed in property name.
 *
 * See table 2.2 in the devicetree specification.
 */
bool
valid_property_char(char c)
{
	switch (c) {
	case '0'...'9':
	case 'a'...'z':
	case 'A'...'Z':
	case ',':
	case '.':
	case '_':
	case '+':
	case '?':
	case '#':
	case '-':
		return true;
	}
	return false;
}

}

/*
 * piece
 */
piece::piece(node &parent, std::string_view name)
: parent_{std::ref(parent)}
, name_{name}
{
	/* REVISIT: optionally validate names? */
	if (name_.empty())
		throw std::invalid_argument{"empty name"};
}

piece::~piece() = default;

std::string_view
piece::name() const
{
	return name_;
}

std::optional<std::reference_wrapper<node>>
piece::parent()
{
	return parent_;
}

std::optional<std::reference_wrapper<const node>>
piece::parent() const
{
	return parent_;
}

bool
is_property(const piece &p)
{
	return dynamic_cast<const property *>(&p) != nullptr;
}

bool
is_node(const piece &p)
{
	return dynamic_cast<const node *>(&p) != nullptr;
}

property &
as_property(piece &p)
{
	return dynamic_cast<property &>(p);
}

const property &
as_property(const piece &p)
{
	return dynamic_cast<const property &>(p);
}

node &
as_node(piece &p)
{
	return dynamic_cast<node &>(p);

}

const node &
as_node(const piece &p)
{
	return dynamic_cast<const node &>(p);
}

/*
 * property
 */
property::property(node &parent, std::string_view name)
: piece{parent, name}
{
	/* REVISIT: optionally validate names? */
	const auto &n = name;
	if (size(n) > 31)
		throw std::invalid_argument{"property name too long"};
	if (std::find_if_not(begin(n), end(n), valid_property_char) != end(n))
		throw std::invalid_argument{"invalid property name"};
}

std::span<const std::byte>
property::get() const
{
	return value_;
}

void
property::set(container &&v)
{
	value_ = std::move(v);
}

void
property::set(std::span<const std::byte> v)
{
	value_.assign(v.begin(), v.end());
}

void
set(property &p, uint32_t v)
{
	v = cpu_to_fdt32(v);
	set(p, {reinterpret_cast<std::byte *>(&v), sizeof(v)});
}

void
set(property &p, uint64_t v)
{
	v = cpu_to_fdt64(v);
	set(p, {reinterpret_cast<std::byte *>(&v), sizeof(v)});
}

void
set(property &p, std::string_view v)
{
	/* string is empty or null teriminated, ok to assign */
	if (v.empty() || v.back() == 0) {
		set(p, {v.data(), v.size()});
		return;
	}

	/* string is not null terminated, create a temporary then assign */
	std::vector<std::byte> t;
	const std::byte *b = reinterpret_cast<const std::byte *>(v.data());
	t.insert(t.end(), b, b + v.size());
	t.emplace_back(0_byte);
	set(p, std::move(t));
}

void
set(property &p, const std::vector<std::string_view> &v)
{
	std::vector<std::byte> t;
	for (const auto &s : v) {
		if (v.empty())
			continue;
		const std::byte *b = reinterpret_cast<const std::byte *>(v.data());
		t.insert(t.end(), b, b + v.size());
		if (s.back() != 0)
			t.emplace_back(0_byte);
	}
	set(p, std::move(t));
}

void
set(property &p, property::container &&v)
{
	p.set(std::move(v));
}

void
set(property &p, std::span<const std::byte> v)
{
	p.set(v);
}

bool
is_empty(const property &p)
{
	return as_bytes(p).empty();
}

bool
is_u32(const property &p)
{
	return as_bytes(p).size() == sizeof(uint32_t);
}

bool
is_u64(const property &p)
{
	return as_bytes(p).size() == sizeof(uint64_t);
}

bool
is_string(const property &p)
{
	/* must be null terminated with no embedded nulls */
	const auto &v = as_bytes(p);
	if (v.size() < 2)
		return false;
	if (find(v.begin(), v.end(), 0_byte) != v.end() - 1)
		return false;
	return true;
}

bool
is_stringlist(const property &p)
{
	/* must be null terminated, may have embedded nulls, must not be
	 * all null */
	const auto &v = as_bytes(p);
	if (v.size() < 2)
		return false;
	if (v.back() != 0_byte)
		return false;
	return std::find_if_not(v.begin(), v.end(), is_0_byte) != v.end();
}

/*
 * as_*
 */
uint32_t
as_u32(const property &p)
{
	if (!is_u32(p))
		throw std::invalid_argument{"not a u32"};
	uint32_t t;
	std::copy_n(as_bytes(p).data(), sizeof(t),
		    reinterpret_cast<std::byte *>(&t));
	return fdt32_to_cpu(t);
}

uint64_t
as_u64(const property &p)
{
	if (!is_u64(p))
		throw std::invalid_argument{"not a u64"};
	uint64_t t;
	std::copy_n(as_bytes(p).data(), sizeof(t),
		    reinterpret_cast<std::byte *>(&t));
	return fdt64_to_cpu(t);
}

std::string_view
as_string(const property &p)
{
	/* REVISIT: expensive sanity check could be optional? */
	if (!is_string(p))
		throw std::invalid_argument{"not a string"};
	const auto &v = as_bytes(p);
	return {reinterpret_cast<const char *>(v.data()), v.size() - 1};
}

std::vector<std::string_view>
as_stringlist(const property &p)
{
	/* REVISIT: expensive sanity check could be optional? */
	if (!is_stringlist(p))
		throw std::invalid_argument{"not a stringlist"};
	std::vector<std::string_view> t;
	const auto &v = as_bytes(p);
	const char *d = reinterpret_cast<const char *>(v.data());
	for (auto it = v.begin(); it != v.end();) {
		auto e = std::find(it, v.end(), 0_byte) + 1;
		if (e - it > 1)
			t.emplace_back(d + (it - v.begin()), e - it - 1);
		it = e;
	}
	return t;
}

std::span<const std::byte>
as_bytes(const property &p)
{
	return p.get();
}

/*
 * node
 */
bool
node::set_compare::operator()(const piece_p &l, const piece_p &r) const
{
	return l->name() < r->name();
}

node::node(node &parent, std::string_view name)
: piece{parent, name}
{
	/* REVISIT: optionally validate names? */
	const auto &nn{node_name(*this)};
	const auto &ua{unit_address(*this)};
	if (size(nn) > 31)
		throw std::invalid_argument{"node name too long"};
	if (std::find_if_not(begin(nn), end(nn), valid_node_char) != end(nn))
		throw std::invalid_argument{"invalid node name"};
	if (std::find_if_not(begin(ua), end(ua), valid_node_char) != end(ua))
		throw std::invalid_argument{"invalid unit address"};
}

node &
add_node(node &n, std::string_view name)
{
	return n.add<node>(name);
}

property &
add_property(node &n, std::string_view name)
{
	return n.add<property>(name);
}

std::string_view
node_name(const node &n)
{
	const auto &nn = n.name();
	return nn.substr(0, nn.find('@'));
}

std::string_view
unit_address(const node &n)
{
	const auto &nn = n.name();
	auto at = nn.find('@');
	if (at == std::string_view::npos)
		return {};
	return nn.substr(at + 1);
}

bool
contains(const node &n, std::string_view path)
{
	return find(n, path).has_value();
}

std::optional<std::reference_wrapper<const piece>>
find(const node &n, std::string_view path)
{
	return find_impl(n, path);
}

std::optional<std::reference_wrapper<piece>>
find(node &n, std::string_view path)
{
	return find_impl(n, path);
}

/*
 * fdt
 */
node &
fdt::root()
{
	return root_;
}

const node &
fdt::root() const
{
	return root_;
}

fdt
load(std::span<const std::byte> d)
{
	/* TODO(efficiency): probably only need a minimal header check here */
	if (auto r = fdt_check_full(d.data(), d.size()); r < 0)
		throw std::invalid_argument{fdt_strerror(r)};

	fdt t;
	/* TODO(incomplete): load memory reservation block */
	/* TODO(incomplete): load boot cpuid */
	load(d, 0, t.root());
	return t;
}

bool
contains(const fdt &f, std::string_view path)
{
	if (!path.starts_with('/'))
		throw std::invalid_argument{"bad path"};
	return contains(f.root(), path.substr(1));
}

std::optional<std::reference_wrapper<const piece>>
find(const fdt &f, std::string_view path)
{
	if (!path.starts_with('/'))
		throw std::invalid_argument{"bad path"};
	return find(f.root(), path.substr(1));

}

std::optional<std::reference_wrapper<piece>>
find(fdt &f, std::string_view path)
{
	if (!path.starts_with('/'))
		throw std::invalid_argument{"bad path"};
	return find(f.root(), path.substr(1));
}

node &
get_node(fdt &f, std::string_view path)
{
	return as_node(find(f, path).value());
}

const node &
get_node(const fdt &f, std::string_view path)
{
	return as_node(find(f, path).value());
}

property &
get_property(fdt &f, std::string_view path)
{
	return as_property(find(f, path).value());
}

const property &
get_property(const fdt &f, std::string_view path)
{
	return as_property(find(f, path).value());
}

}
