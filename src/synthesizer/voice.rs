// voice.rs

use std::f32::consts::TAU; // TAU = 2π (equivalente ao seu TWO_PI)

/// Representa uma única voz sintetizadora.
///
/// Responsabilidades:
/// - Gerar uma onda senoidal simples
/// - Manter seu próprio estado (ativa/inativa, fase, frequência, ganho)
/// - Avançar no tempo sample por sample
pub struct Voice {
    active: bool,
    frequency: f32,
    phase: f32,
    amplitude: f32,
    sample_rate: u32,
    phase_increment: f32,
}

impl Voice {
    /// Cria uma nova voz.
    ///
    /// # Panics
    /// Em builds debug, entra em pânico se `sample_rate` for 0.
    pub fn new(sample_rate: u32) -> Self {
        debug_assert!(sample_rate > 0, "Sample rate must be greater than 0");

        Voice {
            active: false,
            frequency: 0.0,
            phase: 0.0,
            amplitude: 0.0,
            sample_rate,
            phase_increment: 0.0,
        }
    }

    /// Inicia a voz com uma nova nota.
    ///
    /// # Panics
    /// Em builds debug, entra em pânico se `frequency <= 0.0` ou
    /// `amplitude` estiver fora de `[0.0, 1.0]`.
    pub fn start(&mut self, frequency: f32, amplitude: f32) {
        debug_assert!(frequency > 0.0, "Frequency must be positive");
        debug_assert!(
            (0.0..=1.0).contains(&amplitude),
            "Amplitude must be between 0 and 1"
        );

        self.active = true;
        self.frequency = frequency;
        self.amplitude = amplitude;
        self.phase = 0.0; // Reset de fase para evitar cliques (versão básica)

        self.phase_increment = frequency / self.sample_rate as f32;
    }

    /// Para a voz imediatamente.
    pub fn stop(&mut self) {
        self.active = false;
    }

    /// Gera o próximo sample da onda (entre -1.0 e 1.0).
    pub fn tick(&mut self) -> f32 {
        if !self.active {
            return 0.0;
        }

        self.phase += self.phase_increment;
        if self.phase >= 1.0 {
            self.phase -= 1.0;
        }

        (TAU * self.phase).sin() * self.amplitude
    }

    /// Verifica se a voz está ativa.
    pub fn is_active(&self) -> bool {
        self.active
    }

    /// Obtém a frequência atual da voz.
    pub fn frequency(&self) -> f32 {
        self.frequency
    }
}