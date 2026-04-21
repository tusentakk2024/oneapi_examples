#ifndef IQ_RESAMPLER_IPP_H
#define IQ_RESAMPLER_IPP_H

#include <vector>
#include <ipp.h>
#include <memory>

/**
 * @brief Intel IPP ippsResamplePolyphase_32fcを使用したIQ信号リサンプラー
 * 
 * 19.2 kHzから16 kHzへの効率的なリサンプリングを実装
 * ippsResamplePolyphase_32fcはPolyphaseフィルタアーキテクチャを使用し、
 * 複素数32ビット浮動小数点数に最適化されている
 */
class IQResamplerIPP {
private:
    int m_iInputSampleRate;      // 入力サンプリングレート (Hz)
    int m_iOutputSampleRate;     // 出力サンプリングレート (Hz)
    int m_iGcdValue;             // 最大公約数
    int m_iUpsampling;           // アップサンプリング係数（output / GCD）
    int m_iDownsampling;         // ダウンサンプリング係数（input / GCD）
    float m_fQuality;            // リサンプリング品質（0-1）
    
    // IPPリサンプリングコンテキスト
    std::unique_ptr<void, decltype(&ippsResamplePolyphaseFixedDeleteContext_32fc)> m_pContext;
    int m_iMaxInputLen;          // 最大入力サンプル数
    
    // ユークリッド互除法でGCDを計算
    int ComputeGcd(int a, int b);
    
    // IPPコンテキストを初期化
    void InitializeIppContext();
    
public:
    /**
     * @brief コンストラクタ
     * @param inRate 入力サンプリングレート (Hz)
     * @param outRate 出力サンプリングレート (Hz)
     * @param quality リサンプリング品質 (0.0-1.0, デフォルト0.5)
     * @param maxInLen 最大入力サンプル数（バッファサイズ計算用）
     */
    IQResamplerIPP(int inRate, int outRate, float quality = 0.5f, int maxInLen = 4096);
    
    /**
     * @brief デストラクタ
     */
    ~IQResamplerIPP() = default;
    
    /**
     * @brief IQ信号をリサンプリング
     * @param input 入力IQ信号（Ipp32fc型）
     * @return リサンプリング後の出力IQ信号
     */
    std::vector<Ipp32fc> Resample(const std::vector<Ipp32fc>& input);
    
    /**
     * @brief リサンプラーの統計情報を表示
     */
    void PrintStatistics() const;
    
    /**
     * @brief 入力サンプリングレートを取得
     */
    int GetInputSampleRate() const { return m_iInputSampleRate; }
    
    /**
     * @brief 出力サンプリングレートを取得
     */
    int GetOutputSampleRate() const { return m_iOutputSampleRate; }
    
    /**
     * @brief アップサンプリング係数を取得
     */
    int GetUpsamplingFactor() const { return m_iUpsampling; }
    
    /**
     * @brief ダウンサンプリング係数を取得
     */
    int GetDownsamplingFactor() const { return m_iDownsampling; }
};

#endif // IQ_RESAMPLER_IPP_H
