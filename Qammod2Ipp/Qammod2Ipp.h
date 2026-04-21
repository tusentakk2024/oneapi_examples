#ifndef QAMMOD2IPP_H
#define QAMMOD2IPP_H

#include <vector>
#include <ipp.h>
#include <complex>

/**
 * @brief Intel IPPを使用したQAM変調器
 *
 * Matlabのqammod関数と同等の機能を実装
 * 矩形QAMマッピングを使用し、平均電力が1になるように正規化
 */
class Qammod2Ipp {
private:
    // QAMマッピングテーブルを初期化
    static void InitializeMappingTable(int M, std::vector<Ipp32fc>& table);

public:
    /**
     * @brief シンボルをQAM変調 (MATLAB qammodと同等)
     * @param X 入力シンボルベクトル (0からM-1の整数)
     * @param M 変調次数 (4, 16, 64, etc.)
     * @return QAM変調された複素数信号
     */
    static std::vector<Ipp32fc> Exec(const std::vector<int>& X, int M);
};

#endif // QAMMOD2IPP_H