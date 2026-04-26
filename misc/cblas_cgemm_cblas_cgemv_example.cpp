#include <iostream>
#include <iomanip>
#include "mkl.h"

//------------------------------------------------------------
// 複素数生成
//------------------------------------------------------------
inline MKL_Complex8 make_complex(float r, float i)
{
    MKL_Complex8 v;
    v.real = r;
    v.imag = i;
    return v;
}

//------------------------------------------------------------
// 行列表示
//------------------------------------------------------------
void printMatrix(const MKL_Complex8* M, int rows, int cols, const char* name)
{
    std::cout << "\n" << name << " =\n";
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            const MKL_Complex8& v = M[i * cols + j];
            std::cout << std::setw(12)
                << "(" << v.real << "," << v.imag << ") ";
        }
        std::cout << "\n";
    }
}

//------------------------------------------------------------
// ベクトル表示
//------------------------------------------------------------
void printVector(const MKL_Complex8* V, int size, const char* name)
{
    std::cout << "\n" << name << " =\n";
    for (int i = 0; i < size; i++)
    {
        std::cout << "(" << V[i].real << "," << V[i].imag << ")\n";
    }
}

//------------------------------------------------------------
// 複素数行列積 + 行ごとの総和
//
// Step1:
//   C = A × B
//
// Step2:
//   Sum = C × ones
//
// 行列サイズ:
//
//        A(M×K)        B(K×N)
//     ┌────────┐    ┌────────┐
//     │   A    │ ×  │   B    │
//     └────────┘    └────────┘
//            ↓
//          C(M×N)
//
//        C(M×N)      ones(N×1)
//     ┌────────┐    ┌───┐
//     │   C    │ ×  │ 1 │
//     └────────┘    │ 1 │
//                   │...│
//                   └───┘
//            ↓
//         Sum(M×1)
//------------------------------------------------------------
void complexMatrixMultiplyAndRowSumMKL(
    const MKL_Complex8* A,
    const MKL_Complex8* B,
    MKL_Complex8* C,
    MKL_Complex8* Sum,
    int M, int N, int K)
{
    //--------------------------------------------------------
    // Step1 : C = A × B
    //--------------------------------------------------------
    MKL_Complex8 alpha1 = { 1.0f, 0.0f };
    MKL_Complex8 beta1 = { 0.0f, 0.0f };

    cblas_cgemm(
        CblasRowMajor,    // [1] C/C++行優先格納
        CblasNoTrans,     // [2] Aそのまま
        CblasNoTrans,     // [3] Bそのまま
        M,                // [4] Cの行数
        N,                // [5] Cの列数
        K,                // [6] 積和内部次元
        &alpha1,          // [7] alpha
        A,                // [8] A先頭
        K,                // [9] lda = Aの1行要素数
        B,                // [10] B先頭
        N,                // [11] ldb = Bの1行要素数
        &beta1,           // [12] beta
        C,                // [13] 出力C
        N                 // [14] ldc = Cの1行要素数
    );

    //--------------------------------------------------------
    // Step2 : Sum = C × ones
    //--------------------------------------------------------
    MKL_Complex8* ones = (MKL_Complex8*)mkl_malloc(sizeof(MKL_Complex8) * N, 64);

    for (int i = 0; i < N; i++)
    {
        ones[i].real = 1.0f;
        ones[i].imag = 0.0f;
    }

    MKL_Complex8 alpha2 = { 1.0f, 0.0f };
    MKL_Complex8 beta2 = { 0.0f, 0.0f };

    cblas_cgemv(
        CblasRowMajor,    // [1] C/C++行優先格納
        CblasNoTrans,     // [2] Cをそのまま使う
        M,                // [3] Cの行数
        N,                // [4] Cの列数
        &alpha2,          // [5] alpha
        C,                // [6] 行列C先頭
        N,                // [7] lda = Cの1行要素数
        ones,             // [8] 全要素1ベクトル
        1,                // [9] incx = xの要素間隔
        &beta2,           // [10] beta
        Sum,              // [11] 出力Sum
        1                 // [12] incy = yの要素間隔
    );

    mkl_free(ones);
}

//------------------------------------------------------------
// main
//------------------------------------------------------------
int main()
{
    const int M = 2;
    const int K = 3;
    const int N = 2;

    MKL_Complex8* A = (MKL_Complex8*)mkl_malloc(sizeof(MKL_Complex8) * M * K, 64);
    MKL_Complex8* B = (MKL_Complex8*)mkl_malloc(sizeof(MKL_Complex8) * K * N, 64);
    MKL_Complex8* C = (MKL_Complex8*)mkl_malloc(sizeof(MKL_Complex8) * M * N, 64);
    MKL_Complex8* Sum = (MKL_Complex8*)mkl_malloc(sizeof(MKL_Complex8) * M, 64);

    if (!A || !B || !C || !Sum)
    {
        std::cerr << "mkl_malloc failed.\n";
        return -1;
    }

    //--------------------------------------------------------
    // A = 2×3
    //--------------------------------------------------------
    A[0] = make_complex(1, 1);
    A[1] = make_complex(2, 0);
    A[2] = make_complex(3, -1);
    A[3] = make_complex(4, 2);
    A[4] = make_complex(5, 0);
    A[5] = make_complex(6, 1);

    //--------------------------------------------------------
    // B = 3×2
    //--------------------------------------------------------
    B[0] = make_complex(1, 0);
    B[1] = make_complex(2, 1);
    B[2] = make_complex(3, -1);
    B[3] = make_complex(4, 0);
    B[4] = make_complex(5, 2);
    B[5] = make_complex(6, -2);

    complexMatrixMultiplyAndRowSumMKL(A, B, C, Sum, M, N, K);

    printMatrix(A, M, K, "A");
    printMatrix(B, K, N, "B");
    printMatrix(C, M, N, "C = A * B");
    printVector(Sum, M, "Row Sum of C");

    mkl_free(A);
    mkl_free(B);
    mkl_free(C);
    mkl_free(Sum);

    return 0;
}