#include <gtest/gtest.h>

#include "../libfit++.h"

#include <fstream>

namespace {

std::optional<std::vector<std::byte>>
get_key(std::string_view key_name_hint)
{
	std::filesystem::path p{key_name_hint};
	p += "_pub.der";

	std::vector<std::byte> d(std::filesystem::file_size(p));
	std::ifstream f(p, std::ios::binary);
	f.read(reinterpret_cast<char *>(data(d)), size(d));
	return d;
}

std::vector<std::byte>
read_file(const std::filesystem::path &p, size_t off, size_t len)
{
	std::vector<std::byte> d(len);
	std::ifstream f(p, std::ios::binary);
	f.seekg(off);
	f.read(reinterpret_cast<char *>(data(d)), size(d));
	if (static_cast<size_t>(f.gcount()) != len)
		throw std::runtime_error("bad offset/length!");
	return d;
}

}

TEST(fit, verify_image_hashes)
{
	const auto &f{fdt::load("verify.fit")};
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-1")));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-2")));

	const auto &[fo, fod]{fdt::load_keep("verify-offset.fit")};
	EXPECT_TRUE(fit::verify_image_hashes(get_node(fo, "/images/test-1"), fod,
	    [](auto off, auto len, const auto &process) {
		process(read_file("verify-offset.fit", off, len));
	}));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(fo, "/images/test-2"), fod,
	    [](auto off, auto len, const auto &process) {
		process(read_file("verify-offset.fit", off, len));
	}));

	const auto &[fp, fpd]{fdt::load_keep("verify-position.fit")};
	EXPECT_TRUE(fit::verify_image_hashes(get_node(fp, "/images/test-1"), fpd,
	    [](auto off, auto len, const auto &process) {
		process(read_file("verify-position.fit", off, len));
	}));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(fp, "/images/test-2"), fpd,
	    [](auto off, auto len, const auto &process) {
		process(read_file("verify-position.fit", off, len));
	}));
}

TEST(fit, verify_image_signatures)
{
	const auto &f{fdt::load("verify.fit")};
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-1"),
						 get_key));

	const auto &[fo, fod]{fdt::load_keep("verify-offset.fit")};
	EXPECT_TRUE(fit::verify_image_signatures(get_node(fo, "/images/test-1"),
						 get_key, fod,
	    [](auto off, auto len, const auto &process) {
		process(read_file("verify-offset.fit", off, len));
	}));

	const auto &[fp, fpd]{fdt::load_keep("verify-position.fit")};
	EXPECT_TRUE(fit::verify_image_signatures(get_node(fp, "/images/test-1"),
						 get_key, fpd,
	    [](auto off, auto len, const auto &process) {
		process(read_file("verify-position.fit", off, len));
	}));
}

TEST(fit, verify_config_signatures)
{
	const auto &[f, fd]{fdt::load_keep("verify.fit")};
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-1"), get_key, fd));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-2"), get_key, fd));

	const auto &[fo, fod]{fdt::load_keep("verify-offset.fit")};
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(fo, "/configurations/config-1"), get_key, fod,
	    [](auto off, auto len, const auto &process) {
		process(read_file("verify-offset.fit", off, len));
	}));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(fo, "/configurations/config-2"), get_key, fod,
	    [](auto off, auto len, const auto &process) {
		process(read_file("verify-offset.fit", off, len));
	}));

	const auto &[fp, fpd]{fdt::load_keep("verify-position.fit")};
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(fp, "/configurations/config-1"), get_key, fpd,
	    [](auto off, auto len, const auto &process) {
		process(read_file("verify-position.fit", off, len));
	}));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(fp, "/configurations/config-2"), get_key, fpd,
	    [](auto off, auto len, const auto &process) {
		process(read_file("verify-position.fit", off, len));
	}));
}
