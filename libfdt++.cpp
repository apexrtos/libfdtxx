/*
 * fdt++ - flattened device tree C++ library.
 *
 * Copyright 2020 Patrick Oppenlander <patrick.oppenlander@gmail.com>
 *
 * SPDX-License-Identifier: 0BSD
 */
#include "libfdt++.h"

#include <algorithm>
#include <fstream>
#include <type_traits>

extern "C" {
#include <libfdt.h>
}

namespace fdt {

namespace {

/*
 * _byte - create a byte literal
 */
constexpr
std::byte
operator""_byte(unsigned long long v)
{
	return static_cast<std::byte>(v);
}

/*
 * is_0_byte - test if byte is equal to 0_byte
 */
constexpr
bool
is_0_byte(std::byte v)
{
	return v == 0_byte;
}

/*
 * fdt_resize - c++ fdt_resize wrapper
 */
void
fdt_resize(std::vector<std::byte> &d, size_t sz = 0)
{
	d.resize(size(d) + std::max(sz, size(d) / 2));
	if (auto r = ::fdt_resize(data(d), data(d), static_cast<int>(size(d)));
							r < 0)
		throw std::runtime_error{fdt_strerror(r)};
}

/*
 * fdt_create - c++ fdt_create wrapper
 */
void
fdt_create(std::vector<std::byte> &d)
{
	int r;
	while ((r = ::fdt_create(data(d), static_cast<int>(size(d)))) ==
							-FDT_ERR_NOSPACE)
		fdt_resize(d, 128);
	if (r < 0)
		throw std::runtime_error{fdt_strerror(r)};
}

/*
 * fdt_finish_reservemap - c++ fdt_finish_reservemap wrapper
 */
void
fdt_finish_reservemap(std::vector<std::byte> &d)
{
	int r;
	while ((r = ::fdt_finish_reservemap(data(d))) == -FDT_ERR_NOSPACE)
		fdt_resize(d);
	if (r < 0)
		throw std::runtime_error{fdt_strerror(r)};
}

/*
 * fdt_begin_node - c++ fdt_begin_node wrapper
 */
void
fdt_begin_node(std::string_view n, std::vector<std::byte> &d)
{
	int r;
	while ((r = ::fdt_begin_node(data(d), data(n))) == -FDT_ERR_NOSPACE)
		fdt_resize(d);
	if (r < 0)
		throw std::runtime_error{fdt_strerror(r)};
}

/*
 * fdt_end_node - c++ fdt_end_node wrapper
 */
void
fdt_end_node(std::vector<std::byte> &d)
{
	int r;
	while ((r = ::fdt_end_node(data(d))) == -FDT_ERR_NOSPACE)
		fdt_resize(d);
	if (r < 0)
		throw std::runtime_error{fdt_strerror(r)};
}

/*
 * fdt_property - c++ fdt_property wrapper
 */
void
fdt_property(std::string_view n, std::span<const std::byte> v,
	     std::vector<std::byte> &d)
{
	int r;
	while ((r = ::fdt_property(data(d), data(n), data(v),
				   static_cast<int>(size(v)))) ==
							-FDT_ERR_NOSPACE)
		fdt_resize(d, size(v));
	if (r < 0)
		throw std::runtime_error{fdt_strerror(r)};
}

/*
 * fdt_finish - c++ fdt_finish wrapper
 */
void
fdt_finish(std::vector<std::byte> &d)
{
	int r;
	while ((r = ::fdt_finish(data(d))) == -FDT_ERR_NOSPACE)
		fdt_resize(d);
	if (r < 0)
		throw std::runtime_error{fdt_strerror(r)};
}

/*
 * load - load FDT node from d starting at node_offset into n
 */
void
load(std::span<const std::byte> d, const int node_offset, node &n)
{
	const void *p = data(d);
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
 * save - save FDT node n into d
 */
void
save(const node &n, std::vector<std::byte> &d)
{
	fdt_begin_node(name(n), d);
	for (const auto &cp : properties(n))
		fdt_property(name(cp), as_bytes(cp), d);
	for (const auto &cn : subnodes(n))
		save(cn, d);
	fdt_end_node(d);
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
	auto c = children(n);
	auto it = lower_bound(begin(c), end(c), nn, [](auto &l, auto &r) {
		return name(l) < r;
	});
	if (it == end(c))
		return std::nullopt;
	/* unit address is optional in node name */
	if (name(*it) != nn && (!is_node(*it) || node_name(as_node(*it)) != nn))
		return std::nullopt;
	/* REVISIT: check for ambiguous path? */
	if (sep == std::string_view::npos)
		return std::ref(*it);
	if (!is_node(*it))
		return std::nullopt;
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
	return
	    (c >= '0' && c <= '9') ||
	    (c >= 'a' && c <= 'z') ||
	    (c >= 'A' && c <= 'Z') ||
	    c == ',' || c == '.' || c == '_' || c == '+' || c == '-';
}

/*
 * valid_property_char - test if character is allowed in property name.
 *
 * See table 2.2 in the devicetree specification.
 */
bool
valid_property_char(char c)
{
	return
	    (c >= '0' && c <= '9') ||
	    (c >= 'a' && c <= 'z') ||
	    (c >= 'A' && c <= 'Z') ||
	    c == ',' || c == '.' || c == '_' || c == '+' ||
	    c == '?' || c == '#' || c == '-';
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
	if (empty(name_))
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
operator==(const piece &l, const piece &r)
{
	return name(l) == name(r) && l.v_equal(r);
}

std::string_view
name(const piece &p)
{
	return p.name();
}

std::optional<std::reference_wrapper<node>>
parent(piece &p)
{
	return p.parent();
}

std::optional<std::reference_wrapper<const node>> parent(const piece &p)
{
	return p.parent();
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

bool
property::v_equal(const piece &r) const
{
	if (!is_property(r))
		return false;
	const auto &lv{get()};
	const auto &rv{as_property(r).get()};
	return std::equal(begin(lv), end(lv), begin(rv), end(rv));
}

void
property::set(std::span<const std::byte> v)
{
	value_.assign(begin(v), end(v));
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
	if (empty(v) || v.back() == 0) {
		set(p, {data(v), size(v)});
		return;
	}

	/* string is not null terminated, create a temporary then assign */
	std::vector<std::byte> t;
	const std::byte *b = reinterpret_cast<const std::byte *>(data(v));
	t.insert(end(t), b, b + size(v));
	t.emplace_back(0_byte);
	set(p, std::move(t));
}

void
set(property &p, const std::vector<std::string_view> &v)
{
	std::vector<std::byte> t;
	for (const auto &s : v) {
		if (empty(s))
			continue;
		const std::byte *b = reinterpret_cast<const std::byte *>(data(s));
		t.insert(end(t), b, b + size(s));
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
	return empty(as_bytes(p));
}

bool
is_u32(const property &p)
{
	return size(as_bytes(p)) == sizeof(uint32_t);
}

bool
is_u64(const property &p)
{
	return size(as_bytes(p)) == sizeof(uint64_t);
}

bool
is_string(const property &p)
{
	/* must be null terminated with no embedded nulls */
	const auto &v = as_bytes(p);
	if (size(v) < 2)
		return false;
	if (find(begin(v), end(v), 0_byte) != end(v) - 1)
		return false;
	return true;
}

bool
is_stringlist(const property &p)
{
	/* must be null terminated, may have embedded nulls, must not be
	 * all null */
	const auto &v = as_bytes(p);
	if (size(v) < 2)
		return false;
	if (v.back() != 0_byte)
		return false;
	return std::find_if_not(begin(v), end(v), is_0_byte) != end(v);
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
	std::copy_n(data(as_bytes(p)), sizeof(t),
		    reinterpret_cast<std::byte *>(&t));
	return fdt32_to_cpu(t);
}

uint64_t
as_u64(const property &p)
{
	if (!is_u64(p))
		throw std::invalid_argument{"not a u64"};
	uint64_t t;
	std::copy_n(data(as_bytes(p)), sizeof(t),
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
	return {reinterpret_cast<const char *>(data(v)), size(v) - 1};
}

std::vector<std::string_view>
as_stringlist(const property &p)
{
	/* REVISIT: expensive sanity check could be optional? */
	if (!is_stringlist(p))
		throw std::invalid_argument{"not a stringlist"};
	std::vector<std::string_view> t;
	const auto &v = as_bytes(p);
	const char *d = reinterpret_cast<const char *>(data(v));
	for (auto it = begin(v); it != end(v);) {
		auto e = std::find(it, end(v), 0_byte) + 1;
		if (e - it > 1)
			t.emplace_back(d + (it - begin(v)), e - it - 1);
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
	if (empty(nn) ||
	    std::find_if_not(begin(nn), end(nn), valid_node_char) != end(nn))
		throw std::invalid_argument{"invalid node name"};
	if (ua.has_value() && (ua->empty() ||
	    std::find_if_not(begin(*ua), end(*ua), valid_node_char) != end(*ua)))
		throw std::invalid_argument{"invalid unit address"};
}

bool
node::v_equal(const piece &r) const
{
	if (!is_node(r))
		return false;
	const auto &lc{children()};
	const auto &rc{as_node(r).children()};
	return std::equal(begin(lc), end(lc), begin(rc), end(rc));
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
	const auto &nn = name(n);
	return nn.substr(0, nn.find('@'));
}

std::optional<std::string_view>
unit_address(const node &n)
{
	const auto &nn = name(n);
	auto at = nn.find('@');
	if (at == std::string_view::npos)
		return std::nullopt;
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

node &
get_node(node &n, std::string_view path)
{
	return as_node(find(n, path).value());
}

const node &
get_node(const node &n, std::string_view path)
{
	return as_node(find(n, path).value());
}

property &
get_property(node &n, std::string_view path)
{
	return as_property(find(n, path).value());
}

const property &
get_property(const node &n, std::string_view path)
{
	return as_property(find(n, path).value());
}

/*
 * fdt
 */
fdt::fdt()
: root_{std::make_unique<node>()}
{ }

node &
fdt::root()
{
	return *root_;
}

const node &
fdt::root() const
{
	return *root_;
}

bool
operator==(const fdt &l, const fdt &r)
{
	/* TODO(incomplete): memory reservation block */
	/* TODO(incomplete): boot cpuid */
	return l.root() == r.root();
}

node &
root(fdt &f)
{
	return f.root();
}

const node &
root(const fdt &f)
{
	return f.root();
}

fdt
load(std::span<const std::byte> d)
{
	/* TODO(efficiency): probably only need a minimal header check here */
	if (auto r = fdt_check_full(data(d), size(d)); r < 0)
		throw std::invalid_argument{fdt_strerror(r)};

	fdt t;
	/* TODO(incomplete): load memory reservation block */
	/* TODO(incomplete): load boot cpuid */
	load(d, 0, root(t));
	return t;
}

fdt
load(const std::filesystem::path &p)
{
	return load_keep(p).first;
}

std::pair<fdt, std::vector<std::byte>>
load_keep(const std::filesystem::path &p)
{
	/* REVISIT(efficiency): something lighter than ifstream? */
	std::vector<std::byte> d;
	std::ifstream f(p, std::ios::binary);

	auto fill = [&](size_t len) {
		const auto prev{d.size()};
		d.resize(len);
		f.read(reinterpret_cast<char *>(data(d)) + prev, len - prev);
		if (static_cast<size_t>(f.gcount()) != len - prev)
			throw std::runtime_error{fdt_strerror(FDT_ERR_TRUNCATED)};
	};

	fill(FDT_V1_SIZE);
	fill(fdt_header_size(data(d)));
	if (auto r = fdt_check_header(data(d)); r < 0)
		throw std::runtime_error{fdt_strerror(r)};
	fill(fdt_totalsize(data(d)));

	return {load(d), std::move(d)};
}

std::vector<std::byte>
save(const fdt &f)
{
	/* REVISIT: this is far from optimal but it's about as good as we can
	 * get without reimplementing libfdt */
	std::vector<std::byte> t{4000};
	fdt_create(t);
	/* TODO(incomplete): save memory reservation block */
	/* TODO(incomplete): save boot cpuid */
	fdt_finish_reservemap(t);
	save(root(f), t);
	fdt_finish(t);
	t.resize(fdt_totalsize(data(t)));
	return t;
}

bool
contains(const fdt &f, std::string_view path)
{
	if (!path.starts_with('/'))
		throw std::invalid_argument{"bad path"};
	return contains(root(f), path.substr(1));
}

std::optional<std::reference_wrapper<const piece>>
find(const fdt &f, std::string_view path)
{
	if (!path.starts_with('/'))
		throw std::invalid_argument{"bad path"};
	return find(root(f), path.substr(1));

}

std::optional<std::reference_wrapper<piece>>
find(fdt &f, std::string_view path)
{
	if (!path.starts_with('/'))
		throw std::invalid_argument{"bad path"};
	return find(root(f), path.substr(1));
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
