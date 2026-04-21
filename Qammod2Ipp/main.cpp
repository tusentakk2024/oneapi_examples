#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "Qammod2Ipp.h"

/**
 * @brief テスト関数：QAM変調の実行
 */
void test_qam_modulation() {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "     Intel IPP QAM Modulator Demo" << std::endl;
    std::cout << "     Equivalent to MATLAB qammod function" << std::endl;
    std::cout << std::string(60, '=') << "\n" << std::endl;

    const int M = 16;  // 16-QAM
    const int numInputs = 10;  // 入力信号数

    // 入力信号を生成 (0からM-1)
    std::vector<int> inputs(numInputs);
    std::cout << "Generating test inputs..." << std::endl;
    std::cout << "  Modulation order (M): " << M << std::endl;
    std::cout << "  Number of inputs: " << numInputs << std::endl;
    std::cout << "  Inputs: ";
    for (int i = 0; i < numInputs; ++i) {
        inputs[i] = i % M;  // 0,1,2,...,M-1,0,1,...
        std::cout << inputs[i] << " ";
    }
    std::cout << std::endl << std::endl;

    // QAM変調器を作成
    try {
        // QAM変調を実行 (MATLAB qammodと同等)
        std::cout << "Executing QAM modulation..." << std::endl;
        auto modulated = Qammod2Ipp::Exec(inputs, M);

        // 結果を表示
        std::cout << "\nModulation Results:" << std::endl;
        std::cout << "  Input inputs -> Output IQ samples" << std::endl;
        std::cout << std::fixed << std::setprecision(6);
        for (size_t i = 0; i < modulated.size(); ++i) {
            std::cout << "  Symbol " << inputs[i] << " -> I = " << std::setw(9) << modulated[i].re
                      << ", Q = " << std::setw(9) << modulated[i].im << std::endl;
        }

        // 平均電力を計算
        float avgPower = 0.0f;
        for (const auto& sample : modulated) {
            avgPower += sample.re * sample.re + sample.im * sample.im;
        }
        avgPower /= modulated.size();

        std::cout << "\nSignal Power Analysis:" << std::endl;
        std::cout << "  Average power: " << std::setprecision(8) << avgPower << " (should be ~1.0)" << std::endl;
        std::cout << "  Average power (dB): " << 10 * log10(avgPower) << " dB" << std::endl;

        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "QAM modulation completed successfully!" << std::endl;
        std::cout << std::string(60, '=') << "\n" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "\nError during QAM modulation: " << e.what() << std::endl;
        return;
    }
}

/**
 * @brief メイン関数
 */
int main() {
    try {
        // テストを実行
        test_qam_modulation();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nFatal error: " << e.what() << std::endl;
        return 1;
    }
}