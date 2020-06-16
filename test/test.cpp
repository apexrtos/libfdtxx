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
	EXPECT_EQ(f.root().parent().has_value(), false);
	EXPECT_EQ(&get_node(f, "/l1@1").parent()->get(), &f.root());
	EXPECT_EQ(&get_node(f, "/l1@1/l2@1").parent()->get(), &get_node(f, "/l1@1"));
	EXPECT_EQ(&get_property(f, "/l1@1/l2@1/l1#1-l2#1-prop").parent()->get(), &get_node(f, "/l1@1/l2@1"));

	/* piece::parent const */
	EXPECT_EQ(fc.root().parent().has_value(), false);
	EXPECT_EQ(&get_node(fc, "/l1@1").parent()->get(), &fc.root());
	EXPECT_EQ(&get_node(fc, "/l1@1/l2@1").parent()->get(), &get_node(fc, "/l1@1"));
	EXPECT_EQ(&get_property(fc, "/l1@1/l2@1/l1#1-l2#1-prop").parent()->get(), &get_node(fc, "/l1@1/l2@1"));
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
	auto &p{add_property(f.root(), "test")};

	const uint32_t val{0xdeadbeef};
	set(p, val);

	EXPECT_EQ(as_u32(p), val);
}

TEST(property, set_u64)
{
	fdt::fdt f;
	auto &p{add_property(f.root(), "test")};

	const uint64_t val{0xdeadbeefcafef00d};
	set(p, val);

	EXPECT_EQ(as_u64(p), val);
}

TEST(property, set_string)
{
	fdt::fdt f;
	auto &p{add_property(f.root(), "test")};

	const auto val{"hello world"};
	set(p, val);

	EXPECT_EQ(as_string(p), val);
}

TEST(property, set_stringlist)
{
	fdt::fdt f;
	auto &p{add_property(f.root(), "test")};

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

	EXPECT_THROW(add_node(f.root(), ""), std::invalid_argument);
	EXPECT_THROW(add_node(f.root(), "@"), std::invalid_argument);
	EXPECT_THROW(add_node(f.root(), "empty-unit-address@"), std::invalid_argument);
	EXPECT_THROW(add_node(f.root(), "@empty-node-name"), std::invalid_argument);
	EXPECT_THROW(add_node(f.root(), "this-name-is-longer-than-the-31-character-limit"), std::invalid_argument);
	EXPECT_THROW(add_node(f.root(), "!-is-not-allowed"), std::invalid_argument);
	EXPECT_THROW(add_node(f.root(), "valid@!-is-not-allowed"), std::invalid_argument);

	auto &n{add_node(f.root(), "node-name@unit-address")};

	EXPECT_EQ(n.name(), "node-name@unit-address");
	EXPECT_EQ(node_name(n), "node-name");
	EXPECT_EQ(unit_address(n), "unit-address");
}

TEST(property, name)
{
	fdt::fdt f;

	EXPECT_THROW(add_property(f.root(), ""), std::invalid_argument);
	EXPECT_THROW(add_property(f.root(), "this-name-is-longer-than-the-31-character-limit"), std::invalid_argument);

	auto &p{add_property(f.root(), "property-name")};

	EXPECT_EQ(p.name(), "property-name");
}

TEST(node, add_node)
{
	fdt::fdt f;

	auto &n1{add_node(f.root(), "n1")};
	auto &n2{add_node(n1, "n2")};
	auto &n3{add_node(n2, "n3")};

	EXPECT_THROW(add_node(n2, "n3"), std::invalid_argument);
	EXPECT_EQ(&get_node(f, "/n1/n2/n3"), &n3);
}

TEST(node, add_property)
{
	fdt::fdt f;

	add_property(f.root(), "empty");
	auto &n1{add_node(f.root(), "n1")};
	add_property(f.root(), "u32", static_cast<uint32_t>(0xdeadbeef));
	add_property(n1, "u64", static_cast<uint64_t>(0xdeadbeefcafef00d));

	EXPECT_THROW(add_property(f.root(), "empty"), std::invalid_argument);
	EXPECT_EQ(as_u32(get_property(f, "/u32")), 0xdeadbeef);
	EXPECT_EQ(as_u64(get_property(f, "/n1/u64")), 0xdeadbeefcafef00d);
}

TEST(node, contains)
{
	auto f{fdt::load("path.dtb")};
	EXPECT_EQ(contains(f.root(), "l1@1/l2@1/l1#1-l2#1-prop"), true);
	EXPECT_EQ(contains(f.root(), "l1@2/l2@1"), true);
	EXPECT_EQ(contains(f.root(), "l1@1/l2"), true);
	EXPECT_EQ(contains(f.root(), "l1@1/l2/l1#1-l2#1-prop"), true);
	EXPECT_THROW(contains(f.root(), "l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(contains(f.root(), "/x"), std::invalid_argument);
	EXPECT_EQ(contains(f.root(), "x"), false);
}

TEST(node, find)
{
	auto f{fdt::load("path.dtb")};
	const auto &fc{f};

	/* find(node &) */
	EXPECT_EQ(as_u32(as_property(*find(f.root(), "l1@1/l2@1/l1#1-l2#1-prop"))), 11);
	EXPECT_EQ(as_u32(as_property(*find(f.root(), "l1@2/l2@1/l1#2-l2#1-prop"))), 21);
	EXPECT_EQ(is_node(*find(f.root(), "l1@2/l2@1")), true);
	EXPECT_EQ(is_node(*find(f.root(), "l1@1/l2")), true);
	EXPECT_EQ(as_u32(as_property(*find(f.root(), "l1@1/l2/l1#1-l2#1-prop"))), 11);
	EXPECT_THROW(find(f.root(), "l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(find(f.root(), "/x"), std::invalid_argument);
	EXPECT_EQ(find(f.root(), "x").has_value(), false);

	/* find(const node &) */
	EXPECT_EQ(as_u32(as_property(*find(fc.root(), "l1@1/l2@1/l1#1-l2#1-prop"))), 11);
	EXPECT_EQ(as_u32(as_property(*find(fc.root(), "l1@2/l2@1/l1#2-l2#1-prop"))), 21);
	EXPECT_EQ(is_node(*find(fc.root(), "l1@2/l2@1")), true);
	EXPECT_EQ(is_node(*find(fc.root(), "l1@1/l2")), true);
	EXPECT_EQ(as_u32(as_property(*find(fc.root(), "l1@1/l2/l1#1-l2#1-prop"))), 11);
	EXPECT_THROW(find(fc.root(), "l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(find(fc.root(), "/x"), std::invalid_argument);
	EXPECT_EQ(find(fc.root(), "x").has_value(), false);
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
	EXPECT_EQ(get_node(f, "/l1@2/l2@1").name(), "l2@1");
	EXPECT_THROW(get_node(f, "x"), std::invalid_argument);
	EXPECT_THROW(get_node(f, "/x"), std::bad_optional_access);
	EXPECT_THROW(get_node(f, "/l1@2/l2@1/l1#2-l2#1-prop"), std::bad_cast);

	/* get_node(const fdt &) */
	EXPECT_EQ(get_node(cf, "/l1@2/l2@1").name(), "l2@1");
	EXPECT_THROW(get_node(cf, "x"), std::invalid_argument);
	EXPECT_THROW(get_node(cf, "/x"), std::bad_optional_access);
	EXPECT_THROW(get_node(cf, "/l1@2/l2@1/l1#2-l2#1-prop"), std::bad_cast);
}

TEST(fdt, get_property)
{
	auto f{fdt::load("path.dtb")};
	const auto &cf{f};

	/* get_property(fdt &) */
	EXPECT_EQ(get_property(f, "/l1@1/l2@1/l1#1-l2#1-prop").name(), "l1#1-l2#1-prop");
	EXPECT_THROW(get_property(f, "x"), std::invalid_argument);
	EXPECT_THROW(get_property(f, "/x"), std::bad_optional_access);
	EXPECT_THROW(get_property(f, "/l1@1"), std::bad_cast);

	/* get_property(const fdt &) */
	EXPECT_EQ(get_property(cf, "/l1@1/l2@1/l1#1-l2#1-prop").name(), "l1#1-l2#1-prop");
	EXPECT_THROW(get_property(cf, "x"), std::invalid_argument);
	EXPECT_THROW(get_property(cf, "/x"), std::bad_optional_access);
	EXPECT_THROW(get_property(cf, "/l1@1"), std::bad_cast);
}