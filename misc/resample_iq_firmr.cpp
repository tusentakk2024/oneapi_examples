#include <iostream>
#include <vector>
#include <complex>
#include <ipp.h>

void resample_iq_firmr(const std::vector<std::complex<float>>& input_signal) {
    const int upFactor = 5;
    const int downFactor = 6;
    const int tapsLen = 60; // upFactorの倍数

    int srcLen = (int)input_signal.size();

    // 【重要】numIters の計算
    // ippsFIRMR における numIters は、通常「downFactorごとの入力ブロック数」を指します。
    // つまり、(srcLen / downFactor) 回の繰り返し処理を指定します。
    int numIters = srcLen / downFactor;
    int expectedDstLen = numIters * upFactor;

    if (numIters <= 0) return;

    int specSize, bufferSize;
    ippsFIRMRGetSize32f_32fc(tapsLen, upFactor, downFactor, &specSize, &bufferSize);

    IppsFIRSpec32f_32fc* pSpec = (IppsFIRSpec32f_32fc*)ippMalloc(specSize);
    Ipp8u* pBuffer = (Ipp8u*)ippMalloc(bufferSize);

    // アライメントを考慮したIPPアロケータを使用
    Ipp32fc* pSrc = (Ipp32fc*)ippMalloc(srcLen * sizeof(Ipp32fc));
    Ipp32fc* pDst = (Ipp32fc*)ippMalloc(expectedDstLen * sizeof(Ipp32fc));

    // データコピー
    for (int i = 0; i < srcLen; ++i) {
        pSrc[i].re = input_signal[i].real();
        pSrc[i].im = input_signal[i].imag();
    }

    std::vector<Ipp32f> taps(tapsLen, 1.0f / upFactor);
    ippsFIRMRInit32f_32fc(taps.data(), tapsLen, upFactor, 0, downFactor, 0, pSpec);

    // 【重要】第5, 第6引数は通常、ステート（pSpec）を使用する場合 NULL で動作します。
    // numIters に正しく「繰り返し回数（srcLen/6）」を渡すことで、
    // 入力・出力バッファの境界を正しく管理させます。
    IppStatus status = ippsFIRMR32f_32fc(pSrc, pDst, numIters, pSpec, nullptr, nullptr, pBuffer);

    if (status == ippStsNoErr) {
        std::cout << "Success! Generated " << expectedDstLen << " samples." << std::endl;
    }
    else {
        std::cerr << "IPP Error: " << ippGetStatusString(status) << std::endl;
    }

    ippFree(pSpec); ippFree(pBuffer); ippFree(pSrc); ippFree(pDst);
}

int main() {
    ippInit();
    // 6の倍数である 1920 サンプルを入力
    std::vector<std::complex<float>> input(1920, { 1.0f, 0.0f });
    resample_iq_firmr(input);
    return 0;
}