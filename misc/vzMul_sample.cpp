#include <mkl.h>
#include <vector>
#include <iostream>

int main() {
    const int n = 1000000; // 100万要素
    std::vector<MKL_Complex16> a(n, {2.0, 3.0});
    std::vector<MKL_Complex16> b(n, {4.0, 5.0});
    std::vector<MKL_Complex16> c(n);

    // ベクトル複素数掛け算の実行 (c = a * b)
    // Intel CPUのSIMD命令を使い、並列に一括計算される
    vzMul(n, a.data(), b.data(), c.data());

    std::cout << "1つ目の結果: " << c[0].real << " + " << c[0].imag << "i" << std::endl;
    return 0;
}