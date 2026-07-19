#include "Voice.hpp"

Voice::Voice(int sampleRate) 
    : active(false)
    , frequency(0.0f)
    , phase(0.0f)
    , amplitude(0.0f)
    , sampleRate(sampleRate) 
{
    // Validação básica da taxa de amostragem
    assert(sampleRate > 0.0 && "Sample rate must be greater than 0");
}

void Voice::start(float frequency, float amplitude){
    // Validação básica, nao aceita frequencia e amplitude zerada
    assert(frequency > 0.0f && "Frequency must be positive");
    assert(amplitude >= 0.0f && amplitude <= 1.0f && "Amplitude must be between 0 and 1");

    this->active = true;
    this->frequency = frequency;
    this->amplitude = amplitude;
    this->phase = 0.0f; // Reset fase para evitar cliques (no básico)

    // Nota: Se quisermos evitar cliques, deveríamos manter a fase contínua
    // Mas para simplicidade, resetamos para zero


    // Calcula o incremento de fase por sample
    // phaseIncrement = frequency / sampleRate
    // Isso representa quantos ciclos completamos por sample
    this->phaseIncrement = frequency / sampleRate;

}

void Voice::stop() {
    this->active = false;
    // Poderia também fazer um fade out aqui, mas no básico paramos bruscamente
}

// Gera o próximo sample
float Voice::tick() {
    // Se inativa, retorna silêncio
    if (!active) {
        return 0.0f;
    }

    // Avança a fase
    this->phase += this->phaseIncrement;
    
    // Mantém a fase no intervalo [0.0, 1.0)
    // Se phase >= 1.0, subtrai 1.0
    if (phase >= 1.0f) {
        phase -= 1.0f;
    }

    // Calcula a onda senoidal
    // MULTIPLICAMOS POR TWO_PI para converter 0-1 para 0-2PI
    float sample = std::sin( TWO_PI * phase);

     // Aplica a amplitude
    sample *= amplitude;
    
    return sample;
}

// Verifica se a voz está ativa
bool Voice::isActive() const {
    return active;
}

// Obtém a frequência atual
float Voice::getFrequency() const {
    return frequency;
}