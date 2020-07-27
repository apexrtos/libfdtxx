/*
 * fit++ - flattened image tree C++ library
 *
 * Copyright 2020 Patrick Oppenlander <patrick.oppenlander@gmail.com>
 *
 * SPDX-License-Identifier: 0BSD
 */
#pragma once

#include "libfdt++.h"

#include <functional>

namespace fit {

/*
 * process_fn - process image data from FIT
 *
 * This function is used to handle FIT images with external data.
 */
using process_fn = std::function<void(std::span<const std::byte>)>;

/*
 * read_fn - read image data from FIT
 *
 * This function is used to handle FIT images with external data.
 *
 * The function must read len bytes from offset and pass them to process_fn.
 * process_fn can be called multiple times with contiguous spans if len is
 * large.
 *
 * An exception must be thrown if the read encounters an error.
 */
using read_fn = std::function<void(size_t off, size_t len, const process_fn &)>;

/*
 * get_public_key_fn - get public key from key name hint
 *
 * This function is used to load keys for verification of FIT signatures.
 *
 * Call process_fn with key bytes if key is available.
 * Return without calling process_fn if key is not required.
 * Throw exception if the key is required but cannot be loaded.
 */
using get_public_key_fn = std::function<void(std::string_view key_name,
					     const process_fn &)>;

/*
 * get_symmetric_key_fn - get symmetric key from key and iv name hints
 *
 * This function is used to load keys for decryption of FIT data.
 *
 * Call key_iv_fn with key and iv bytes.
 * Return without calling key_iv_fn or throw if the key cannot be loaded.
 */
using key_iv_fn = std::function<void(std::span<const std::byte> key,
				     std::span<const std::byte> iv)>;
using get_symmetric_key_fn = std::function<void(std::string_view key_name,
						std::string_view iv_name,
						const key_iv_fn &)>;

/*
 * image_data_size - get data size for image
 *
 * This function returns the size of the binary data for a FIT image node.
 *
 * Throws libfdt++ exceptions if the node is not a valid FIT image node.
 */
size_t
image_data_size(const fdt::node &);

/*
 * image_data - retrieve data for image
 *
 * This function retrieves the binary data for a FIT image node.
 *
 * fdt and read_fn must be provided to support FIT images with external data.
 * get_symmetric_key_fn must be provided to support encrypted FIT image data.
 *
 * Note that there is no way to verify that the correct symmetric key is
 * provided and therefore no guarantee that the decrypted data is sane.
 *
 * The data is passed to process_fn. process_fn can be called multiple times
 * with contiguous spans if the data is large.
 *
 * Throws libfdt++ exceptions if the node is not a valid FIT image node.
 * Throws std::runtime_error if:
 * - The FIT image has external data but no read_fn was provided
 */
void
image_data(const fdt::node &, const process_fn &);

void
image_data(const fdt::node &, const process_fn &, const get_symmetric_key_fn &);

void
image_data(const fdt::node &, const process_fn &,
	   std::span<const std::byte> fdt, const read_fn &);
void
image_data(const fdt::node &, const process_fn &, const get_symmetric_key_fn &,
	   std::span<const std::byte> fdt, const read_fn &);

/*
 * verify_image_hashes - verify all hashes on an image node
 *
 * fdt and read_fn must be provided to support FIT images with external data.
 *
 * Returns true if all hash* nodes verify.
 * Returns false if a hash fails to verify.
 *
 * Throws libfdt++ exceptions if the node is not a valid FIT image node.
 * Throws std::runtime_error if:
 * - There are no hash* nodes to verify
 * - A hash algorithm is not supported
 * - A cryptographic algorithm fails
 * - The FIT image has external data but no read_fn was provided
 */
bool
verify_image_hashes(const fdt::node &);

bool
verify_image_hashes(const fdt::node &,
		    std::span<const std::byte> fdt, const read_fn &);

/*
 * verify_image_signatures - verify all signatures on an image node
 *
 * fdt and read_fn must be provided to support FIT images with external data.
 *
 * Returns true if all required signature* nodes verify.
 * Returns false if there are no signature* nodes.
 * Returns false if a signature fails to verify.
 *
 * Throws libfdt++ exceptions if the node is not a valid FIT image node.
 * Throws std::runtime_error if:
 * - There are no hash* nodes to verify
 * - There are no signature* nodes to verify
 * - A hash algorithm is not supported
 * - A cryptographic algorithm fails
 * - The FIT image has external data but no read_fn was provided
 */
bool
verify_image_signatures(const fdt::node &, const get_public_key_fn &);

bool
verify_image_signatures(const fdt::node &, const get_public_key_fn &,
			std::span<const std::byte> fdt, const read_fn &);

/*
 * verify_config_signatures - verify all signatures on a config node
 *
 * fdt and read_fn must be provided to support FIT images with external data.
 *
 * Returns true if all required signature* nodes verify.
 * Returns false if there are no signature* nodes.
 * Returns false if a signature fails to verify.
 *
 * Throws libfdt++ exceptions if the node is not a valid FIT configuration node.
 * Throws std::runtime_error.
 */
bool
verify_config_signatures(const fdt::node &, const get_public_key_fn &,
			 std::span<const std::byte> fdt);

bool
verify_config_signatures(const fdt::node &, const get_public_key_fn &,
			 std::span<const std::byte> fdt, const read_fn &);

}
