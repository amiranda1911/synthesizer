#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <portaudio.h>

#include "Synthesizer.hpp"

// ============================================================
// CONFIGURAÇÕES DE ÁUDIO
// ============================================================
const int SAMPLE_RATE = 44100;      // Taxa de amostragem padrão
const int FRAMES_PER_BUFFER = 256;  // Tamanho do buffer (menor = menor latência)
const int NUM_CHANNELS = 1;         // Mono (simplifica)

// ============================================================
// CALLBACK DO PORTAUDIO
// ============================================================
/**
 * Esta função é chamada pelo PortAudio sempre que ele precisa de 
 * mais dados de áudio para tocar.
 * 
 * @param inputBuffer Buffer de entrada (não usado, é NULL)
 * @param outputBuffer Buffer de saída (onde escrevemos o áudio)
 * @param framesPerBuffer Número de frames a processar
 * @param timeInfo Informações de tempo (não usado)
 * @param statusFlags Flags de status (não usado)
 * @param userData Ponteiro para o nosso Synthesizer
 * @return paContinue (0) para continuar, paComplete para parar
 */
static int paCallback(const void* inputBuffer, 
                      void* outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void* userData) {
    
    // Cast do userData para nosso Synthesizer
    Synthesizer* synth = static_cast<Synthesizer*>(userData);
    
    // Cast do outputBuffer para float*
    float* out = static_cast<float*>(outputBuffer);
    
    // Processa o áudio com nosso sintetizador
    // Isso preencherá 'out' com os samples gerados
    synth->processBlock(out, static_cast<int>(framesPerBuffer));
    
    // Retorna paContinue para manter a stream ativa
    return paContinue;
}

// ============================================================
// FUNÇÃO PARA TOCAR UMA SEQUÊNCIA DE NOTAS (EM THREAD SEPARADA)
// ============================================================
void playDemoSequence(Synthesizer& synth) {
    // Pequeno delay para garantir que o áudio já está rodando
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "\n=== TOCANDO SEQUÊNCIA DE NOTAS ===\n" << std::endl;
    
    // Notas da escala de Dó maior (frequências em Hz)
    struct Nota {
        float frequency;
        float duration;  // em segundos
        float amplitude;
    };
    
    std::vector<Nota> melodia = {
        // Escala ascendente
        {261.63f, 0.3f, 0.5f},  // Dó 4
        {293.66f, 0.3f, 0.5f},  // Ré 4
        {329.63f, 0.3f, 0.5f},  // Mi 4
        {349.23f, 0.3f, 0.5f},  // Fá 4
        {392.00f, 0.3f, 0.5f},  // Sol 4
        {440.00f, 0.3f, 0.5f},  // Lá 4
        {493.88f, 0.3f, 0.5f},  // Si 4
        {523.25f, 0.4f, 0.6f},  // Dó 5 (última nota mais longa)
        
        // Pequena pausa
        {0.0f, 0.2f, 0.0f},
        
        // Acorde de Dó maior (três notas juntas)
        {261.63f, 1.0f, 0.3f},  // Dó
        {329.63f, 1.0f, 0.3f},  // Mi
        {392.00f, 1.0f, 0.3f},  // Sol
    };
    
    for (const auto& nota : melodia) {
        if (nota.frequency == 0.0f) {
            // Pausa
            std::cout << "Pausa...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(
                static_cast<int>(nota.duration * 1000)
            ));
            continue;
        }
        
        // Toca a nota
        std::cout << "Tocando: " << nota.frequency << "Hz ("
                  << nota.duration << "s)" << std::endl;
        
        synth.noteOn(nota.frequency, nota.amplitude);
        
        // Aguarda a duração da nota
        std::this_thread::sleep_for(std::chrono::milliseconds(
            static_cast<int>(nota.duration * 1000)
        ));
        
        // Para a nota
        synth.noteOff(nota.frequency);
        
        // Pequeno espaço entre notas
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::cout << "\n=== FIM DA SEQUÊNCIA ===\n" << std::endl;
}

// ============================================================
// FUNÇÃO PARA TOCAR UMA NOTA CONTÍNUA (PARA TESTE)
// ============================================================
void playContinuousNote(Synthesizer& synth, float frequency, float amplitude) {
    std::cout << "\n=== TOCANDO NOTA CONTÍNUA: " << frequency << "Hz ===\n" << std::endl;
    synth.noteOn(frequency, amplitude);
    
    std::cout << "Nota tocando. Pressione ENTER para parar..." << std::endl;
    std::cin.get();  // Aguarda o usuário pressionar Enter
    
    synth.noteOff(frequency);
    std::cout << "Nota parada.\n" << std::endl;
}

// ============================================================
// FUNÇÃO PARA INTERAGIR COM O USUÁRIO (MODO MANUAL)
// ============================================================
void interactiveMode(Synthesizer& synth) {
    std::cout << "\n=== MODO INTERATIVO ===\n" << std::endl;
    std::cout << "Comandos:\n";
    std::cout << "  n <freq> <amp>  - Toca uma nota (ex: n 440 0.5)\n";
    std::cout << "  o <freq>        - Para uma nota (ex: o 440)\n";
    std::cout << "  a               - Para todas as notas\n";
    std::cout << "  s               - Mostra vozes ativas\n";
    std::cout << "  v <vol>         - Ajusta volume master (0-1)\n";
    std::cout << "  d               - Toca sequência demo\n";
    std::cout << "  q               - Sair\n";
    std::cout << "\n> " << std::flush;
    
    std::string comando;
    while (std::cin >> comando) {
        if (comando == "q" || comando == "quit") {
            break;
        }
        else if (comando == "n" || comando == "note") {
            float freq, amp;
            std::cin >> freq >> amp;
            synth.noteOn(freq, amp);
            std::cout << "Tocando " << freq << "Hz com amplitude " << amp << std::endl;
        }
        else if (comando == "o" || comando == "off") {
            float freq;
            std::cin >> freq;
            synth.noteOff(freq);
            std::cout << "Parando " << freq << "Hz" << std::endl;
        }
        else if (comando == "a" || comando == "all") {
            synth.allNotesOff();
            std::cout << "Todas as notas paradas" << std::endl;
        }
        else if (comando == "s" || comando == "status") {
            std::cout << "Vozes ativas: " << synth.getActiveVoices() 
                      << "/" << synth.getMaxVoices() << std::endl;
        }
        else if (comando == "v" || comando == "volume") {
            float vol;
            std::cin >> vol;
            synth.setMasterVolume(vol);
        }
        else if (comando == "d" || comando == "demo") {
            playDemoSequence(synth);
        }
        else {
            std::cout << "Comando desconhecido. Use n, o, a, s, v, d ou q" << std::endl;
        }
        std::cout << "\n> " << std::flush;
    }
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   SIMPLES SINTETIZADOR C++ + PORTAUDIO" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // ============================================================
    // 1. INICIALIZA O PORTAUDIO
    // ============================================================
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "ERRO ao inicializar PortAudio: " 
                  << Pa_GetErrorText(err) << std::endl;
        return 1;
    }
    
    std::cout << "PortAudio inicializado com sucesso!" << std::endl;
    
    // ============================================================
    // 2. CRIA O SINTETIZADOR
    // ============================================================
    const int MAX_VOICES = 16;
    Synthesizer synth(static_cast<float>(SAMPLE_RATE), MAX_VOICES);
    synth.setMasterVolume(0.7f);
    
    std::cout << "Sintetizador criado com " << MAX_VOICES << " vozes" << std::endl;
    std::cout << "Sample Rate: " << SAMPLE_RATE << " Hz" << std::endl;
    std::cout << "Buffer Size: " << FRAMES_PER_BUFFER << " frames" << std::endl;
    std::cout << std::endl;
    
    // ============================================================
    // 3. ABRE A STREAM DE ÁUDIO
    // ============================================================
    PaStream* stream;
    
    // Configuração da stream
    err = Pa_OpenDefaultStream(
        &stream,
        0,                          // Sem entrada
        NUM_CHANNELS,               // Número de canais de saída (1 = mono)
        paFloat32,                  // Formato dos samples (32-bit float)
        SAMPLE_RATE,                // Taxa de amostragem
        FRAMES_PER_BUFFER,          // Frames por buffer
        paCallback,                 // Função de callback
        &synth                      // Dados do usuário (Synthesizer)
    );
    
    if (err != paNoError) {
        std::cerr << "ERRO ao abrir stream de áudio: " 
                  << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return 1;
    }
    
    std::cout << "Stream de áudio aberta com sucesso!" << std::endl;
    
    // ============================================================
    // 4. INICIA A STREAM (O ÁUDIO COMEÇA A TOCAR)
    // ============================================================
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "ERRO ao iniciar stream de áudio: " 
                  << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }
    
    std::cout << "Stream de áudio iniciada!" << std::endl;
    std::cout << "Áudio está fluindo para a placa de som.\n" << std::endl;
    
    // ============================================================
    // 5. MENU PRINCIPAL
    // ============================================================
    std::cout << "ESCOLHA UMA OPÇÃO:\n" << std::endl;
    std::cout << "  1 - Tocar uma nota contínua (440Hz)" << std::endl;
    std::cout << "  2 - Tocar sequência demo (escala + acorde)" << std::endl;
    std::cout << "  3 - Modo interativo (comandos manuais)" << std::endl;
    std::cout << "  4 - Sair" << std::endl;
    std::cout << "\nOpção: " << std::flush;
    
    int opcao;
    std::cin >> opcao;
    
    switch (opcao) {
        case 1:
            playContinuousNote(synth, 440.0f, 0.5f);
            break;
            
        case 2:
            playDemoSequence(synth);
            break;
            
        case 3:
            interactiveMode(synth);
            break;
            
        case 4:
        default:
            std::cout << "Saindo..." << std::endl;
            break;
    }
    
    // ============================================================
    // 6. LIMPEZA E FINALIZAÇÃO
    // ============================================================
    std::cout << "\nParando áudio..." << std::endl;
    
    // Para todas as notas
    synth.allNotesOff();
    
    // Pequeno delay para o áudio finalizar
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Para a stream
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        std::cerr << "ERRO ao parar stream: " << Pa_GetErrorText(err) << std::endl;
    }
    
    // Fecha a stream
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        std::cerr << "ERRO ao fechar stream: " << Pa_GetErrorText(err) << std::endl;
    }
    
    // Finaliza o PortAudio
    Pa_Terminate();
    
    std::cout << "PortAudio finalizado." << std::endl;
    std::cout << "Sintetizador encerrado." << std::endl;
    
    return 0;
}