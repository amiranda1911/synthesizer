use std::error::Error;
use std::io::{stdin, stdout, Write};

mod midi_manager;

use midi_manager::MidiManager;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Criar o gerenciador MIDI
    let mut manager = MidiManager::new()?;
    
    // Listar as portas disponíveis
    manager.list_ports();
    
    // Conectar à primeira porta disponível (índice 0)
    if !manager.ports.is_empty() {
        manager.connect_midi_port(0)?;
        println!("Conectado! Pressione Enter para sair.");
        
        // Manter o programa rodando para receber mensagens
        let mut input = String::new();
        std::io::stdin().read_line(&mut input)?;
    }
    
    Ok(())
}
