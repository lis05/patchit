#include <patch.hpp>

void Instruction::set_compressor(std::shared_ptr<Compressor> compressor) {
    this->compressor = compressor;
}
