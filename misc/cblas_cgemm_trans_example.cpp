#include <iostream>
#include <mkl.h>

int main() {
    // 物理的なサイズ (32行8列)
    const int rows_A = 32;
    const int cols_A = 8;

    // 論理的な演算サイズ: op(A) * B = C
    // op(A) は転置により 8行32列 となる
    const int M = 8;  // op(A)の行数
    const int N = 1;  // Bの列数
    const int K = 32; // op(A)の列数 / Bの行数

    // 1. メモリ確保
    MKL_Complex8* A = (MKL_Complex8*)mkl_malloc(rows_A * cols_A * sizeof(MKL_Complex8), 64);
    MKL_Complex8* B = (MKL_Complex8*)mkl_malloc(K * N * sizeof(MKL_Complex8), 64); // 32x1
    MKL_Complex8* C = (MKL_Complex8*)mkl_malloc(M * N * sizeof(MKL_Complex8), 64); // 8x1

    // 2. 初期化
    for (int i = 0; i < rows_A * cols_A; ++i) A[i] = {1.0f, 0.0f};
    for (int i = 0; i < K; ++i) B[i] = {(float)(i + 1), 0.0f};
    
    MKL_Complex8 alpha = {1.0f, 0.0f};
    MKL_Complex8 beta  = {0.0f, 0.0f};

    // 3. 【統合】転置しながら行列積を実行
    // A(32x8)を転置(8x32)として扱い、B(32x1)と掛ける
    cblas_cgemm(
        CblasRowMajor, 
        CblasTrans,    // ★ Aをメモリ上で動かさず、計算時に転置として扱う
        CblasNoTrans,  // Bはそのまま
        M, N, K,       // 8, 1, 32
        &alpha, 
        A, 
        8,             // [lda] Aの物理的な列数(8)を指定
        B, 
        1,             // [ldb] Bの物理的な列数(1)
        &beta, 
        C, 
        1              // [ldc] Cの物理的な列数(1)
    );

    // 4. 結果表示
    std::cout << "--- Result C (8x1) Integrated Transpose & GEMM ---" << std::endl;
    for (int i = 0; i < 3; ++i) { // 先頭3要素
        std::cout << "C[" << i << "]: (" << C[i].real << ", " << C[i].imag << ")" << std::endl;
    }

    mkl_free(A); mkl_free(B); mkl_free(C);
    return 0;
}