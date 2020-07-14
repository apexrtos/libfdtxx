#include <gtest/gtest.h>

#include "../libfdt++.h"

namespace {

constexpr std::byte
operator""_b(unsigned long long v)
{
	return static_cast<std::byte>(v);
}

}

TEST(piece, parent)
{
	auto f{fdt::load("path.dtb")};
	const auto &fc{f};

	/* piece::parent */
	EXPECT_FALSE(parent(root(f)).has_value());
	EXPECT_EQ(&parent(get_node(f, "/l1@1"))->get(), &root(f));
	EXPECT_EQ(&parent(get_node(f, "/l1@1/l2@1"))->get(), &get_node(f, "/l1@1"));
	EXPECT_EQ(&parent(get_property(f, "/l1@1/l2@1/l1#1-l2#1-prop"))->get(), &get_node(f, "/l1@1/l2@1"));

	/* piece::parent const */
	EXPECT_FALSE(parent(root(fc)).has_value());
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
	EXPECT_TRUE(is_property(property));
	EXPECT_FALSE(is_node(property));
	EXPECT_FALSE(is_property(node));
	EXPECT_TRUE(is_node(node));

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

TEST(property, set_uint32_t)
{
	fdt::fdt f;
	auto &p{add_property(root(f), "test")};

	const uint32_t val{0xdeadbeef};
	set(p, val);

	EXPECT_EQ(as<uint32_t>(p), val);
}

TEST(property, set_uint64_t)
{
	fdt::fdt f;
	auto &p{add_property(root(f), "test")};

	const uint64_t val{0xdeadbeefcafef00d};
	set(p, val);

	EXPECT_EQ(as<uint64_t>(p), val);
}

TEST(property, set_string)
{
	fdt::fdt f;
	auto &p{add_property(root(f), "test")};

	const auto val{"hello world!"};
	set(p, val);

	EXPECT_EQ(as_string(p), val);
}

TEST(property, set_stringlist)
{
	fdt::fdt f;
	auto &p{add_property(root(f), "test")};

	const std::vector<std::string_view> val{"hello", "world!"};
	set(p, val);

	EXPECT_EQ(as_stringlist(p), val);
}

TEST(property, is_empty)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_TRUE(is_empty(get_property(f, "/property-empty")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-u32")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-u64")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-string")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-1")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-2")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-3")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-4")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-5")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-6")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-7")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-8")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-9")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-10")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-11")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-12")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-13")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-14")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-15")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-16")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-17")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-18")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-19")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-20")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-21")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-22")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-23")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-24")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-25")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-26")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-27")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-28")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-29")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-30")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-31")));
	EXPECT_FALSE(is_empty(get_property(f, "/property-32")));
}

TEST(property, is_string)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is_string(get_property(f, "/property-empty")));
	EXPECT_FALSE(is_string(get_property(f, "/property-u32")));
	EXPECT_FALSE(is_string(get_property(f, "/property-u64")));
	EXPECT_TRUE(is_string(get_property(f, "/property-string")));
	EXPECT_FALSE(is_string(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is_string(get_property(f, "/property-1")));
	EXPECT_FALSE(is_string(get_property(f, "/property-2")));
	EXPECT_FALSE(is_string(get_property(f, "/property-3")));
	EXPECT_FALSE(is_string(get_property(f, "/property-4")));
	EXPECT_FALSE(is_string(get_property(f, "/property-5")));
	EXPECT_FALSE(is_string(get_property(f, "/property-6")));
	EXPECT_FALSE(is_string(get_property(f, "/property-7")));
	EXPECT_FALSE(is_string(get_property(f, "/property-8")));
	EXPECT_FALSE(is_string(get_property(f, "/property-9")));
	EXPECT_FALSE(is_string(get_property(f, "/property-10")));
	EXPECT_FALSE(is_string(get_property(f, "/property-11")));
	EXPECT_FALSE(is_string(get_property(f, "/property-12")));
	EXPECT_FALSE(is_string(get_property(f, "/property-13")));
	EXPECT_FALSE(is_string(get_property(f, "/property-14")));
	EXPECT_FALSE(is_string(get_property(f, "/property-15")));
	EXPECT_FALSE(is_string(get_property(f, "/property-16")));
	EXPECT_FALSE(is_string(get_property(f, "/property-17")));
	EXPECT_FALSE(is_string(get_property(f, "/property-18")));
	EXPECT_FALSE(is_string(get_property(f, "/property-19")));
	EXPECT_FALSE(is_string(get_property(f, "/property-20")));
	EXPECT_FALSE(is_string(get_property(f, "/property-21")));
	EXPECT_FALSE(is_string(get_property(f, "/property-22")));
	EXPECT_FALSE(is_string(get_property(f, "/property-23")));
	EXPECT_FALSE(is_string(get_property(f, "/property-24")));
	EXPECT_FALSE(is_string(get_property(f, "/property-25")));
	EXPECT_FALSE(is_string(get_property(f, "/property-26")));
	EXPECT_FALSE(is_string(get_property(f, "/property-27")));
	EXPECT_FALSE(is_string(get_property(f, "/property-28")));
	EXPECT_FALSE(is_string(get_property(f, "/property-29")));
	EXPECT_FALSE(is_string(get_property(f, "/property-30")));
	EXPECT_FALSE(is_string(get_property(f, "/property-31")));
	EXPECT_FALSE(is_string(get_property(f, "/property-32")));
}

TEST(property, is_stringlist)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-empty")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-u32")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-u64")));
	EXPECT_TRUE(is_stringlist(get_property(f, "/property-string")));
	EXPECT_TRUE(is_stringlist(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-1")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-2")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-3")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-4")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-5")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-6")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-7")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-8")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-9")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-10")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-11")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-12")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-13")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-14")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-15")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-16")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-17")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-18")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-19")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-20")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-21")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-22")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-23")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-24")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-25")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-26")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-27")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-28")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-29")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-30")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-31")));
	EXPECT_FALSE(is_stringlist(get_property(f, "/property-32")));
}

TEST(property, is_u32)
{
	using T = uint32_t;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is<T>(get_property(f, "/property-empty")));
	EXPECT_TRUE(is<T>(get_property(f, "/property-u32")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-u64")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-string")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-1")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-2")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-3")));
	EXPECT_TRUE(is<T>(get_property(f, "/property-4")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-5")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-6")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-7")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-8")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-9")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-10")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-11")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-12")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-13")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-14")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-15")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-16")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-17")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-18")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-19")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-20")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-21")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-22")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-23")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-24")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-25")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-26")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-27")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-28")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-29")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-30")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-31")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-32")));
}

TEST(property, is_u64)
{
	using T = uint64_t;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is<T>(get_property(f, "/property-empty")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-u32")));
	EXPECT_TRUE(is<T>(get_property(f, "/property-u64")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-string")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-1")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-2")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-3")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-4")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-5")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-6")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-7")));
	EXPECT_TRUE(is<T>(get_property(f, "/property-8")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-9")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-10")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-11")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-12")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-13")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-14")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-15")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-16")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-17")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-18")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-19")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-20")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-21")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-22")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-23")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-24")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-25")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-26")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-27")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-28")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-29")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-30")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-31")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-32")));
}

TEST(property, is_pair_u32u64)
{
	using T = std::pair<uint32_t, uint64_t>;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is<T>(get_property(f, "/property-empty")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-u32")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-u64")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-string")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-1")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-2")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-3")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-4")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-5")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-6")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-7")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-8")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-9")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-10")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-11")));
	EXPECT_TRUE(is<T>(get_property(f, "/property-12")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-13")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-14")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-15")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-16")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-17")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-18")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-19")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-20")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-21")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-22")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-23")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-24")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-25")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-26")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-27")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-28")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-29")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-30")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-31")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-32")));
}

TEST(property, is_tuple_u8u16u32u64)
{
	using T = std::tuple<uint8_t, uint16_t, uint32_t, uint64_t>;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is<T>(get_property(f, "/property-empty")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-u32")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-u64")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-string")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-1")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-2")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-3")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-4")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-5")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-6")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-7")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-8")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-9")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-10")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-11")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-12")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-13")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-14")));
	EXPECT_TRUE(is<T>(get_property(f, "/property-15")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-16")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-17")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-18")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-19")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-20")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-21")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-22")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-23")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-24")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-25")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-26")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-27")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-28")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-29")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-30")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-31")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-32")));
}

TEST(property, is_array_tuple_u32u64_2)
{
	using U = std::tuple<uint32_t, uint64_t>;
	using T = std::array<U, 2>;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is<T>(get_property(f, "/property-empty")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-u32")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-u64")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-string")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-1")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-2")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-3")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-4")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-5")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-6")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-7")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-8")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-9")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-10")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-11")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-12")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-13")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-14")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-15")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-16")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-17")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-18")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-19")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-20")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-21")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-22")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-23")));
	EXPECT_TRUE(is<T>(get_property(f, "/property-24")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-25")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-26")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-27")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-28")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-29")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-30")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-31")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-32")));
}

TEST(property, is_array_u32_3)
{
	using T = std::array<uint32_t, 3>;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is<T>(get_property(f, "/property-empty")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-u32")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-u64")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-string")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-1")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-2")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-3")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-4")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-5")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-6")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-7")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-8")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-9")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-10")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-11")));
	EXPECT_TRUE(is<T>(get_property(f, "/property-12")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-13")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-14")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-15")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-16")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-17")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-18")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-19")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-20")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-21")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-22")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-23")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-24")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-25")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-26")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-27")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-28")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-29")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-30")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-31")));
	EXPECT_FALSE(is<T>(get_property(f, "/property-32")));
}

TEST(property, is_u32_array)
{
	using T = uint32_t;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-empty")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-u32")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-u64")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-string")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-1")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-2")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-3")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-4")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-5")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-6")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-7")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-8")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-9")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-10")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-11")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-12")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-13")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-14")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-15")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-16")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-17")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-18")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-19")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-20")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-21")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-22")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-23")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-24")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-25")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-26")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-27")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-28")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-29")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-30")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-31")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-32")));
}

TEST(property, is_u64_array)
{
	using T = uint64_t;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-empty")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-u32")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-u64")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-string")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-1")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-2")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-3")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-4")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-5")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-6")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-7")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-8")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-9")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-10")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-11")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-12")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-13")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-14")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-15")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-16")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-17")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-18")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-19")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-20")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-21")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-22")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-23")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-24")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-25")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-26")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-27")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-28")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-29")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-30")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-31")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-32")));
}

TEST(property, is_pair_u32u64_array)
{
	using T = std::pair<uint32_t, uint64_t>;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-empty")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-u32")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-u64")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-string")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-1")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-2")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-3")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-4")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-5")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-6")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-7")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-8")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-9")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-10")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-11")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-12")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-13")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-14")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-15")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-16")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-17")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-18")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-19")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-20")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-21")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-22")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-23")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-24")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-25")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-26")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-27")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-28")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-29")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-30")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-31")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-32")));
}

TEST(property, is_tuple_u8u16u32u64_array)
{
	using T = std::tuple<uint8_t, uint16_t, uint32_t, uint64_t>;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-empty")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-u32")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-u64")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-string")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-1")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-2")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-3")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-4")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-5")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-6")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-7")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-8")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-9")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-10")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-11")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-12")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-13")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-14")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-15")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-16")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-17")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-18")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-19")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-20")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-21")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-22")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-23")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-24")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-25")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-26")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-27")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-28")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-29")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-30")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-31")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-32")));
}

TEST(property, is_array_u32_3_array)
{
	using T = std::array<uint32_t, 3>;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-empty")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-u32")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-u64")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-string")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-stringlist")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-1")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-2")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-3")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-4")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-5")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-6")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-7")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-8")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-9")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-10")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-11")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-12")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-13")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-14")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-15")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-16")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-17")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-18")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-19")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-20")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-21")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-22")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-23")));
	EXPECT_TRUE(is_array<T>(get_property(f, "/property-24")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-25")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-26")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-27")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-28")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-29")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-30")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-31")));
	EXPECT_FALSE(is_array<T>(get_property(f, "/property-32")));
}

TEST(property, as_u32)
{
	using T = uint32_t;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as<T>(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_EQ(as<T>(get_property(f, "/property-u32")), 32);
	EXPECT_THROW(as<T>(get_property(f, "/property-u64")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_EQ(as<T>(get_property(f, "/property-4")), 0x01020304);
	EXPECT_THROW(as<T>(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-7")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-8")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-11")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-12")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-15")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-16")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-23")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-24")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-31")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-32")), std::invalid_argument);
}

TEST(property, as_u64)
{
	using T = uint64_t;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as<T>(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_EQ(as<T>(get_property(f, "/property-u64")), 64);
	EXPECT_THROW(as<T>(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-4")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-7")), std::invalid_argument);
	EXPECT_EQ(as<T>(get_property(f, "/property-8")), 0x0102030405060708);
	EXPECT_THROW(as<T>(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-11")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-12")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-15")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-16")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-23")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-24")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-31")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-32")), std::invalid_argument);
}

TEST(property, as_string)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as_string(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-u64")), std::invalid_argument);
	EXPECT_EQ(as_string(get_property(f, "/property-string")), "hello world!");
	EXPECT_THROW(as_string(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-4")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-7")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-8")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-11")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-12")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-15")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-16")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-23")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-24")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-31")), std::invalid_argument);
	EXPECT_THROW(as_string(get_property(f, "/property-32")), std::invalid_argument);
}

TEST(property, as_stringlist)
{
	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as_stringlist(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-u64")), std::invalid_argument);
	const std::vector<std::string_view> val_string{"hello world!"};
	EXPECT_EQ(as_stringlist(get_property(f, "/property-string")), val_string);
	const std::vector<std::string_view> val_stringlist{"hello", "world!"};
	EXPECT_EQ(as_stringlist(get_property(f, "/property-stringlist")), val_stringlist);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-4")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-7")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-8")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-11")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-12")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-15")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-16")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-23")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-24")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-31")), std::invalid_argument);
	EXPECT_THROW(as_stringlist(get_property(f, "/property-32")), std::invalid_argument);
}

TEST(property, as_pair_u32u64)
{
	using T = std::pair<uint32_t, uint64_t>;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as<T>(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-u64")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-4")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-7")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-8")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-11")), std::invalid_argument);
	EXPECT_EQ(as<T>(get_property(f, "/property-12")), T(0x01020304, 0x05060708090a0b0c));
	EXPECT_THROW(as<T>(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-15")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-16")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-23")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-24")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-31")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-32")), std::invalid_argument);
}

TEST(property, as_tuple_u8u16u32u64)
{
	using T = std::tuple<uint8_t, uint16_t, uint32_t, uint64_t>;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as<T>(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-u64")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-4")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-7")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-8")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-11")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-12")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_EQ(as<T>(get_property(f, "/property-15")), T(0x01, 0x0203, 0x04050607, 0x08090a0b0c0d0e0f));
	EXPECT_THROW(as<T>(get_property(f, "/property-16")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-23")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-24")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-31")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-32")), std::invalid_argument);
}

TEST(property, as_array_u32_3)
{
	using T = std::array<uint32_t, 3>;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as<T>(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-u64")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-4")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-7")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-8")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-11")), std::invalid_argument);
	const T val{0x01020304, 0x05060708, 0x090a0b0c};
	EXPECT_EQ(as<T>(get_property(f, "/property-12")), val);
	EXPECT_THROW(as<T>(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-15")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-16")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-23")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-24")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-31")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-32")), std::invalid_argument);
}

TEST(property, as_array_tuple_u32u64_2)
{
	using U = std::tuple<uint32_t, uint64_t>;
	using T = std::array<U, 2>;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as<T>(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-u64")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-4")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-7")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-8")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-11")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-12")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-15")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-16")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-23")), std::invalid_argument);
	const T val{U{0x01020304, 0x05060708090a0b0c}, U{0x0d0e0f10, 0x1112131415161718}};
	EXPECT_EQ(as<T>(get_property(f, "/property-24")), val);
	EXPECT_THROW(as<T>(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-31")), std::invalid_argument);
	EXPECT_THROW(as<T>(get_property(f, "/property-32")), std::invalid_argument);
}

TEST(property, as_u32_array)
{
	using T = uint32_t;
	using std::ranges::equal;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as_array<T>(get_property(f, "/property-empty")), std::invalid_argument);
	const std::array<T, 1> val32{32};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-u32")), val32));
	const std::array<T, 2> val64{0, 64};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-u64")), val64));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-3")), std::invalid_argument);
	const std::array<T, 1> val1{0x01020304};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-4")), val1));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-7")), std::invalid_argument);
	const std::array<T, 2> val2{0x01020304, 0x05060708};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-8")), val2));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-11")), std::invalid_argument);
	const std::array<T, 3> val3{0x01020304, 0x05060708, 0x090a0b0c};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-12")), val3));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-15")), std::invalid_argument);
	const std::array<T, 4> val4{0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-16")), val4));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-19")), std::invalid_argument);
	const std::array<T, 5> val5{0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-20")), val5));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-23")), std::invalid_argument);
	const std::array<T, 6> val6{0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314, 0x15161718};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-24")), val6));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-27")), std::invalid_argument);
	const std::array<T, 7> val7{0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314, 0x15161718, 0x191a1b1c};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-28")), val7));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-31")), std::invalid_argument);
	const std::array<T, 8> val8{0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314, 0x15161718, 0x191a1b1c, 0x1d1e1f20};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-32")), val8));
}

TEST(property, as_u64_array)
{
	using T = uint64_t;
	using std::ranges::equal;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as_array<T>(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-u32")), std::invalid_argument);
	const std::array<T, 1> val64{64};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-u64")), val64));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-4")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-7")), std::invalid_argument);
	const std::array<T, 1> val1{0x0102030405060708};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-8")), val1));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-11")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-12")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-15")), std::invalid_argument);
	const std::array<T, 2> val2{0x0102030405060708, 0x090a0b0c0d0e0f10};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-16")), val2));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-23")), std::invalid_argument);
	const std::array<T, 3> val3{0x0102030405060708, 0x090a0b0c0d0e0f10, 0x1112131415161718};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-24")), val3));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-31")), std::invalid_argument);
	const std::array<T, 4> val4{0x0102030405060708, 0x090a0b0c0d0e0f10, 0x1112131415161718, 0x191a1b1c1d1e1f20};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-32")), val4));
}

TEST(property, as_pair_u32u64_array)
{
	using T = std::pair<uint32_t, uint64_t>;
	using std::ranges::equal;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as_array<T>(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-u64")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-4")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-7")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-8")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-11")), std::invalid_argument);
	const std::array<T, 1> val1{T{0x01020304, 0x05060708090a0b0c}};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-12")), val1));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-15")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-16")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-23")), std::invalid_argument);
	const std::array<T, 2> val2{T{0x01020304, 0x05060708090a0b0c}, T{0x0d0e0f10, 0x1112131415161718}};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-24")), val2));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-31")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-32")), std::invalid_argument);
}

TEST(property, as_tuple_u8u16u32u64_array)
{
	using T = std::tuple<uint8_t, uint16_t, uint32_t, uint64_t>;
	using std::ranges::equal;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as_array<T>(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-u64")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-4")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-7")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-8")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-11")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-12")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-14")), std::invalid_argument);
	const std::array<T, 1> val1{T{0x01, 0x0203, 0x04050607, 0x08090a0b0c0d0e0f}};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-15")), val1));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-16")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-23")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-24")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-29")), std::invalid_argument);
	const std::array<T, 2> val2{T{0x01, 0x0203, 0x04050607, 0x08090a0b0c0d0e0f}, T{0x10, 0x1112, 0x13141516, 0x1718191a1b1c1d1e}};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-30")), val2));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-31")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-32")), std::invalid_argument);
}

TEST(property, as_array_u32_3_array)
{
	using T = std::array<uint32_t, 3>;
	using std::ranges::equal;

	const auto &f{fdt::load("properties.dtb")};
	EXPECT_THROW(as_array<T>(get_property(f, "/property-empty")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-u32")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-u64")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-string")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-stringlist")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-1")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-2")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-3")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-4")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-5")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-6")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-7")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-8")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-9")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-10")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-11")), std::invalid_argument);
	const std::array<T, 1> val1{T{0x01020304, 0x05060708, 0x090a0b0c}};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-12")), val1));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-13")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-14")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-15")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-16")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-17")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-18")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-19")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-20")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-21")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-22")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-23")), std::invalid_argument);
	const std::array<T, 2> val2{T{0x01020304, 0x05060708, 0x090a0b0c}, T{0x0d0e0f10, 0x11121314, 0x15161718}};
	EXPECT_TRUE(equal(as_array<T>(get_property(f, "/property-24")), val2));
	EXPECT_THROW(as_array<T>(get_property(f, "/property-25")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-26")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-27")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-28")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-29")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-30")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-31")), std::invalid_argument);
	EXPECT_THROW(as_array<T>(get_property(f, "/property-32")), std::invalid_argument);
}

TEST(property, as_bytes)
{
	using std::ranges::equal;

	const auto &f{fdt::load("properties.dtb")};
	const std::span<std::byte> val_empty;
	EXPECT_TRUE(equal(as_bytes(get_property(f, "/property-empty")), val_empty));
	const std::array<std::byte, 4> val_u32{0_b, 0_b, 0_b, 32_b};
	EXPECT_TRUE(equal(as_bytes(get_property(f, "/property-u32")), val_u32));
	const std::array<std::byte, 8> val_u64{0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 64_b};
	EXPECT_TRUE(equal(as_bytes(get_property(f, "/property-u64")), val_u64));
	std::span<const std::byte> val_string{
		reinterpret_cast<const std::byte *>("hello world!"), 13};
	EXPECT_TRUE(equal(as_bytes(get_property(f, "/property-string")), val_string));
	std::span<const std::byte> val_stringlist{
		reinterpret_cast<const std::byte *>("hello\0world!"), 13};
	EXPECT_TRUE(equal(as_bytes(get_property(f, "/property-stringlist")), val_stringlist));
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
	EXPECT_EQ(as<uint32_t>(get_property(f, "/u32")), 0xdeadbeef);
	EXPECT_EQ(as<uint64_t>(get_property(f, "/n1/u64")), 0xdeadbeefcafef00d);
}

TEST(node, contains)
{
	auto f{fdt::load("path.dtb")};
	EXPECT_TRUE(contains(root(f), "l1@1/l2@1/l1#1-l2#1-prop"));
	EXPECT_TRUE(contains(root(f), "l1@2/l2@1"));
	EXPECT_TRUE(contains(root(f), "l1@1/l2"));
	EXPECT_TRUE(contains(root(f), "l1@1/l2/l1#1-l2#1-prop"));
	EXPECT_THROW(contains(root(f), "l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(contains(root(f), "/x"), std::invalid_argument);
	EXPECT_FALSE(contains(root(f), "x"));
}

TEST(node, find)
{
	auto f{fdt::load("path.dtb")};
	const auto &fc{f};

	/* find(node &) */
	EXPECT_EQ(as<uint32_t>(as_property(*find(root(f), "l1@1/l2@1/l1#1-l2#1-prop"))), 11);
	EXPECT_EQ(as<uint32_t>(as_property(*find(root(f), "l1@2/l2@1/l1#2-l2#1-prop"))), 21);
	EXPECT_TRUE(is_node(*find(root(f), "l1@2/l2@1")));
	EXPECT_TRUE(is_node(*find(root(f), "l1@1/l2")));
	EXPECT_EQ(as<uint32_t>(as_property(*find(root(f), "l1@1/l2/l1#1-l2#1-prop"))), 11);
	EXPECT_THROW(find(root(f), "l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(find(root(f), "/x"), std::invalid_argument);
	EXPECT_FALSE(find(root(f), "x").has_value());

	/* find(const node &) */
	EXPECT_EQ(as<uint32_t>(as_property(*find(root(fc), "l1@1/l2@1/l1#1-l2#1-prop"))), 11);
	EXPECT_EQ(as<uint32_t>(as_property(*find(root(fc), "l1@2/l2@1/l1#2-l2#1-prop"))), 21);
	EXPECT_TRUE(is_node(*find(root(fc), "l1@2/l2@1")));
	EXPECT_TRUE(is_node(*find(root(fc), "l1@1/l2")));
	EXPECT_EQ(as<uint32_t>(as_property(*find(root(fc), "l1@1/l2/l1#1-l2#1-prop"))), 11);
	EXPECT_THROW(find(root(fc), "l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(find(root(fc), "/x"), std::invalid_argument);
	EXPECT_FALSE(find(root(fc), "x").has_value());
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
	EXPECT_EQ(as<uint32_t>(as_property(*find(f, "/l1@1/l2@1/l1#1-l2#1-prop"))), 11);
	EXPECT_EQ(as<uint32_t>(as_property(*find(f, "/l1@2/l2@1/l1#2-l2#1-prop"))), 21);
	EXPECT_TRUE(is_node(*find(f, "/l1@2/l2@1")));
	EXPECT_TRUE(is_node(*find(f, "/l1@1/l2")));
	EXPECT_EQ(as<uint32_t>(as_property(*find(f, "/l1@1/l2/l1#1-l2#1-prop"))), 11);
	EXPECT_THROW(find(f, "/l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(find(f, "x"), std::invalid_argument);
	EXPECT_FALSE(find(f, "/x").has_value());

	/* find(fdt node &) */
	EXPECT_EQ(as<uint32_t>(as_property(*find(fc, "/l1@1/l2@1/l1#1-l2#1-prop"))), 11);
	EXPECT_EQ(as<uint32_t>(as_property(*find(fc, "/l1@2/l2@1/l1#2-l2#1-prop"))), 21);
	EXPECT_TRUE(is_node(*find(fc, "/l1@2/l2@1")));
	EXPECT_TRUE(is_node(*find(fc, "/l1@1/l2")));
	EXPECT_EQ(as<uint32_t>(as_property(*find(fc, "/l1@1/l2/l1#1-l2#1-prop"))), 11);
	EXPECT_THROW(find(fc, "/l1@1//l2"), std::invalid_argument);
	EXPECT_THROW(find(fc, "x"), std::invalid_argument);
	EXPECT_FALSE(find(fc, "/x").has_value());
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
