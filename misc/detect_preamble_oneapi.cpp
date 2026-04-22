#include <iostream>
#include <vector>
#include <complex>
#include <ipp.h>

std::vector<int> detect_preamble_oneapi(const std::vector<std::complex<float>>& rx_signal,
    const std::vector<std::complex<float>>& preamble,
    float threshold) {
    int srcLen = (int)rx_signal.size();
    int tapsLen = (int)preamble.size();
    std::vector<int> detected_indices;

    if (srcLen < tapsLen) return detected_indices;

    // 1. フィルタ係数の準備（プリアンブルの共役かつ時間反転）
    std::vector<Ipp32fc> taps(tapsLen);
    for (int i = 0; i < tapsLen; ++i) {
        taps[i].re = preamble[tapsLen - 1 - i].real();
        taps[i].im = -preamble[tapsLen - 1 - i].imag();
    }

    // 2. FIRSR フィルタの初期化に必要なメモリサイズ計算
    int specSize, bufferSize;
    ippsFIRSRGetSize(tapsLen, ipp32fc, &specSize, &bufferSize);

    // 3. バッファの確保 (IPP専用アロケータを使用)
    IppsFIRSpec_32fc* pSpec = (IppsFIRSpec_32fc*)ippMalloc(specSize);
    Ipp8u* pBuffer = (Ipp8u*)ippMalloc(bufferSize);

    // 4. フィルタステートの初期化
    ippsFIRSRInit_32fc(taps.data(), tapsLen, ippAlgDirect, pSpec);

    // 5. フィルタリング実行 (相互相関)
    std::vector<Ipp32fc> dstCorr(srcLen);
    const Ipp32fc* pSrc = reinterpret_cast<const Ipp32fc*>(rx_signal.data());

    // ippsFIRSR_32fc は最新の oneAPI IPP で定義されている標準関数です
    ippsFIRSR_32fc(pSrc, dstCorr.data(), srcLen, pSpec, nullptr, nullptr, pBuffer);

    // 6. 検出処理
    for (int i = tapsLen - 1; i < srcLen; ++i) {
        float mag_sq = dstCorr[i].re * dstCorr[i].re + dstCorr[i].im * dstCorr[i].im;
        if (mag_sq > threshold) {
            detected_indices.push_back(i - (tapsLen - 1));
            i += tapsLen; // 重複検出回避
        }
    }

    // 7. 解放
    ippFree(pSpec);
    ippFree(pBuffer);

    return detected_indices;
}

int main() {
    ippInit(); // 必須

    std::vector<std::complex<float>> preamble(64, { 1.0f, -1.0f });
    std::vector<std::complex<float>> rx_data(1024, { 0.0f, 0.0f });

    // テストデータの挿入
    for (int i = 0; i < 64; ++i) rx_data[500 + i] = preamble[i];

    auto results = detect_preamble_oneapi(rx_data, preamble, 3000.0f);

    for (int idx : results) {
        std::cout << "Detected at index: " << idx << std::endl;
    }

    return 0;
}