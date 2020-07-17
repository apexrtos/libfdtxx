#include <gtest/gtest.h>

#include "../libfit++.h"

#include <fstream>
#include <random>

namespace {

std::optional<std::vector<std::byte>>
get_key(fit::key_type type, std::string_view key_name_hint)
{
	std::filesystem::path p{key_name_hint};
	switch (type) {
	case fit::key_type::public_key:
		p += "_pub.der";
		break;
	case fit::key_type::symmetric_key:
	case fit::key_type::symmetric_iv:
		p += ".bin";
		break;
	}

	std::vector<std::byte> d(std::filesystem::file_size(p));
	std::ifstream f(p, std::ios::binary);
	f.read(reinterpret_cast<char *>(data(d)), size(d));
	return d;
}

std::vector<std::byte>
read_file(const std::filesystem::path &p)
{
	std::vector<std::byte> d{std::filesystem::file_size(p)};
	std::ifstream f{p, std::ios::binary};
	f.read(reinterpret_cast<char *>(data(d)), size(d));
	return d;
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
						 get_key));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes128"),
						 get_key));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes192"),
						 get_key));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes256"),
						 get_key));
}

TEST(fit, verify_image_signatures_offset)
{
	const auto &[f, fd]{fdt::load_keep("verify-offset.fit")};
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-1"),
						 get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes128"),
						 get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes192"),
						 get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes256"),
						 get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
}

TEST(fit, verify_image_signatures_position)
{
	const auto &[f, fd]{fdt::load_keep("verify-position.fit")};
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-1"),
						 get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes128"),
						 get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes192"),
						 get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_image_signatures(get_node(f, "/images/test-aes256"),
						 get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
}

TEST(fit, verify_config_signatures_inline)
{
	const auto &[f, fd]{fdt::load_keep("verify.fit")};
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-1"), get_key, fd));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-2"), get_key, fd));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-3"), get_key, fd));
}

TEST(fit, verify_config_signatures_offset)
{
	const auto &[f, fd]{fdt::load_keep("verify-offset.fit")};
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-1"), get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-2"), get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-3"), get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	}));
}

TEST(fit, verify_config_signatures_position)
{
	const auto &[f, fd]{fdt::load_keep("verify-position.fit")};
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-1"), get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-2"), get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
	EXPECT_TRUE(fit::verify_config_signatures(
			get_node(f, "/configurations/config-3"), get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	}));
}

TEST(fit, image_data_inline)
{
	using std::ranges::equal;

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
	}, get_key);
	EXPECT_TRUE(equal(data, read_file("4k_random")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes192"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_key);
	EXPECT_TRUE(equal(data, as_bytes("test-aes192 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes256"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_key);
	EXPECT_TRUE(equal(data, as_bytes("test-aes256 data")));
}

TEST(fit, image_data_offset)
{
	using std::ranges::equal;

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
	}, get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, read_file("4k_random")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes192"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-aes192 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes256"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-offset.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-aes256 data")));
}

TEST(fit, image_data_position)
{
	using std::ranges::equal;

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
	}, get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, read_file("4k_random")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes192"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-aes192 data")));
	data.clear();
	fit::image_data(get_node(f, "/images/test-aes256"),
	    [&](std::span<const std::byte> d) {
		data.insert(end(data), begin(d), end(d));
	}, get_key, fd,
	    [](auto off, auto len, const auto &process) {
		read_file("verify-position.fit", off, len, process);
	});
	EXPECT_TRUE(equal(data, as_bytes("test-aes256 data")));
}

TEST(fit, decrypt_fuzz)
{
	using std::ranges::equal;

	std::vector<std::byte> data;

	const auto &[f, fd]{fdt::load_keep("verify-position.fit")};
	for (size_t i = 0; i < 10000; ++i) {
		data.clear();
		fit::image_data(get_node(f, "/images/test-aes128"),
		[&](std::span<const std::byte> d) {
			data.insert(end(data), begin(d), end(d));
		}, get_key, fd,
		[](auto off, auto len, const auto &process) {
			read_file_random("verify-position.fit", off, len, process);
		});
		EXPECT_TRUE(equal(data, read_file("4k_random")));
	}
}
