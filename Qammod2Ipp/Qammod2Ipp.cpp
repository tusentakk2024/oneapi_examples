#include "Qammod2Ipp.h"
#include <cmath>
#include <stdexcept>

void Qammod2Ipp::InitializeMappingTable(int M, std::vector<Ipp32fc>& table) {
    table.resize(M);
    
    // 矩形QAMマッピングの計算
    int side = static_cast<int>(std::sqrt(M));
    if (side * side != M) {
        throw std::invalid_argument("Modulation order must be a perfect square for rectangular QAM");
    }
    
    float normalization = std::sqrt(2.0f * (M - 1) / 3.0f);  // 平均電力1に正規化
    
    for (int i = 0; i < M; ++i) {
        // バイナリマッピング (MATLAB qammodデフォルト)
        int row = i / side;
        int col = i % side;
        
        // IとQの値 (- (side-1), -(side-3), ..., side-1)
        float I = 2.0f * col - (side - 1);
        float Q = 2.0f * row - (side - 1);
        
        table[i] = {I / normalization, Q / normalization};
    }
}

std::vector<Ipp32fc> Qammod2Ipp::Exec(const std::vector<int>& X, int M) {
    if (M <= 0 || (M & (M - 1)) != 0) {
        throw std::invalid_argument("Modulation order must be a positive power of 2");
    }
    
    std::vector<Ipp32fc> table;
    InitializeMappingTable(M, table);
    
    std::vector<Ipp32fc> output(X.size());
    
    for (size_t i = 0; i < X.size(); ++i) {
        int input = X[i];
        if (input < 0 || input >= M) {
            throw std::invalid_argument("Symbol value out of range");
        }
        output[i] = table[input];
    }
    
    return output;
}