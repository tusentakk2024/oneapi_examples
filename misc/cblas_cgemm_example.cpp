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
// Intel推奨 高速複素行列積
//
// cblas_cgemm 内部数式
// C = alpha * op(A) * op(B) + beta * C
//
// 今回は
// op(A)=A
// op(B)=B
// alpha=1
// beta=0
//
// よって
// C = A × B
//
//------------------------------------------------------------
void complexMatrixMultiplyMKL(
    const MKL_Complex8* A,
    const MKL_Complex8* B,
    MKL_Complex8* C,
    int M, int N, int K)
{
    MKL_Complex8 alpha = { 1.0f, 0.0f };
    MKL_Complex8 beta = { 0.0f, 0.0f };

    cblas_cgemm(
        CblasRowMajor,    // [1] 行列データのメモリ配置形式(Layout)
        //
        //     CblasRowMajor :
        //         C/C++標準の行優先格納
        //
        //     CblasColMajor :
        //         Fortran形式の列優先格納

        CblasNoTrans,     // [2] 行列Aに対して適用する演算(op(A))
        //
        //     CblasNoTrans   : Aそのまま
        //     CblasTrans     : A転置
        //     CblasConjTrans : A共役転置

        CblasNoTrans,     // [3] 行列Bに対して適用する演算(op(B))
        //
        //     CblasNoTrans   : Bそのまま
        //     CblasTrans     : B転置
        //     CblasConjTrans : B共役転置

        M,                // [4] 出力行列Cの行数
        //     = op(A)の行数

        N,                // [5] 出力行列Cの列数
        //     = op(B)の列数

        K,                // [6] 積和内部次元
        //     = op(A)の列数
        //     = op(B)の行数

        &alpha,           // [7] 複素係数 alpha
        //     C = alpha*op(A)*op(B) + beta*C

        A,                // [8] 行列Aの先頭ポインタ
        //     mkl_malloc確保済み64byte aligned領域

        K,                // [9] lda = leading dimension of A
        //
        //     RowMajorではAの1行の要素数
        //     AはM×K → 1行K要素 → lda=K
        //
        //     転置指定しても実メモリ上の元の行幅を書く

        B,                // [10] 行列Bの先頭ポインタ

        N,                // [11] ldb = leading dimension of B
        //
        //      RowMajorではBの1行の要素数
        //      BはK×N → 1行N要素 → ldb=N

        &beta,            // [12] 複素係数 beta
        //
        //      beta=0  : C初期値無視
        //      beta=1  : Cへ加算
        //      beta=-1 : Cを減算

        C,                // [13] 出力行列Cの先頭ポインタ

        N                 // [14] ldc = leading dimension of C
                          //
                          //      RowMajorではCの1行の要素数
                          //      CはM×N → 1行N要素 → ldc=N
    );
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

    if (!A || !B || !C)
    {
        std::cerr << "mkl_malloc failed.\n";
        return -1;
    }

    A[0] = make_complex(1, 1);
    A[1] = make_complex(2, 0);
    A[2] = make_complex(3, -1);
    A[3] = make_complex(4, 2);
    A[4] = make_complex(5, 0);
    A[5] = make_complex(6, 1);

    B[0] = make_complex(1, 0);
    B[1] = make_complex(2, 1);
    B[2] = make_complex(3, -1);
    B[3] = make_complex(4, 0);
    B[4] = make_complex(5, 2);
    B[5] = make_complex(6, -2);

    complexMatrixMultiplyMKL(A, B, C, M, N, K);

    printMatrix(A, M, K, "A");
    printMatrix(B, K, N, "B");
    printMatrix(C, M, N, "C = A * B");

    mkl_free(A);
    mkl_free(B);
    mkl_free(C);

    return 0;
}