use midir::{MidiInput, MidiInputPort, MidiInputConnection};

pub struct MidiManager {
    pub port_names: Vec<String>,
    pub ports: Vec<MidiInputPort>,
    pub connection: Option<MidiInputConnection<()>>,
}

// Estrutura para armazenar informações da nota
#[derive(Debug, Clone)]
pub struct MidiNote {
    pub channel: u8,
    pub note: u8,
    pub velocity: u8,
    pub is_note_on: bool,
}

impl MidiManager {
    pub fn new() -> Result<Self, Box<dyn std::error::Error>> {
        // Criar uma instância do MidiInput
        let midi_in = MidiInput::new("Midir Reading Input")?;
        
        // Obter as portas disponíveis
        let ports = midi_in.ports();
        
        // Verificar se há portas disponíveis
        if ports.is_empty() {
            return Err("Nenhum dispositivo de entrada MIDI encontrado.".into());
        }
        
        // Coletar os nomes das portas
        let mut port_names = Vec::new();
        for port in &ports {
            let name = midi_in.port_name(port)?;
            port_names.push(name);
        }
        
        Ok(MidiManager { 
            port_names,
            ports,
            connection: None,
        })
    }

    pub fn connect_midi_port(&mut self, port_index: usize) -> Result<(), Box<dyn std::error::Error>> {
        // Verificar se o índice da porta é válido
        if port_index >= self.ports.len() {
            return Err(format!("Índice da porta {} fora do intervalo (máximo: {})", 
                port_index, self.ports.len() - 1).into());
        }
        
        // Criar uma nova instância do MidiInput para a conexão
        let midi_in = MidiInput::new("Midir Reading Input")?;
        let in_port = &self.ports[port_index];
        
        println!("Conectando ao dispositivo de entrada MIDI: {}", self.port_names[port_index]);

        // Estabelecer a conexão
        let conn = midi_in.connect(
            in_port,
            "midir-read-input",
            move |stamp, message, _| {
                if let Some(note) = Self::decode_midi_message(message) {
                    // Exibir informações da nota
                    // ==========================================
                    // 1. ACESSANDO OS VALORES INDIVIDUALMENTE
                    // ==========================================
                    
                    // Número da nota (0-127)
                    let note_number = note.note;
                    println!("Número da nota: {}", note_number);
                    
                    // Canal MIDI (0-15, onde 0 = Canal 1)
                    let channel = note.channel;
                    println!("Canal: {} (MIDI Canal {})", channel, channel + 1);
                    
                    // Velocity (intensidade)
                    let velocity = note.velocity;
                    println!("Velocity: {}", velocity);
                    
                    // Se é Note On ou Note Off
                    let is_on = note.is_note_on;
                    println!("É Note On? {}", is_on);
                    
                    // ==========================================
                    // 2. CONVERTENDO PARA NOME DA NOTA
                    // ==========================================
                    
                    let note_name = Self::note_number_to_name(note.note);
                    println!("Nome da nota: {}", note_name);
                    
                    // ==========================================
                    // 3. CALCULANDO A FREQUÊNCIA
                    // ==========================================
                    
                    let frequency = 440.0 * 2.0f32.powf((note.note as f32 - 69.0) / 12.0);
                    println!("Frequência: {:.2} Hz", frequency);
                            
                    // Aqui você pode processar a nota como quiser
                    // Por exemplo, tocar um som, salvar em um arquivo, etc.
                }
                
            },
            (),
        )?;
        
        // Armazenar a conexão para mantê-la viva
        self.connection = Some(conn);
        Ok(())
    }
    
    pub fn list_ports(&self) {
        println!("Portas de entrada MIDI disponíveis:");
        for (i, name) in self.port_names.iter().enumerate() {
            println!("  {}: {}", i, name);
        }
    }


    pub fn note_number_to_name(note: u8) -> String {
        let note_names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
        let octave = (note / 12) as i8 - 1;
        let note_name = note_names[(note % 12) as usize];
        let position = note % 12; // Posição na oitava (0-11)
        
        format!("{}{} (grau {})", note_name, octave, position)
    }

    // Função para decodificar mensagens MIDI
    pub fn decode_midi_message(message: &[u8]) -> Option<MidiNote> {
        if message.is_empty() {
            return None;
        }
        
        let status = message[0];
        let channel = status & 0x0F; // Os 4 bits menos significativos são o canal
        let message_type = status & 0xF0; // Os 4 bits mais significativos são o tipo
        
        match message_type {
            0x80 => { // Note Off
                if message.len() >= 3 {
                    Some(MidiNote {
                        channel,
                        note: message[1],
                        velocity: message[2],
                        is_note_on: false,
                    })
                } else {
                    None
                }
            }
            0x90 => { // Note On
                if message.len() >= 3 {
                    let velocity = message[2];
                    // Se velocity for 0, é na verdade um Note Off
                    let is_note_on = velocity > 0;
                    Some(MidiNote {
                        channel,
                        note: message[1],
                        velocity,
                        is_note_on,
                    })
                } else {
                    None
                }
            }
            0xA0 => { // Aftertouch (polifônico)
                // Não processamos por enquanto
                None
            }
            0xB0 => { // Control Change
                // Não processamos por enquanto
                None
            }
            0xC0 => { // Program Change
                // Não processamos por enquanto
                None
            }
            0xD0 => { // Channel Pressure
                // Não processamos por enquanto
                None
            }
            0xE0 => { // Pitch Bend
                // Não processamos por enquanto
                None
            }
            _ => {
                // Mensagem desconhecida
                None
            }
        }
    }

}