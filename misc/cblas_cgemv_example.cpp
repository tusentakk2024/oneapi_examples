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
// Intel MKL による複素数行列の各行総和
//
// 数学的には
//
//      sum = A × ones
//
//        A(rows×cols)        ones(cols×1)
//     ┌─────────────┐      ┌───┐
//rows │             │ cols │ 1 │
//     │      A      │  ×   │ 1 │
//     │             │      │...│
//     └─────────────┘      └───┘
//              ↓
//         sum(rows×1)
//
//------------------------------------------------------------
void complexRowSumMKL(
    const MKL_Complex8* A,
    MKL_Complex8* sum,
    int rows,
    int cols)
{
    //--------------------------------------------------------
    // 全要素1の複素ベクトル ones を生成
    //--------------------------------------------------------
    MKL_Complex8* ones = (MKL_Complex8*)mkl_malloc(sizeof(MKL_Complex8) * cols, 64);

    for (int i = 0; i < cols; i++)
    {
        ones[i].real = 1.0f;
        ones[i].imag = 0.0f;
    }

    MKL_Complex8 alpha = { 1.0f, 0.0f };
    MKL_Complex8 beta = { 0.0f, 0.0f };

    //--------------------------------------------------------
    // cblas_cgemv 内部数式
    //
    // y = alpha * op(A) * x + beta * y
    //
    // 今回:
    // x = ones
    //
    // よって
    // sum = A × ones = 各行の総和
    //--------------------------------------------------------
    cblas_cgemv(
        CblasRowMajor,    // [1] Layout : 行列メモリ配置形式
        //
        //     CblasRowMajor :
        //         C/C++標準の行優先格納
        //         1行ごとに連続メモリ
        //
        //         [a00 a01 a02
        //          a10 a11 a12]
        //
        //         メモリ:
        //         a00,a01,a02,a10,a11,a12
        //
        //     CblasColMajor :
        //         Fortran列優先格納
        //         1列ごとに連続メモリ
        //
        //         メモリ:
        //         a00,a10,a01,a11,a02,a12
        //
        //     ※oneMKL内部はColMajor文化が強いが
        //       C/C++では通常RowMajorを使う

        CblasNoTrans,     // [2] TransA : 行列Aに適用する演算 op(A)
        //
        //     CblasNoTrans :
        //         Aをそのまま使用
        //
        //     CblasTrans :
        //         Aを転置して使用
        //         → この指定にすると列ごとの総和計算に使える
        //
        //     CblasConjTrans :
        //         Aを共役転置して使用
        //         複素共役 + 転置
        //
        //     今回は各行和なのでAそのまま

        rows,             // [3] M : op(A)の行数
        //
        //     y = op(A) * x の出力ベクトル長になる
        //
        //     今回 A(rows×cols)
        //     op(A)=A
        //     → 出力sumの要素数 = rows

        cols,             // [4] N : op(A)の列数
        //
        //     xベクトルの要素数になる
        //
        //     今回 ones(cols)
        //     よって N=cols

        &alpha,           // [5] alpha : 複素係数
        //
        //     y = alpha*op(A)*x + beta*y
        //
        //     alpha=(1,0):
        //         行列ベクトル積そのまま
        //
        //     alpha=(0.5,0):
        //         結果を半分
        //
        //     alpha=(0,1):
        //         結果をj倍
        //
        //     複素位相回転にも使える

        A,                // [6] A : 行列Aの先頭ポインタ
        //
        //     mkl_mallocで確保した64byte aligned領域
        //     複素数 rows×cols 要素

        cols,             // [7] lda : leading dimension of A
        //
        //     leading dimensionとは
        //     「次の行へ移るためのメモリ上の要素間隔」
        //
        //     RowMajor + NoTrans の場合:
        //         Aの1行要素数 = cols
        //
        //     よって lda = cols
        //
        //     RowMajor + Trans指定でも
        //     実メモリ上の元の行幅を書く
        //
        //     ※ここを間違えると値破壊

        ones,             // [8] x : 入力ベクトル先頭ポインタ
        //
        //     今回は全要素1
        //
        //     x = [1,1,1,...]^T
        //
        //     これにより各行要素が全部足される

        1,                // [9] incx : xベクトルの要素間隔(stride)
        //
        //     incx=1 :
        //         x[0],x[1],x[2]... と連続アクセス
        //
        //     incx=2 :
        //         x[0],x[2],x[4]... を読む
        //
        //     間引きベクトルも指定可能
        //
        //     今回は通常連続配列

        &beta,            // [10] beta : 出力ベクトル既存値への係数
        //
        //      y = alpha*A*x + beta*y
        //
        //      beta=(0,0):
        //          既存sum値を無視して新規計算
        //
        //      beta=(1,0):
        //          既存sumへ加算
        //
        //      beta=(-1,0):
        //          既存sumを減算
        //
        //      複素係数付き累積も可能

        sum,              // [11] y : 出力ベクトル先頭ポインタ
        //
        //      計算結果 rows要素がここへ書き込まれる
        //
        //      beta!=0 の場合は
        //      既存値も読み込まれる

        1                 // [12] incy : yベクトルの要素間隔(stride)
                          //
                          //      incy=1 :
                          //          y[0],y[1],y[2]... へ連続書込
                          //
                          //      incy=2 :
                          //          y[0],y[2],y[4]... へ書込
                          //
                          //      疎な出力配置も可能
                          //
                          //      今回は通常連続配列
    );

    mkl_free(ones);
}

//------------------------------------------------------------
// main
//------------------------------------------------------------
int main()
{
    const int rows = 3;
    const int cols = 3;

    MKL_Complex8* A = (MKL_Complex8*)mkl_malloc(sizeof(MKL_Complex8) * rows * cols, 64);
    MKL_Complex8* sum = (MKL_Complex8*)mkl_malloc(sizeof(MKL_Complex8) * rows, 64);

    if (!A || !sum)
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

    A[6] = make_complex(7, -2);
    A[7] = make_complex(8, 0);
    A[8] = make_complex(9, 3);

    complexRowSumMKL(A, sum, rows, cols);

    printMatrix(A, rows, cols, "A");
    printVector(sum, rows, "Row Sum");

    mkl_free(A);
    mkl_free(sum);

    return 0;
}