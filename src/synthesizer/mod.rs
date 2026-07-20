mod voice;
use voice::Voice;

/// Orquestrador principal do sintetizador.
///
/// Responsabilidades:
/// - Gerenciar um pool de vozes (alocação/desalocação)
/// - Processar comandos de notas (note_on/note_off)
/// - Gerar blocos de áudio (soma de todas as vozes ativas)
/// - Controlar volume master e efeitos básicos
pub struct Synthesizer {
    voices: Vec<Voice>,
    sample_rate: f32,
    master_volume: f32,
    max_voices: usize,
}

impl Synthesizer {
    /// Cria um novo sintetizador.
    ///
    /// # Panics
    /// Em builds debug, entra em pânico se `sample_rate <= 0.0` ou `max_voices == 0`.
    pub fn new(sample_rate: f32, max_voices: usize) -> Self {
        debug_assert!(sample_rate > 0.0, "Sample rate must be greater than 0");
        debug_assert!(max_voices > 0, "Max voices must be greater than 0");

        let voices = (0..max_voices)
            .map(|_| Voice::new(sample_rate as u32))
            .collect();

        println!(
            "Synthesizer initialized with {max_voices} voices at {sample_rate} Hz"
        );

        Synthesizer {
            voices,
            sample_rate,
            master_volume: 0.8, // Volume inicial de 80%
            max_voices,
        }
    }

    /// Toca uma nota (Note On).
    ///
    /// Retorna `true` se conseguiu alocar uma voz, `false` se todas ocupadas
    /// ou a frequência for inválida.
    pub fn note_on(&mut self, frequency: f32, amplitude: f32) -> bool {
        if frequency <= 0.0 {
            eprintln!("Warning: Invalid frequency {frequency}");
            return false;
        }

        let amplitude = if !(0.0..=1.0).contains(&amplitude) {
            eprintln!("Warning: Invalid amplitude {amplitude}");
            amplitude.clamp(0.0, 1.0)
        } else {
            amplitude
        };

        let Some(free_voice_index) = self.find_free_voice() else {
            // Todas as vozes estão ocupadas - Voice Stealing
            eprintln!("Warning: No free voices available. Note {frequency}Hz ignored.");
            return false;
        };

        self.voices[free_voice_index].start(frequency, amplitude);

        println!("Note On: {frequency}Hz (Voice {free_voice_index})");

        true
    }

    /// Para uma nota (Note Off).
    ///
    /// Retorna `true` se encontrou e parou a nota, `false` caso contrário.
    pub fn note_off(&mut self, frequency: f32) -> bool {
        if frequency <= 0.0 {
            eprintln!("Warning: Invalid frequency {frequency}");
            return false;
        }

        let Some(voice_index) = self.find_voice_by_frequency(frequency) else {
            eprintln!("Warning: Note {frequency}Hz not found");
            return false;
        };

        self.voices[voice_index].stop();

        println!("Note Off: {frequency}Hz (Voice {voice_index})");

        true
    }

    /// Para todas as notas imediatamente (Panic).
    pub fn all_notes_off(&mut self) {
        for voice in &mut self.voices {
            if voice.is_active() {
                voice.stop();
            }
        }
        println!("All notes off");
    }

    /// Processa um bloco de áudio, escrevendo no `output_buffer`.
    pub fn process_block(&mut self, output_buffer: &mut [f32]) {
        for sample in output_buffer.iter_mut() {
            let mut sum = 0.0;

            for voice in &mut self.voices {
                if voice.is_active() {
                    sum += voice.tick();
                }
            }

            sum *= self.master_volume;
            *sample = Self::apply_clipping(sum);
        }
    }

    /// Define o volume master (é limitado ao intervalo `[0.0, 1.0]`).
    pub fn set_master_volume(&mut self, volume: f32) {
        self.master_volume = volume.clamp(0.0, 1.0);
        println!("Master volume set to {}", self.master_volume);
    }

    /// Obtém o volume master atual.
    pub fn master_volume(&self) -> f32 {
        self.master_volume
    }

    /// Obtém o número de vozes ativas no momento.
    pub fn active_voices(&self) -> usize {
        self.voices.iter().filter(|v| v.is_active()).count()
    }

    /// Obtém o número máximo de vozes.
    pub fn max_voices(&self) -> usize {
        self.max_voices
    }

    /// Encontra o índice de uma voz livre (IDLE).
    fn find_free_voice(&self) -> Option<usize> {
        self.voices.iter().position(|v| !v.is_active())
    }

    /// Encontra o índice de uma voz ativa com uma frequência específica.
    fn find_voice_by_frequency(&self, frequency: f32) -> Option<usize> {
        self.voices
            .iter()
            .position(|v| v.is_active() && (v.frequency() - frequency).abs() < 0.001)
    }

    /// Aplica clipping simples (hard clipping) para evitar distorção.
    fn apply_clipping(sample: f32) -> f32 {
        sample.clamp(-1.0, 1.0)
    }
}

impl Drop for Synthesizer {
    fn drop(&mut self) {
        self.all_notes_off();
        println!("Synthesizer destroyed");
    }
}