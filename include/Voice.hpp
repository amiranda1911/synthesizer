#ifndef VOICE_HPP
#define VOICE_HPP

#include <cmath>
#include <cassert>


/**
 * Classe Voice - Representa uma única voz sintetizadora
 * 
 * Responsabilidades:
 * - Gerar uma onda senoidal simples
 * - Manter seu próprio estado (ativa/inativa, fase, frequência, ganho)
 * - Avançar no tempo sample por sample
 */
class Voice {
public:
    /**
     * Construtor
     * @param sampleRate Taxa de amostragem do sistema (ex: 44100 Hz)
     */
    Voice(int sampleRate);
    
    /**
     * Inicia a voz com uma nova nota
     * @param frequency Frequência em Hertz (ex: 440.0 para Lá)
     * @param amplitude Amplitude do som (0.0 a 1.0)
     */
    void start(float frequency, float amplitude);
    
    /**
     * Para a voz imediatamente
     */
    void stop();
    
    /**
     * Gera o próximo sample da onda
     * @return O próximo sample (float entre -1.0 e 1.0)
     */
    float tick();
    
    /**
     * Verifica se a voz está ativa
     * @return true se ativa, false se inativa
     */
    bool isActive() const;
    
    /**
     * Obtém a frequência atual da voz
     * @return Frequência em Hertz
     */
    float getFrequency() const;

private:
    // Estado da voz
    bool active;           // Está tocando?
    float frequency;       // Frequência em Hz
    float phase;           // Posição da onda (0.0 a 1.0)
    float amplitude;       // Ganho/Volume (0.0 a 1.0)
    int sampleRate;      // Taxa de amostragem
    float phaseIncrement; // phaseIncrement = frequency / sampleRate
    
    // Constante matemática
    static constexpr float TWO_PI = 6.283185307179586f;
};

#endif // VOICE_HPP