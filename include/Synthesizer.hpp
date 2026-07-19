#ifndef SYNTHESIZER_HPP
#define SYNTHESIZER_HPP

#include "Voice.hpp"
#include <vector>
#include <array>

/**
 * Classe Synthesizer - Orquestrador principal
 * 
 * Responsabilidades:
 * - Gerenciar um pool de vozes (alocação/desalocação)
 * - Processar comandos de notas (noteOn/noteOff)
 * - Gerar blocos de áudio (soma de todas as vozes ativas)
 * - Controlar volume master e efeitos básicos
 */
class Synthesizer {
public:
    /**
     * Construtor
     * @param sampleRate Taxa de amostragem (ex: 44100)
     * @param maxVoices Número máximo de vozes polifônicas (ex: 16)
     */
    Synthesizer(float sampleRate, int maxVoices = 16);
    
    /**
     * Destrutor
     */
    ~Synthesizer();
    
    /**
     * Toca uma nota (Nota On)
     * @param frequency Frequência em Hertz
     * @param amplitude Volume da nota (0.0 a 1.0)
     * @return true se conseguiu alocar uma voz, false se todas ocupadas
     */
    bool noteOn(float frequency, float amplitude);
    
    /**
     * Para uma nota (Nota Off)
     * @param frequency Frequência da nota a ser parada
     * @return true se encontrou e parou a nota, false se não encontrou
     */
    bool noteOff(float frequency);
    
    /**
     * Para todas as notas imediatamente (Panic)
     */
    void allNotesOff();
    
    /**
     * Processa um bloco de áudio
     * @param outputBuffer Buffer de saída (já alocado)
     * @param numSamples Número de samples a processar
     */
    void processBlock(float* outputBuffer, int numSamples);
    
    /**
     * Define o volume master
     * @param volume Volume (0.0 a 1.0)
     */
    void setMasterVolume(float volume);
    
    /**
     * Obtém o volume master atual
     * @return Volume atual
     */
    float getMasterVolume() const;
    
    /**
     * Obtém o número de vozes ativas no momento
     * @return Quantidade de vozes ativas
     */
    int getActiveVoices() const;
    
    /**
     * Obtém o número máximo de vozes
     * @return Capacidade máxima
     */
    int getMaxVoices() const;

private:
    // Pool de vozes
    std::vector<Voice> voices;  // Usando vector para flexibilidade
    
    // Configurações
    float sampleRate;
    float masterVolume;
    int maxVoices;
    
    // Métodos auxiliares
    /**
     * Encontra uma voz livre (IDLE)
     * @return Índice da voz livre, ou -1 se nenhuma disponível
     */
    int findFreeVoice() const;
    
    /**
     * Encontra uma voz ativa com uma frequência específica
     * @param frequency Frequência a procurar
     * @return Índice da voz encontrada, ou -1 se não encontrada
     */
    int findVoiceByFrequency(float frequency) const;
    
    /**
     * Aplica clipping simples para evitar distorção
     * @param sample Sample a ser limitado
     * @return Sample limitado entre -1.0 e 1.0
     */
    float applyClipping(float sample) const;
};

#endif // SYNTHESIZER_HPP