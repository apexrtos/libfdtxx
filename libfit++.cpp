#include "libfit++.h"

#include <array>
#include <tomcrypt.h>

extern "C" {
#include <libfdt.h>
}

using namespace std::literals;

/*
 * These defines are to limit supported key formats at compile time to reduce
 * code size, e.g.:
 *
 * gcc -DRSA_PKCS_8=0 -DRSA_X509=0
 */
#ifndef RSA_PKCS_1
	#define RSA_PKCS_1 1
#endif
#ifndef RSA_PKCS_8
	#define RSA_PKCS_8 1
#endif
#ifndef RSA_X509
	#define RSA_X509 1
#endif

namespace fit {

namespace {

constexpr std::span<const std::byte> empty_span;

/*
 * contains - check if container contains value
 */
template<class Container, class Value>
bool
contains(const Container &c, const Value &v)
{
	return std::find(begin(c), end(c), v) != end(c);
}

/*
 * ltc_check - check return value of libtomcrypt function and throw on error
 */
void
ltc_check(int ret)
{
	if (ret == CRYPT_OK)
		return;
	throw std::runtime_error(error_to_string(ret));
}

/*
 * ltc_hash - wrap up libtomcrypt hash operations
 */
class ltc_hash {
public:
	ltc_hash(std::string_view name)
	: hd_{hash_descriptor[find(name)]}
	{
		assert(hd_.hashsize <= size(hash_));
		ltc_check(hd_.init(&state_));
	}

	void
	process(std::span<const std::byte> d)
	{
		ltc_check(hd_.process(&state_,
			reinterpret_cast<const unsigned char *>(data(d)),
			size(d)));
	}

	void
	done()
	{
		hd_.done(&state_, reinterpret_cast<unsigned char *>(data(hash_)));
	}

	std::span<const std::byte>
	value() const
	{
		return {data(hash_), hd_.hashsize};
	}

	static int find(std::string_view name)
	{
		LTC_MUTEX_LOCK(&ltc_hash_mutex);
		for (auto i{0}; i < TAB_SIZE; i++) {
			if (hash_descriptor[i].name != name)
				continue;
			LTC_MUTEX_UNLOCK(&ltc_hash_mutex);
			return i;
		}
		LTC_MUTEX_UNLOCK(&ltc_hash_mutex);

		throw std::runtime_error{"hash not supported"};
	}

private:
	const ltc_hash_descriptor &hd_;
	hash_state state_;
	std::array<std::byte, MAXBLOCKSIZE> hash_;
};

/*
 * ltc_rsa - wrap up libtomcrypt rsa operations
 */
class ltc_rsa {
public:
	ltc_rsa() = default;
	ltc_rsa(ltc_rsa &&) = delete;
	ltc_rsa(const ltc_rsa &) = delete;
	ltc_rsa &operator=(ltc_rsa &&) = delete;
	ltc_rsa &operator=(const ltc_rsa &) = delete;

	~ltc_rsa()
	{
		rsa_free(&k_);
	}

	void
	load(std::span<const std::byte> k)
	{
		rsa_free(&k_);

		const unsigned char *d{reinterpret_cast<const unsigned char *>(data(k))};
		if (RSA_PKCS_1 &&
		    rsa_import(d, size(k), &k_) == CRYPT_OK)
			return;
		if (RSA_PKCS_8 &&
		    rsa_import_pkcs8(d, size(k), nullptr, 0, &k_) == CRYPT_OK)
			return;
		if (RSA_X509 &&
		    rsa_import_x509(d, size(k), &k_) == CRYPT_OK)
			return;
		throw std::runtime_error{"rsa key import failed"};
	}

	bool
	verify(std::span<const std::byte> signature,
	       std::span<const std::byte> hash,
	       std::string_view hash_algo)
	{
		int s;
		ltc_check(rsa_verify_hash_ex(
			reinterpret_cast<const unsigned char *>(data(signature)),
			size(signature),
			reinterpret_cast<const unsigned char *>(data(hash)),
			size(hash),
			LTC_PKCS_1_V1_5, ltc_hash::find(hash_algo), 0, &s, &k_));
		return s == 1;
	}

private:
	rsa_key k_{};
};

/*
 * ltc_cipher - wrap up libtomcrypt cipher operations
 */
class ltc_cipher {
public:
	static int find(std::string_view name)
	{
		LTC_MUTEX_LOCK(&ltc_cipher_mutex);
		for (auto i{0}; i < TAB_SIZE; i++) {
			if (cipher_descriptor[i].name != name)
				continue;
			LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);
			return i;
		}
		LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);

		throw std::runtime_error{"cipher not supported"};
	}
};

/*
 * ltc_cbc - wrap up libtomcrypt cbc operations
 */
class ltc_cbc {
public:
	ltc_cbc() = default;
	ltc_cbc(ltc_cbc &&) = delete;
	ltc_cbc(const ltc_cbc &) = delete;
	ltc_cbc &operator=(ltc_cbc &&) = delete;
	ltc_cbc &operator=(const ltc_cbc &) = delete;

	~ltc_cbc()
	{
		cbc_done(&cbc_);
	}

	void
	init(std::string_view cipher, std::span<const std::byte> key,
	     std::span<const std::byte> iv)
	{
		cbc_done(&cbc_);

		auto idx{ltc_cipher::find(cipher)};
		if (static_cast<int>(size(iv)) != cipher_descriptor[idx].block_length)
			throw std::runtime_error{"bad iv size"};
		ltc_check(cbc_start(idx,
			reinterpret_cast<const unsigned char *>(data(iv)),
			reinterpret_cast<const unsigned char *>(data(key)),
			size(key), 0, &cbc_));
	}

	void
	decrypt(std::span<const std::byte> ct, const process_fn &process)
	{
		/* REVISIT(efficiency): b_.resize() initalises memory */
		/* REVISIT(efficiency): inplace decrypt for whole blocks? */

		if (empty(ct))
			return;

		if (!empty(b_) ||
		    size(ct) < static_cast<size_t>(cbc_.blocklen)) {
			/* handle incomplete blocks */
			const auto sz{std::min(cbc_.blocklen - size(b_),
					       size(ct))};
			b_.insert(end(b_), begin(ct), begin(ct) + sz);

			/* not enough data to complete block */
			if (size(b_) != static_cast<size_t>(cbc_.blocklen))
				return;

			/* decrypt complete reassembled block */
			ltc_check(cbc_decrypt(
				reinterpret_cast<const unsigned char *>(data(b_)),
				reinterpret_cast<unsigned char *>(data(b_)),
				size(b_), &cbc_));
			process(b_);
			b_.clear();

			/* process remaining ciphertext */
			return decrypt(ct.subspan(sz), process);
		}

		/* decrypt whole blocks */
		const auto sz{size(ct) / cbc_.blocklen * cbc_.blocklen};
		b_.resize(sz);
		ltc_check(cbc_decrypt(
			reinterpret_cast<const unsigned char *>(data(ct)),
			reinterpret_cast<unsigned char *>(data(b_)),
			sz, &cbc_));
		process(b_);
		b_.clear();

		/* process remaining ciphertext */
		return decrypt(ct.subspan(sz), process);
	}

private:
	std::vector<std::byte> b_;
	symmetric_CBC cbc_{};
};

/*
 * no_external - read function stub when no external data source is provided
 */
void
no_external(size_t off, size_t len, const process_fn &fn)
{
	throw std::runtime_error{"no external data source"};
}

/*
 * no_key - key function stub when no key source is provided
 */
void
no_key(std::string_view key_name, std::string_view iv_name, const key_iv_fn &f)
{
	throw std::runtime_error{"no key source"};
}

/*
 * get_hash_value - find the hash value for hash algorithm algo
 */
std::span<const std::byte>
get_hash_value(const fdt::node &n, std::string_view algo)
{
	for (const auto &h : subnodes(n)) {
		if (!name(h).starts_with("hash"))
			continue;
		if (as_string(get_property(h, "algo")) != algo)
			continue;
		return as_bytes(get_property(h, "value"));
	}
	throw std::runtime_error{"hash not found"};
}

/*
 * hash_raw_nodes - run hash algorithm across FDT structure block for nodes
 */
void
hash_raw_nodes(const std::span<const std::byte> fdt,
	       const std::span<const std::string_view> nodes,
	       const std::span<const std::string_view> exclude_props,
	       ltc_hash &hash)
{
	std::string path;
	int off, noff{0}, hoff{-1};
	const auto soff{fdt_off_dt_struct(data(fdt))};

	auto run_hash = [&](bool run) {
		if (run) {
			if (hoff >= 0)
				return;
			hoff = off;
		} else {
			if (hoff < 0)
				return;
			hash.process(fdt.subspan(soff + hoff, off - hoff));
			hoff = -1;
		}
	};

	/*
	 * hlev == 0: don't hash anything at this level
	 * hlev == 1: hash structure at this level (BEGIN_NODE, END_NODE)
	 * hlev == 2: hash everything at this level
	 */
	auto walk_nodes = [&](const auto &self, const int hlev) -> void {
		while (true) {
			off = noff;
			switch (fdt_next_tag(data(fdt), off, &noff)) {
			case FDT_PROP: {
				int len;
				const auto &prop{fdt_get_property_by_offset(data(fdt), off, nullptr)};
				if (!prop)
					throw std::runtime_error{fdt_strerror(FDT_ERR_TRUNCATED)};
				const auto &str{fdt_get_string(data(fdt), fdt32_to_cpu(prop->nameoff), &len)};
				if (!str)
					throw std::runtime_error{fdt_strerror(len)};
				run_hash(hlev > 1 && !contains(exclude_props, str));
				break;
			}
			case FDT_NOP:
				run_hash(hlev > 1);
				break;
			case FDT_BEGIN_NODE: {
				int len;
				const auto &name{fdt_get_name(data(fdt), off, &len)};
				if (!name)
					throw std::runtime_error{fdt_strerror(len)};
				const auto path_prev{size(path)};
				if (!path.ends_with('/'))
					path.append("/");
				path.append(name, len);
				const auto include{contains(nodes, path)};
				run_hash(hlev > 1 || include);
				self(self, include ? 2 : hlev > 1);
				path.erase(path_prev);
				break;
			}
			case FDT_END_NODE:
				run_hash(hlev);
				return;
			case FDT_END:
				if (noff < 0)
					throw std::runtime_error{fdt_strerror(noff)};
				run_hash(true);
				return;
			}
		}
	};

	walk_nodes(walk_nodes, 0);

	/* hash any remaining data */
	off = noff;
	run_hash(false);
}

/*
 * image_data_raw - retrieve potentially encrypted image data
 */
void
image_data_raw(const fdt::node &n, const process_fn &process,
	   std::span<const std::byte> fdt, const read_fn &read)
{
	/* inline data */
	if (const auto &d{find(n, "data")}; d) {
		process(as_bytes(as_property(*d)));
		return;
	}

	/* external data */
	const auto data_size{as<uint32_t>(get_property(n, "data-size"))};

	/* data-offset starts at 4-byte aligned offset after fdt */
	if (const auto &d{find(n, "data-offset")}; d) {
		if (size(fdt) < FDT_V1_SIZE)
			throw std::runtime_error{fdt_strerror(FDT_ERR_TRUNCATED)};
		const auto data_begin{(fdt_totalsize(data(fdt)) + 3) & -4};
		const auto off{as<uint32_t>(as_property(*d))};
		read(data_begin + off, data_size, process);
		return;
	}

	/* data-position is absolute */
	if (const auto &d{find(n, "data-position")}; d) {
		read(as<uint32_t>(as_property(*d)), data_size, process);
		return;
	}

	throw std::runtime_error{"missing data property"};
}

}

size_t
image_data_size(const fdt::node &n)
{
	/* encrypted data */
	if (const auto &s{find(n, "data-size-unciphered")}; s)
		return as<uint32_t>(as_property(*s));

	/* inline data */
	if (const auto &d{find(n, "data")}; d)
		return size(as_bytes(as_property(*d)));

	/* external data */
	return as<uint32_t>(get_property(n, "data-size"));
}

void
image_data(const fdt::node &n, const process_fn &process)
{
	return image_data(n, process, no_key, empty_span, no_external);
}

void
image_data(const fdt::node &n, const process_fn &process,
	   const get_symmetric_key_fn &get_key)
{
	return image_data(n, process, get_key, empty_span, no_external);
}

void
image_data(const fdt::node &n, const process_fn &process,
	   std::span<const std::byte> fdt, const read_fn &read)
{
	return image_data(n, process, no_key, fdt, read);
}

void
image_data(const fdt::node &n, const process_fn &process,
	   const get_symmetric_key_fn &get_key,
	   std::span<const std::byte> fdt, const read_fn &read)
{
	/* encrypted data */
	const auto &have_cipher{find(n, "cipher")};
	if (!have_cipher)
		return image_data_raw(n, process, fdt, read);

	/* get cipher properties */
	const auto &cipher{as_node(have_cipher.value())};
	const auto &algo{as_string(get_property(cipher, "algo"))};
	const auto &key_name{as_string(get_property(cipher, "key-name-hint"))};
	const auto &iv_name{as_string(get_property(cipher, "iv-name-hint"))};
	const auto &keylen_begin{std::find_if(begin(algo), end(algo), isdigit)};
	const auto &cipher_name{algo.substr(0, keylen_begin - begin(algo))};

	/* load key */
	bool loaded{false};
	ltc_cbc cbc;
	get_key(key_name, iv_name, [&](std::span<const std::byte> key,
				       std::span<const std::byte> iv) {
		cbc.init(cipher_name, key, iv);
		loaded = true;
	});

	/* symmetric keys must be provided */
	if (!loaded)
		throw std::runtime_error{"missing symmetric key"};

	/* decrypt image */
	size_t remain{as<uint32_t>(get_property(n, "data-size-unciphered"))};
	image_data_raw(n, [&](std::span<const std::byte> ct) {
		cbc.decrypt(ct, [&](std::span<const std::byte> pt) {
			const auto sz{std::min(size(pt), remain)};
			if (!sz)
				return;
			process({data(pt), sz});
			remain -= sz;
		});
	}, fdt, read);
}

bool
verify_image_hashes(const fdt::node &n)
{
	return verify_image_hashes(n, empty_span, no_external);
}

bool
verify_image_hashes(const fdt::node &n,
		    std::span<const std::byte> fdt, const read_fn &read)
{
	auto have_hash{false};
	for (const auto &h : subnodes(n)) {
		if (!name(h).starts_with("hash"))
			continue;

		const auto &algo{as_string(get_property(h, "algo"))};
		const auto &value{as_bytes(get_property(h, "value"))};

		if (algo == "crc32") {
#ifdef LTC_CRC32
			std::array<std::byte, 4> h;
			if (size(value) != size(h))
				return false;
			crc32_state s;
			crc32_init(&s);
			image_data_raw(n, [&s](std::span<const std::byte> d) {
				   crc32_update(&s, reinterpret_cast<const unsigned char *>(data(d)), size(d));
			}, fdt, read);
			crc32_finish(&s, data(h), size(h));
			if (!std::ranges::equal(h, value))
				return false;
			have_hash = true;
#else
			throw std::runtime_error{"crc32 not supported"};
#endif
		} else {
			ltc_hash h{algo};
			image_data_raw(n, [&](std::span<const std::byte> d) {
				h.process(d);
			}, fdt, read);
			h.done();
			if (!std::ranges::equal(h.value(), value))
				return false;
			have_hash = true;
		}
	}

	if (!have_hash)
		throw std::runtime_error{"no hashes"};

	return true;
}

bool
verify_image_signatures(const fdt::node &n, const get_public_key_fn &get_key)
{
	return verify_image_signatures(n, get_key, empty_span, no_external);
}

bool
verify_image_signatures(const fdt::node &n, const get_public_key_fn &get_key,
			std::span<const std::byte> fdt, const read_fn &read)
{
	/* make sure image hasn't been corrupted */
	if (!verify_image_hashes(n, fdt, read))
		return false;

	auto have_signature{false};

	for (const auto &s : subnodes(n)) {
		if (!name(s).starts_with("signature"))
			continue;

		/* get signature properties */
		const auto &key_name{as_string(get_property(s, "key-name-hint"))};
		const auto &algo{as_string(get_property(s, "algo"))};
		const auto &sval{as_bytes(get_property(s, "value"))};
		const auto &algo_split{algo.find(',')};
		if (algo_split == std::string_view::npos)
			throw std::runtime_error{"signature algorithm not supported"};
		const auto &hash_algo{algo.substr(0, algo_split)};
		const auto &sig_algo{algo.substr(algo_split + 1)};
		const auto &hval{get_hash_value(n, hash_algo)};

		if (!sig_algo.starts_with("rsa"))
			throw std::runtime_error{"signature algorithm not supported"};

		/* load key */
		ltc_rsa rsa;
		bool loaded{false};
		get_key(key_name, [&](std::span<const std::byte> d) {
			rsa.load(d);
			loaded = true;
		});

		/* key not required? */
		if (!loaded)
			continue;

		/* verify signature */
		if (!rsa.verify(sval, hval, hash_algo))
			return false;

		have_signature = true;
	}

	if (!have_signature)
		throw std::runtime_error{"no signatures"};

	return true;
}

bool
verify_config_signatures(const fdt::node &n, const get_public_key_fn &get_key,
			 std::span<const std::byte> fdt)
{
	return verify_config_signatures(n, get_key, fdt, no_external);
}

bool
verify_config_signatures(const fdt::node &n, const get_public_key_fn &get_key,
			 std::span<const std::byte> fdt, const read_fn &read)
{
	/* sanity check fdt blob */
	if (size(fdt) < FDT_V1_SIZE ||
	    size(fdt) < fdt_header_size(data(fdt)) ||
	    size(fdt) < fdt_totalsize(data(fdt)))
		throw std::runtime_error{fdt_strerror(FDT_ERR_TRUNCATED)};
	if (auto r = fdt_check_header(data(fdt)); r < 0)
		throw std::runtime_error{fdt_strerror(r)};

	auto have_signature{false};
	std::vector<std::string_view> verified_images;

	for (const auto &s : subnodes(n)) {
		if (!name(s).starts_with("signature"))
			continue;

		/* get signature properties */
		const auto &strings_off{fdt_off_dt_strings(data(fdt))};
		const auto strings_size{as<std::array<uint32_t, 2>>(get_property(s, "hashed-strings"))[1]};
		const auto &hashed_nodes{as_stringlist(get_property(s, "hashed-nodes"))};
		const auto &sval{as_bytes(get_property(s, "value"))};
		const auto &algo{as_string(get_property(s, "algo"))};
		const auto &hash_algo{algo.substr(0, algo.find(','))};
		const auto &key_name{as_string(get_property(s, "key-name-hint"))};
		const auto exclude_props = {"data"sv, "data-size"sv, "data-position"sv, "data-offset"sv};

		/* load key */
		ltc_rsa rsa;
		bool loaded{false};
		get_key(key_name, [&](std::span<const std::byte> d) {
			rsa.load(d);
			loaded = true;
		});

		/* key not required? */
		if (!loaded)
			continue;

		/* sanity check - configuration must hash itself */
		if (!contains(hashed_nodes, path(n)))
			return false;

		/* verify hash on all signed images */
		for (const auto &h : hashed_nodes) {
			if (!h.starts_with("/images/") ||
			    h.find('/', 8) != std::string_view::npos)
				continue;
			const auto p{h.substr(1)};
			if (contains(verified_images, p))
				continue;
			if (!verify_image_hashes(get_node(root(n), p), fdt, read))
				return false;
			verified_images.push_back(p);
		}

		/* make sure strings size is sane */
		if (strings_off + strings_size > size(fdt))
			throw std::runtime_error{fdt_strerror(FDT_ERR_TRUNCATED)};

		/* compute hash */
		ltc_hash h{hash_algo};
		hash_raw_nodes(fdt, hashed_nodes, exclude_props, h);
		h.process(fdt.subspan(strings_off, strings_size));
		h.done();

		/* verify signature */
		if (!rsa.verify(sval, h.value(), hash_algo))
			return false;

		have_signature = true;
	}

	if (!have_signature)
		throw std::runtime_error{"no signatures"};

	return true;
}

}
