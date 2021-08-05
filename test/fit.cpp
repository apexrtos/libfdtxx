#include <gtest/gtest.h>

#include "../libfit++.h"

#include <fstream>
#include <random>

namespace {

std::vector<std::byte>
read_file(const std::filesystem::path &p)
{
	std::vector<std::byte> d(std::filesystem::file_size(p));
	std::ifstream f{p, std::ios::binary};
	f.read(reinterpret_cast<char *>(data(d)), size(d));
	return d;
}

void
get_symmetric_key(std::string_view key_name, std::string_view iv_name,
		  const fit::key_iv_fn &load)
{
	std::filesystem::path k_p{key_name};
	std::filesystem::path iv_p{iv_name};
	k_p += ".bin";
	iv_p += ".bin";
	load(read_file(k_p), read_file(iv_p));
}

void
get_public_key(std::string_view key_name, const fit::process_fn &load)
{
	std::filesystem::path p{key_name};
	p += "_pub.der";
	load(read_file(p));
}

void
read_file(const std::filesystem::path &p, size_t off, size_t len,
	  const fit::process_fn &process)
{
	std::vector<std::byte> d{len};
	std::ifstream f{p, std::ios::binary};
	f.seekg(off);
	f.read(reinterpret_cast<char *>(data(d)), size(d));
	if (static_cast<size_t>(f.gcount()) != len)
		throw std::runtime_error("bad offset/length!");
	/* process 1 byte at a time to excercise libfit++ */
	for (auto b : d)
		process({&b, 1});
}

void
read_file_random(const std::filesystem::path &p, size_t off, size_t len,
	  const fit::process_fn &process)
{
	std::vector<std::byte> d{len};
	std::ifstream f{p, std::ios::binary};
	f.seekg(off);
	f.read(reinterpret_cast<char *>(data(d)), size(d));
	if (static_cast<size_t>(f.gcount()) != len)
		throw std::runtime_error("bad offset/length!");
	/* process random chunks to excercise libfit++ */
	off = 0;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<size_t> uniform_dist(1, 128);
	while (off < len) {
		auto sz{std::min(uniform_dist(gen), len - off)};
		process({&d[off], sz});
		off += sz;
	}
}

std::span<const std::byte>
as_bytes(const char *s)
{
	return {reinterpret_cast<const std::byte *>(s), strlen(s) + 1};
}

#ifdef __cpp_lib_ranges
using std::ranges::equal;
#else
bool equal(const auto &l, const auto &r)
{
	if (size(l) != size(r))
		return false;
	return std::equal(begin(l), end(l), begin(r));
}
#endif

}

TEST(fit, verify_image_hashes_inline)
{
	const auto &f{fdt::load("verify.fit")};
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-1")));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-2")));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-aes128")));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-aes192")));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-aes256")));
}

TEST(fit, verify_image_hashes_offset)
{
	const auto &[f, fd]{fdt::load_keep("verify-offset.fit")};
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-1"), fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-2"), fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-aes128"), fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-aes192"), fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-aes256"), fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
}

TEST(fit, verify_image_hashes_position)
{
	const auto &[f, fd]{fdt::load_keep("verify-position.fit")};
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-1"), fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-2"), fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-aes128"), fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-aes192"), fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_hashes(get_node(f, "/images/test-aes256"), fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
}

TEST(fit, verify_image_signatures_inline)
{
	const auto &f{fdt::load("verify.fit")};
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-1"),
						 get_public_key));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes128"),
						 get_public_key));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes192"),
						 get_public_key));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes256"),
						 get_public_key));
}

TEST(fit, verify_image_signatures_offset)
{
	const auto &[f, fd]{fdt::load_keep("verify-offset.fit")};
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-1"),
						 get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes128"),
						 get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes192"),
						 get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes256"),
						 get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
}

TEST(fit, verify_image_signatures_position)
{
	const auto &[f, fd]{fdt::load_keep("verify-position.fit")};
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-1"),
						 get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes128"),
						 get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes192"),
						 get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes256"),
						 get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
}

TEST(fit, verify_config_signatures_inline)
{
	const auto &[f, fd]{fdt::load_keep("verify.fit")};
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-1"), get_public_key, fd));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-2"), get_public_key, fd));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-3"), get_public_key, fd));
}

TEST(fit, verify_config_signatures_offset)
{
	const auto &[f, fd]{fdt::load_keep("verify-offset.fit")};
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-1"), get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-2"), get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-3"), get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
}

TEST(fit, verify_config_signatures_position)
{
	const auto &[f, fd]{fdt::load_keep("verify-position.fit")};
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-1"), get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-2"), get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-3"), get_public_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
}

TEST(fit, image_data_size_inline)
{
	const auto &f{fdt::load("verify.fit")};
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-1")), 12u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-2")), 12u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-aes128")), 4096u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-aes192")), 17u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-aes256")), 17u);
}

TEST(fit, image_data_size_offset)
{
	const auto &f{fdt::load("verify-offset.fit")};
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-1")), 12u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-2")), 12u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-aes128")), 4096u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-aes192")), 17u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-aes256")), 17u);
}

TEST(fit, image_data_size_position)
{
	const auto &f{fdt::load("verify-position.fit")};
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-1")), 12u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-2")), 12u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-aes128")), 4096u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-aes192")), 17u);
	EXPECT_EQ(fit::image_data_size(get_node(f, "/images/test-aes256")), 17u);
}

TEST(fit, image_data_inline)
{
	std::vector<std::byte> data;

	const auto &f{fdt::load("verify.fit")};
	data.clear();
	fit::image_data(get_node(f, "/images/test-1"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	});
	EXPECT_TRUE(equal(data, as_bytes("test-1 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-2"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	});
	EXPECT_TRUE(equal(data, as_bytes("test-2 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes128"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_symmetric_key);
	EXPECT_TRUE(equal(data, read_file("4k_random")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes192"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_symmetric_key);
	EXPECT_TRUE(equal(data, as_bytes("test-aes192 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes256"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_symmetric_key);
	EXPECT_TRUE(equal(data, as_bytes("test-aes256 data")));
}

TEST(fit, image_data_offset)
{
	std::vector<std::byte> data;

	const auto &[f, fd]{fdt::load_keep("verify-offset.fit")};
	data.clear();
	fit::image_data(get_node(f, "/images/test-1"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-1 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-2"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-2 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes128"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_symmetric_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, read_file("4k_random")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes192"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_symmetric_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-aes192 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes256"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_symmetric_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-aes256 data")));
}

TEST(fit, image_data_position)
{
	std::vector<std::byte> data;

	const auto &[f, fd]{fdt::load_keep("verify-position.fit")};
	data.clear();
	fit::image_data(get_node(f, "/images/test-1"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-1 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-2"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-2 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes128"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_symmetric_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, read_file("4k_random")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes192"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_symmetric_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-aes192 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes256"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_symmetric_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-aes256 data")));
}

TEST(fit, decrypt_fuzz)
{
	std::vector<std::byte> data;

	const auto &[f, fd]{fdt::load_keep("verify-position.fit")};
	for (size_t i = 0; i < 10000; ++i) {
		data.clear();
		fit::image_data(get_node(f, "/images/test-aes128"),
		[&](std::span<const std::byte> d) {
			data.insert(end(data), begin(d), end(d));
		}, get_symmetric_key, fd,
		[](auto off, auto len, const auto &process) {
			read_file_random("verify-position.fit", off, len, process);
		});
		EXPECT_TRUE(equal(data, read_file("4k_random")));
	}
}
