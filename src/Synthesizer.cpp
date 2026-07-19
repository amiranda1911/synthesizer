#include "Synthesizer.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>

// Construtor
Synthesizer::Synthesizer(float sampleRate, int maxVoices)
    : sampleRate(sampleRate)
    , masterVolume(0.8f)  // Volume inicial de 80%
    , maxVoices(maxVoices) {
    
    // Validações
    assert(sampleRate > 0.0f && "Sample rate must be greater than 0");
    assert(maxVoices > 0 && "Max voices must be greater than 0");
    
    // Cria o pool de vozes
    voices.reserve(maxVoices);
    for (int i = 0; i < maxVoices; ++i) {
        voices.emplace_back(sampleRate);  // Cria cada voz com a mesma sample rate
    }
    
    std::cout << "Synthesizer initialized with " << maxVoices << " voices at "
              << sampleRate << " Hz" << std::endl;
}


// Para todas as notas
void Synthesizer::allNotesOff() {
    for (auto& voice : voices) {
        if (voice.isActive()) {
            voice.stop();
        }
    }
    std::cout << "All notes off" << std::endl;
}

// Destrutor
Synthesizer::~Synthesizer() {
    allNotesOff();
    std::cout << "Synthesizer destroyed" << std::endl;
}

// Processa um bloco de áudio
void Synthesizer::processBlock(float* outputBuffer, int numSamples) {
    // Zera o buffer de saída
    std::fill(outputBuffer, outputBuffer + numSamples, 0.0f);
    
    // Para cada sample do bloco
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
        float sum = 0.0f;
        
        // Soma todas as vozes ativas
        for (auto& voice : voices) {
            if (voice.isActive()) {
                sum += voice.tick();
            }
        }
        
        // Aplica volume master e clipping
        sum *= masterVolume;
        sum = applyClipping(sum);
        
        // Escreve no buffer
        outputBuffer[sampleIndex] = sum;
    }
}

// Define o volume master
void Synthesizer::setMasterVolume(float volume) {
    masterVolume = std::max(0.0f, std::min(1.0f, volume));
    std::cout << "Master volume set to " << masterVolume << std::endl;
}

// Obtém o volume master
float Synthesizer::getMasterVolume() const {
    return masterVolume;
}

// Obtém o número de vozes ativas
int Synthesizer::getActiveVoices() const {
    int active = 0;
    for (const auto& voice : voices) {
        if (voice.isActive()) {
            ++active;
        }
    }
    return active;
}

// Obtém o número máximo de vozes
int Synthesizer::getMaxVoices() const {
    return maxVoices;
}

// Encontra uma voz livre
int Synthesizer::findFreeVoice() const {
    for (int i = 0; i < static_cast<int>(voices.size()); ++i) {
        if (!voices[i].isActive()) {
            return i;
        }
    }
    return -1;  // Nenhuma voz livre
}

// Encontra uma voz por frequência
int Synthesizer::findVoiceByFrequency(float frequency) const {
    for (int i = 0; i < static_cast<int>(voices.size()); ++i) {
        if (voices[i].isActive() && 
            std::abs(voices[i].getFrequency() - frequency) < 0.001f) {
            return i;
        }
    }
    return -1;  // Não encontrado
}

// Aplica clipping simples
float Synthesizer::applyClipping(float sample) const {
    // Clipping suave (tanh) ou hard clipping
    // Aqui usamos hard clipping simples
    if (sample > 1.0f) return 1.0f;
    if (sample < -1.0f) return -1.0f;
    return sample;
}


bool Synthesizer::noteOn(float frequency, float amplitude) {
    // Validações
    if (frequency <= 0.0f) {
        std::cerr << "Warning: Invalid frequency " << frequency << std::endl;
        return false;
    }
    
    if (amplitude < 0.0f || amplitude > 1.0f) {
        std::cerr << "Warning: Invalid amplitude " << amplitude << std::endl;
        amplitude = std::max(0.0f, std::min(1.0f, amplitude));  // Clamp
    }
    
    // Procura uma voz livre
    int freeVoiceIndex = findFreeVoice();
    
    if (freeVoiceIndex == -1) {
        // Todas as vozes estão ocupadas - Voice Stealing
        std::cerr << "Warning: No free voices available. Note " 
                  << frequency << "Hz ignored." << std::endl;
        return false;
    }
    
    // Ativa a voz encontrada
    voices[freeVoiceIndex].start(frequency, amplitude);
    
    // Debug
    std::cout << "Note On: " << frequency << "Hz (Voice " 
              << freeVoiceIndex << ")" << std::endl;
    
    return true;
}

// Para uma nota
bool Synthesizer::noteOff(float frequency) {
    if (frequency <= 0.0f) {
        std::cerr << "Warning: Invalid frequency " << frequency << std::endl;
        return false;
    }
    
    // Procura a voz tocando esta frequência
    int voiceIndex = findVoiceByFrequency(frequency);
    
    if (voiceIndex == -1) {
        std::cerr << "Warning: Note " << frequency << "Hz not found" << std::endl;
        return false;
    }
    
    // Para a voz encontrada
    voices[voiceIndex].stop();
    
    // Debug
    std::cout << "Note Off: " << frequency << "Hz (Voice " 
              << voiceIndex << ")" << std::endl;
    
    return true;
}