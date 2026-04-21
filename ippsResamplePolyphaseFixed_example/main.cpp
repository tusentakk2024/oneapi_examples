#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "IQResamplerIPP.h"

/**
 * @brief テスト関数：IQ信号リサンプリングの実行
 */
void test_iq_resampling() {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "     Intel IPP ippsResamplePolyphase_32fc Demo" << std::endl;
    std::cout << "     IQ Signal Resampling: 19.2 kHz -> 16 kHz" << std::endl;
    std::cout << std::string(60, '=') << "\n" << std::endl;
    
    const int inputRate = 19200;   // 19.2 kHz
    const int outputRate = 16000;  // 16 kHz
    const int durationMs = 200;    // 200 msのテスト信号
    
    // テスト信号パラメータ
    float testFreq1 = 1000.0f;   // 1 kHz
    float testFreq2 = 3000.0f;   // 3 kHz
    
    // テスト信号を生成
    int numInputSamples = (inputRate * durationMs) / 1000;
    std::vector<Ipp32fc> iq_signal(iNumInputSamples);
    
    std::cout << "Generating test signal..." << std::endl;
    std::cout << "  Duration: " << durationMs << " ms" << std::endl;
    std::cout << "  Frequencies: " << testFreq1 << " Hz, " << testFreq2 << " Hz" << std::endl;
    std::cout << "  Input samples: " << numInputSamples << std::endl << std::endl;
    
    // I成分とQ成分に異なる周波数を設定
    for (int i = 0; i < numInputSamples; i++) {
        float time = static_cast<float>(i) / inputRate;
        float phaseI = 2.0f * M_PI * testFreq1 * time;
        float phaseQ = 2.0f * M_PI * testFreq2 * time;
        
        iq_signal[i].re = cosf(phaseI);  // I成分 (1 kHz)
        iq_signal[i].im = sinf(phaseQ);  // Q成分 (3 kHz)
    }
    
    // リサンプラーを作成（品質80%）
    try {
        IQResamplerIPP resampler(inputRate, outputRate, 0.8f);
        
        // 統計情報を表示
        resampler.PrintStatistics();
        
        // リサンプリングを実行
        std::cout << "Executing resampling..." << std::endl;
        auto resampled = resampler.Resample(iq_signal);
        
        // 結果の検証
        int expectedSamples = (numInputSamples * outputRate) / inputRate;
        
        std::cout << "\nResampling Results:" << std::endl;
        std::cout << "  Expected output samples: " << expectedSamples << std::endl;
        std::cout << "  Actual output samples:   " << resampled.size() << std::endl;
        
        // 最初と最後のサンプルを表示
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "\nFirst 5 resampled IQ samples:" << std::endl;
        for (size_t i = 0; i < std::min(size_t(5), resampled.size()); i++) {
            std::cout << "  Sample " << i << ": I = " << std::setw(9) << resampled[i].re 
                      << ", Q = " << std::setw(9) << resampled[i].im << std::endl;
        }
        
        std::cout << "\nLast 5 resampled IQ samples:" << std::endl;
        if (resampled.size() >= 5) {
            for (size_t i = resampled.size() - 5; i < resampled.size(); i++) {
                std::cout << "  Sample " << i << ": I = " << std::setw(9) << resampled[i].re 
                          << ", Q = " << std::setw(9) << resampled[i].im << std::endl;
            }
        }
        
        // 信号パワーを計算
        float powerBefore = 0.0f, powerAfter = 0.0f;
        for (const auto& sample : iq_signal) {
            powerBefore += sample.re * sample.re + sample.im * sample.im;
        }
        powerBefore /= numInputSamples;
        
        for (const auto& sample : resampled) {
            powerAfter += sample.re * sample.re + sample.im * sample.im;
        }
        powerAfter /= resampled.size();
        
        std::cout << "\nSignal Power Analysis:" << std::endl;
        std::cout << "  Before resampling: " << std::setprecision(8) << 10 * log10(powerBefore) << " dB" << std::endl;
        std::cout << "  After resampling:  " << std::setprecision(8) << 10 * log10(powerAfter) << " dB" << std::endl;
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "Resampling completed successfully!" << std::endl;
        std::cout << std::string(60, '=') << "\n" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\nError during resampling: " << e.what() << std::endl;
        return;
    }
}

/**
 * @brief メイン関数
 */
int main() {
    try {
        // IPPライブラリの初期化
        ippInit();
        
        // テストを実行
        test_iq_resampling();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nFatal error: " << e.what() << std::endl;
        return 1;
    }
}
