#pragma once

#include <cstddef>
#include <vector>

class Compressor {
public:
	/*
	 * Compress the provided data.
	 */
	virtual std::vector<std::byte> compress(const std::vector<std::byte> &data) = 0;

	/*
	 * Decompress the provided data. Returns an empty vector on error.
	 */
	virtual std::vector<std::byte> decompress(const std::vector<std::byte> &data) = 0;
};

/*
 * Doesn't compress the data, returning the original bytes instead.
 */
class PlainCompressor: public Compressor {
private:
	PlainCompressor();

public:
	static PlainCompressor &get();

	std::vector<std::byte> compress(const std::vector<std::byte> &data) override;
	std::vector<std::byte> decompress(const std::vector<std::byte> &data) override;
};
