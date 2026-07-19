use midir::{MidiInput, MidiInputPort, MidiInputConnection};

pub struct MidiManager {
    pub port_names: Vec<String>,
    pub ports: Vec<MidiInputPort>,
    pub connection: Option<MidiInputConnection<()>>,
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
                println!("Mensagem recebida ({}): {:?} (len = {})", stamp, message, message.len());
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
}