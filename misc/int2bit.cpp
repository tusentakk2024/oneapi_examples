#include <iostream>
#include <vector>
#include <cstdint>

void int2bit(const std::vector<int32_t>& input,
    std::vector<uint8_t>& output,
    int bit_width,
    bool msb_first = true)
{
    const size_t n = input.size();
    output.resize(n * bit_width);

    for (size_t i = 0; i < n; ++i) {

        uint32_t v = static_cast<uint32_t>(input[i]);

        for (int b = 0; b < bit_width; ++b) {
            int bit_index = msb_first ? (bit_width - 1 - b) : b;
            output[i * bit_width + b] = (v >> bit_index) & 1u;
        }
    }
}