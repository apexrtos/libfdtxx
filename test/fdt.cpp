#include <gtest/gtest.h>

#include "../libfdt++.h"

constexpr std::byte
operator""_b(unsigned long long v)
{
	return static_cast<std::byte>(v);
}

namespace std {

bool
operator==(std::span<const std::byte> l, std::span<const std::byte> r)
{
	return equal(begin(l), end(l), begin(r), end(r));
}

}

TEST(piece, parent)
{
	auto f{fdt::load("path.dtb")};
	const auto &fc{f};

	/* piece::parent */
	EXPECT_EQ(parent(root(f)).has_value(), false);
	EXPECT_EQ(&parent(get_node(f, "/l1@1"))->get(), &root(f));
	EXPECT_EQ(&parent(get_node(f, "/l1@1/l2@1"))->get(), &get_node(f, "/l1@1"));
	EXPECT_EQ(&parent(get_property(f, "/l1@1/l2@1/l1#1-l2#1-prop"))->get(), &get_node(f, "/l1@1/l2@1"));

	/* piece::parent const */
	EXPECT_EQ(parent(root(fc)).has_value(), false);
	EXPECT_EQ(&parent(get_node(fc, "/l1@1"))->get(), &root(fc));
	EXPECT_EQ(&parent(get_node(fc, "/l1@1/l2@1"))->get(), &get_node(fc, "/l1@1"));
	EXPECT_EQ(&parent(get_property(fc, "/l1@1/l2@1/l1#1-l2#1-prop"))->get(), &get_node(fc, "/l1@1/l2@1"));
}

TEST(piece, conversion)
{
	auto f{fdt::load("basic.dtb")};
	auto &property{find(f, "/property").value().get()};
	auto &node{find(f, "/node").value().get()};
	const auto &const_property{property};
	const auto &const_node{node};

	/* is_ */
	EXPECT_EQ(is_property(property), true);
	EXPECT_EQ(is_node(property), false);
	EXPECT_EQ(is_property(node), false);
	EXPECT_EQ(is_node(node), true);

	/* as_(piece &) */
	as_property(property);
	EXPECT_THROW(as_node(property), std::bad_cast);
	as_node(node);
	EXPECT_THROW(as_property(node), std::bad_cast);

	/* as_(const piece &) */
	as_property(const_property);
	EXPECT_THROW(as_node(const_property), std::bad_cast);
	as_node(const_node);
	EXPECT_THROW(as_property(const_node), std::bad_cast);
}

TEST(property, set_u32)
{
	fdt::fdt f;
	auto &p{add_property(root(f), "test")};

	const uint32_t val{0xdeadbeef};
	set(p, val);

	EXPECT_EQ(as_u32(p), val);
}

TEST(property, set_u64)
{
	fdt::fdt f;
	auto &p{add_property(root(f), "test")};

	const uint64_t val{0xdeadbeefcafef00d};
	set(p, val);

	EXPECT_EQ(as_u64(p), val);
}

TEST(property, set_string)
{
	fdt::fdt f;
	auto &p{add_property(root(f), "test")};

	const auto val{"hello world"};
	set(p, val);

	EXPECT_EQ(as_string(p), val);
}

TEST(property, set_stringlist)
{
	fdt::fdt f;
	auto &p{add_property(root(f), "test")};

	const std::vector<std::string_view> val{"hello", "world"};
	set(p, val);

	EXPECT_EQ(as_stringlist(p), val);
}

TEST(property, is_empty)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_EQ(is_empty(get_property(f, "/property-empty")), true);
	EXPECT_EQ(is_empty(get_property(f, "/property-u32")), false);
	EXPECT_EQ(is_empty(get_property(f, "/property-u64")), false);
	EXPECT_EQ(is_empty(get_property(f, "/property-string")), false);
	EXPECT_EQ(is_empty(get_property(f, "/property-stringlist")), false);
}

TEST(property, is_u32)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_EQ(is_u32(get_property(f, "/property-empty")), false);
	EXPECT_EQ(is_u32(get_property(f, "/property-u32")), true);
	EXPECT_EQ(is_u32(get_property(f, "/property-u64")), false);
	EXPECT_EQ(is_u32(get_property(f, "/property-string")), false);
	EXPECT_EQ(is_u32(get_property(f, "/property-stringlist")), false);
}

TEST(property, is_u64)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_EQ(is_u64(get_property(f, "/property-empty")), false);
	EXPECT_EQ(is_u64(get_property(f, "/property-u32")), false);
	EXPECT_EQ(is_u64(get_property(f, "/property-u64")), true);
	EXPECT_EQ(is_u64(get_property(f, "/property-string")), false);
	EXPECT_EQ(is_u64(get_property(f, "/property-stringlist")), false);
}

TEST(property, is_string)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_EQ(is_string(get_property(f, "/property-empty")), false);
	EXPECT_EQ(is_string(get_property(f, "/property-u32")), false);
	EXPECT_EQ(is_string(get_property(f, "/property-u64")), false);
	EXPECT_EQ(is_string(get_property(f, "/property-string")), true);
	EXPECT_EQ(is_string(get_property(f, "/property-stringlist")), false);
}

TEST(property, is_stringlist)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_EQ(is_stringlist(get_property(f, "/property-empty")), false);
	EXPECT_EQ(is_stringlist(get_property(f, "/property-u32")), false);
	EXPECT_EQ(is_stringlist(get_property(f, "/property-u64")), false);
	EXPECT_EQ(is_stringlist(get_property(f, "/property-string")), true);
	EXPECT_EQ(is_stringlist(get_property(f, "/property-stringlist")), true);
}

TEST(property, as_u32)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as_u32(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_EQ(as_u32(get_property(f, "/property-u32")), 32);
	EXPECT_THROW(as_u32(get_property(f, "/property-u64")), std::invalid_argument);
	EXPECT_THROW(as_u32(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as_u32(get_property(f, "/property-stringlist")), std::invalid_argument);
}

TEST(property, as_u64)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as_u64(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as_u64(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_EQ(as_u64(get_property(f, "/property-u64")), 64);
	EXPECT_THROW(as_u64(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as_u64(get_property(f, "/property-stringlist")), std::invalid_argument);
}

TEST(property, as_string)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as_string(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-u64")), std::invalid_argument);
	EXPECT_EQ(as_string(get_property(f, "/property-string")), "hello world");
	EXPECT_THROW(as_string(get_property(f, "/property-stringlist")), std::invalid_argument);
}

TEST(property, as_stringlist)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as_stringlist(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-u64")), std::invalid_argument);
	const std::vector<std::string_view> val_string{"hello world"};
	EXPECT_EQ(as_stringlist(get_property(f, "/property-string")), val_string);
	const std::vector<std::string_view> val_stringlist{"hello", "world"};
	EXPECT_EQ(as_stringlist(get_property(f, "/property-stringlist")), val_stringlist);
}

TEST(property, as_bytes)
{
	const auto &f{fdt::load("properties.dtb")};
	const std::span<std::byte> val_empty;
	EXPECT_EQ(as_bytes(get_property(f, "/property-empty")), val_empty);
	const std::array<std::byte, 4> val_u32{0_b, 0_b, 0_b, 32_b};
	EXPECT_EQ(as_bytes(get_property(f, "/property-u32")), val_u32);
	const std::array<std::byte, 8> val_u64{0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 64_b};
	EXPECT_EQ(as_bytes(get_property(f, "/property-u64")), val_u64);
	std::span<const std::byte> val_string{
		reinterpret_cast<const std::byte *>("hello world"), 12};
	EXPECT_EQ(as_bytes(get_property(f, "/property-string")), val_string);
	std::span<const std::byte> val_stringlist{
		reinterpret_cast<const std::byte *>("hello\0world"), 12};
	EXPECT_EQ(as_bytes(get_property(f, "/property-stringlist")), val_stringlist);
}

TEST(node, name)
{
	fdt::fdt f;

	EXPECT_THROW(add_node(root(f), ""), std::invalid_argument);
	EXPECT_THROW(add_node(root(f), "@"), std::invalid_argument);
	EXPECT_THROW(add_node(root(f), "empty-unit-address@"), std::invalid_argument);
	EXPECT_THROW(add_node(root(f), "@empty-node-name"), std::invalid_argument);
	EXPECT_THROW(add_node(root(f), "this-name-is-longer-than-the-31-character-limit"), std::invalid_argument);
	EXPECT_THROW(add_node(root(f), "!-is-not-allowed"), std::invalid_argument);
	EXPECT_THROW(add_node(root(f), "valid@!-is-not-allowed"), std::invalid_argument);

	auto &n{add_node(root(f), "node-name@unit-address")};

	EXPECT_EQ(name(n), "node-name@unit-address");
	EXPECT_EQ(node_name(n), "node-name");
	EXPECT_EQ(unit_address(n), "unit-address");
}

TEST(node, properties)
{
	auto f{fdt::load("path.dtb")};
	const auto &fc{f};
	std::array<const char *, 2> props{"#address-cells", "#size-cells"};
	size_t i;

	/*
	 * properties(node &)
	 */
	i = 0;
	for (auto &p : properties(root(f))) {
		ASSERT_LT(i, props.size());
		EXPECT_EQ(name(p), props[i]);
		++i;
	};
	EXPECT_EQ(i, props.size());

	/*
	 * properties(const node &)
	 */
	i = 0;
	for (auto &p : properties(root(fc))) {
		ASSERT_LT(i, props.size());
		EXPECT_EQ(name(p), props[i]);
		++i;
	};
	EXPECT_EQ(i, props.size());
}

TEST(node, subnodes)
{
	auto f{fdt::load("path.dtb")};
	const auto &fc{f};
	std::array<const char *, 2> nodes{"l1@1", "l1@2"};
	size_t i;

	/*
	 * subnodes(node &)
	 */
	i = 0;
	for (auto &n : subnodes(root(f))) {
		ASSERT_LT(i, nodes.size());
		EXPECT_EQ(name(n), nodes[i]);
		++i;
	};
	EXPECT_EQ(i, nodes.size());

	/*
	 * subnodes(const node &)
	 */
	i = 0;
	for (auto &n : subnodes(root(fc))) {
		ASSERT_LT(i, nodes.size());
		EXPECT_EQ(name(n), nodes[i]);
		++i;
	};
	EXPECT_EQ(i, nodes.size());
}

TEST(property, name)
{
	fdt::fdt f;

	EXPECT_THROW(add_property(root(f), ""), std::invalid_argument);
	EXPECT_THROW(add_property(root(f), "this-name-is-longer-than-the-31-character-limit"), std::invalid_argument);

	auto &p{add_property(root(f), "property-name")};

	EXPECT_EQ(name(p), "property-name");
}

TEST(node, add_node)
{
	fdt::fdt f;

	auto &n1{add_node(root(f), "n1")};
	auto &n2{add_node(n1, "n2")};
	auto &n3{add_node(n2, "n3")};

	EXPECT_THROW(add_node(n2, "n3"), std::invalid_argument);
	EXPECT_EQ(&get_node(f, "/n1/n2/n3"), &n3);
}

TEST(node, add_property)
{
	fdt::fdt f;

	add_property(root(f), "empty");
	auto &n1{add_node(root(f), "n1")};
	add_property(root(f), "u32", static_cast<uint32_t>(0xdeadbeef));
	add_property(n1, "u64", static_cast<uint64_t>(0xdeadbeefcafef00d));

	EXPECT_THROW(add_property(root(f), "empty"), std::invalid_argument);
	EXPECT_EQ(as_u32(get_property(f, "/u32")), 0xdeadbeef);
	EXPECT_EQ(as_u64(get_property(f, "/n1/u64")), 0xdeadbeefcafef00d);
}

TEST(node, contains)
{
	auto f{fdt::load("path.dtb")};
	EXPECT_EQ(contains(root(f), "l1@1/l2@1/l1#1-l2#1-prop"), true);
	EXPECT_EQ(contains(root(f), "l1@2/l2@1"), true);
	EXPECT_EQ(contains(root(f), "l1@1/l2"), true);
	EXPECT_EQ(contains(root(f), "l1@1/l2/l1#1-l2#1-prop"), true);
	EXPECT_THROW(contains(root(f), "l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(contains(root(f), "/x"), std::invalid_argument);
	EXPECT_EQ(contains(root(f), "x"), false);
}

TEST(node, find)
{
	auto f{fdt::load("path.dtb")};
	const auto &fc{f};

	/* find(node &) */
	EXPECT_EQ(as_u32(as_property(*find(root(f), "l1@1/l2@1/l1#1-l2#1-prop"))), 11);
	EXPECT_EQ(as_u32(as_property(*find(root(f), "l1@2/l2@1/l1#2-l2#1-prop"))), 21);
	EXPECT_EQ(is_node(*find(root(f), "l1@2/l2@1")), true);
	EXPECT_EQ(is_node(*find(root(f), "l1@1/l2")), true);
	EXPECT_EQ(as_u32(as_property(*find(root(f), "l1@1/l2/l1#1-l2#1-prop"))), 11);
	EXPECT_THROW(find(root(f), "l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(find(root(f), "/x"), std::invalid_argument);
	EXPECT_EQ(find(root(f), "x").has_value(), false);

	/* find(const node &) */
	EXPECT_EQ(as_u32(as_property(*find(root(fc), "l1@1/l2@1/l1#1-l2#1-prop"))), 11);
	EXPECT_EQ(as_u32(as_property(*find(root(fc), "l1@2/l2@1/l1#2-l2#1-prop"))), 21);
	EXPECT_EQ(is_node(*find(root(fc), "l1@2/l2@1")), true);
	EXPECT_EQ(is_node(*find(root(fc), "l1@1/l2")), true);
	EXPECT_EQ(as_u32(as_property(*find(root(fc), "l1@1/l2/l1#1-l2#1-prop"))), 11);
	EXPECT_THROW(find(root(fc), "l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(find(root(fc), "/x"), std::invalid_argument);
	EXPECT_EQ(find(root(fc), "x").has_value(), false);
}

TEST(node, get_node)
{
	auto f{fdt::load("path.dtb")};
	auto &n{get_node(f, "/l1@1")};
	const auto &cn{n};

	/* get_node(node &) */
	EXPECT_EQ(name(get_node(n, "l2@1")), "l2@1");
	EXPECT_THROW(get_node(n, "/x"), std::invalid_argument);
	EXPECT_THROW(get_node(n, "x"), std::bad_optional_access);
	EXPECT_THROW(get_node(n, "reg"), std::bad_cast);

	/* get_node(const node &) */
	EXPECT_EQ(name(get_node(cn, "l2@1")), "l2@1");
	EXPECT_THROW(get_node(cn, "/x"), std::invalid_argument);
	EXPECT_THROW(get_node(cn, "x"), std::bad_optional_access);
	EXPECT_THROW(get_node(cn, "reg"), std::bad_cast);
}

TEST(node, get_property)
{
	auto f{fdt::load("path.dtb")};
	auto &n{get_node(f, "/l1@1")};
	const auto &cn{n};

	/* get_property(node &) */
	EXPECT_EQ(name(get_property(n, "reg")), "reg");
	EXPECT_THROW(get_property(n, "/x"), std::invalid_argument);
	EXPECT_THROW(get_property(n, "x"), std::bad_optional_access);
	EXPECT_THROW(get_property(n, "l2@1"), std::bad_cast);

	/* get_property(const node &) */
	EXPECT_EQ(name(get_property(cn, "reg")), "reg");
	EXPECT_THROW(get_property(cn, "/x"), std::invalid_argument);
	EXPECT_THROW(get_property(cn, "x"), std::bad_optional_access);
	EXPECT_THROW(get_property(cn, "l2@1"), std::bad_cast);
}

TEST(fdt, find)
{
	auto f{fdt::load("path.dtb")};
	const auto &fc{f};

	/* find(fdt &) */
	EXPECT_EQ(as_u32(as_property(*find(f, "/l1@1/l2@1/l1#1-l2#1-prop"))), 11);
	EXPECT_EQ(as_u32(as_property(*find(f, "/l1@2/l2@1/l1#2-l2#1-prop"))), 21);
	EXPECT_EQ(is_node(*find(f, "/l1@2/l2@1")), true);
	EXPECT_EQ(is_node(*find(f, "/l1@1/l2")), true);
	EXPECT_EQ(as_u32(as_property(*find(f, "/l1@1/l2/l1#1-l2#1-prop"))), 11);
	EXPECT_THROW(find(f, "/l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(find(f, "x"), std::invalid_argument);
	EXPECT_EQ(find(f, "/x").has_value(), false);

	/* find(fdt node &) */
	EXPECT_EQ(as_u32(as_property(*find(fc, "/l1@1/l2@1/l1#1-l2#1-prop"))), 11);
	EXPECT_EQ(as_u32(as_property(*find(fc, "/l1@2/l2@1/l1#2-l2#1-prop"))), 21);
	EXPECT_EQ(is_node(*find(fc, "/l1@2/l2@1")), true);
	EXPECT_EQ(is_node(*find(fc, "/l1@1/l2")), true);
	EXPECT_EQ(as_u32(as_property(*find(fc, "/l1@1/l2/l1#1-l2#1-prop"))), 11);
	EXPECT_THROW(find(fc, "/l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(find(fc, "x"), std::invalid_argument);
	EXPECT_EQ(find(fc, "/x").has_value(), false);
}

TEST(fdt, get_node)
{
	auto f{fdt::load("path.dtb")};
	const auto &cf{f};

	/* get_node(fdt &) */
	EXPECT_EQ(name(get_node(f, "/l1@2/l2@1")), "l2@1");
	EXPECT_THROW(get_node(f, "x"), std::invalid_argument);
	EXPECT_THROW(get_node(f, "/x"), std::bad_optional_access);
	EXPECT_THROW(get_node(f, "/l1@2/l2@1/l1#2-l2#1-prop"), std::bad_cast);

	/* get_node(const fdt &) */
	EXPECT_EQ(name(get_node(cf, "/l1@2/l2@1")), "l2@1");
	EXPECT_THROW(get_node(cf, "x"), std::invalid_argument);
	EXPECT_THROW(get_node(cf, "/x"), std::bad_optional_access);
	EXPECT_THROW(get_node(cf, "/l1@2/l2@1/l1#2-l2#1-prop"), std::bad_cast);
}

TEST(fdt, get_property)
{
	auto f{fdt::load("path.dtb")};
	const auto &cf{f};

	/* get_property(fdt &) */
	EXPECT_EQ(name(get_property(f, "/l1@1/l2@1/l1#1-l2#1-prop")), "l1#1-l2#1-prop");
	EXPECT_THROW(get_property(f, "x"), std::invalid_argument);
	EXPECT_THROW(get_property(f, "/x"), std::bad_optional_access);
	EXPECT_THROW(get_property(f, "/l1@1"), std::bad_cast);

	/* get_property(const fdt &) */
	EXPECT_EQ(name(get_property(cf, "/l1@1/l2@1/l1#1-l2#1-prop")), "l1#1-l2#1-prop");
	EXPECT_THROW(get_property(cf, "x"), std::invalid_argument);
	EXPECT_THROW(get_property(cf, "/x"), std::bad_optional_access);
	EXPECT_THROW(get_property(cf, "/l1@1"), std::bad_cast);
}

TEST(fdt, equality)
{
	const auto &f1{fdt::load("path.dtb")};
	const auto &f2{fdt::load("path.dtb")};
	const auto &f3{fdt::load("basic.dtb")};

	EXPECT_EQ(f1, f2);
	EXPECT_NE(f2, f3);
}

TEST(fdt, save_load)
{
	const auto &f1{fdt::load("path.dtb")};
	const auto &s{save(f1)};
	const auto &f2{fdt::load(s)};

	EXPECT_EQ(f1, f2);
}
