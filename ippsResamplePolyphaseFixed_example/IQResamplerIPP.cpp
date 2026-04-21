#include "IQResamplerIPP.h"
#include <iostream>
#include <cmath>
#include <stdexcept>

int IQResamplerIPP::ComputeGcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

void IQResamplerIPP::InitializeIppContext() {
    // リサンプリング仕様を設定
    // m_fQualityが高いほどメモリ使用量が増加するが品質が向上
    int iBufferSize = 0;
    IppHintAlgorithm hintAlgorithm = (m_fQuality > 0.8f) ? ippHintAccurate : ippHintFast;
    
    // コンテキスト作成に必要なバッファサイズを取得
    IppStatus statusResult = ippsResamplePolyphaseFixedGetSize_32fc(
        m_iUpsampling, m_iDownsampling, 64, hintAlgorithm, &iBufferSize);
    
    if (statusResult != ippStsNoErr) {
        throw std::runtime_error("Failed to get IPP buffer size: " + 
                               std::string(ippGetStatusString(statusResult)));
    }
    
    // コンテキストを作成
    IppResamplePolyphaseFixed_32fc* pContext = nullptr;
    statusResult = ippsResamplePolyphaseFixedInit_32fc(
        m_iUpsampling, m_iDownsampling, 64, hintAlgorithm, &pContext);
    
    if (statusResult != ippStsNoErr) {
        throw std::runtime_error("Failed to initialize IPP context: " + 
                               std::string(ippGetStatusString(statusResult)));
    }
    
    m_pContext.reset(pContext);
}

IQResamplerIPP::IQResamplerIPP(int inRate, int outRate, float quality, int maxInLen)
    : m_iInputSampleRate(inRate), m_iOutputSampleRate(outRate), 
      m_fQuality(quality), m_iMaxInputLen(maxInLen),
      m_pContext(nullptr, ippsResamplePolyphaseFixedDeleteContext_32fc) {
    
    m_iGcdValue = ComputeGcd(inRate, outRate);
    m_iUpsampling = outRate / m_iGcdValue;   // アップサンプリング係数
    m_iDownsampling = inRate / m_iGcdValue;  // ダウンサンプリング係数
    
    // IPPコンテキストを作成
    InitializeIppContext();
    
    std::cout << "IQResamplerIPP initialized:" << std::endl;
    std::cout << "  Input rate: " << inRate << " Hz" << std::endl;
    std::cout << "  Output rate: " << outRate << " Hz" << std::endl;
    std::cout << "  Upsampling factor (L): " << m_iUpsampling << std::endl;
    std::cout << "  Downsampling factor (M): " << m_iDownsampling << std::endl;
    std::cout << "  Quality: " << quality << std::endl;
}

std::vector<Ipp32fc> IQResamplerIPP::Resample(const std::vector<Ipp32fc>& input) {
    if (input.empty()) {
        std::cerr << "Input signal is empty" << std::endl;
        return {};
    }
    
    int iInputLen = input.size();
    
    // 出力サイズを計算
    // output_len = ceil(input_len * output_rate / input_rate)
    int iOutputLen = (iInputLen * m_iOutputSampleRate + m_iInputSampleRate - 1) 
                     / m_iInputSampleRate;
    
    std::vector<Ipp32fc> output(iOutputLen);
    
    // IPP関数でリサンプリングを実行
    int iNumOutputSamples = 0;
    IppStatus statusResult = ippsResamplePolyphase_32fc(
        input.data(),
        iInputLen,
        output.data(),
        iOutputLen,
        1.0f,  // バリュースケール（利得）
        0.0f,  // サンプル係数
        &iNumOutputSamples,
        m_pContext.get());
    
    if (statusResult != ippStsNoErr) {
        throw std::runtime_error("IPP resampling failed: " + 
                               std::string(ippGetStatusString(statusResult)));
    }
    
    // 実際に出力されたサンプル数に合わせてベクタのサイズを調整
    output.resize(iNumOutputSamples);
    
    return output;
}

void IQResamplerIPP::PrintStatistics() const {
    std::cout << "\n=== IQ Resampler Statistics ===" << std::endl;
    std::cout << "Input sample rate:  " << m_iInputSampleRate << " Hz" << std::endl;
    std::cout << "Output sample rate: " << m_iOutputSampleRate << " Hz" << std::endl;
    std::cout << "Resampling ratio:   " << m_iOutputSampleRate << "/" 
              << m_iInputSampleRate << std::endl;
    std::cout << "L (upsampling):     " << m_iUpsampling << std::endl;
    std::cout << "M (downsampling):   " << m_iDownsampling << std::endl;
    std::cout << "Quality setting:    " << m_fQuality * 100.0f << "%" << std::endl;
    std::cout << "================================\n" << std::endl;
}
