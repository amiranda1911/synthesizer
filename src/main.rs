use core::sync;
use std::sync::{Arc, Mutex};
use cpal::{Device, traits::{DeviceTrait, HostTrait, StreamTrait}};

mod synthesizer;
use synthesizer::Synthesizer;

fn main() {
    let msg = "Audio Device not found!";

    let host = cpal::default_host();
    let device = host
        .default_output_device()
        .expect(msg);

    let config = device.
        default_output_config()
        .expect(msg);

    let sample_rate = config.sample_rate().0 as f32;
    let channels = config.channels() as usize;

    println!("Dispositivo: {}", device.name().unwrap_or_default());
    println!("Sample rate: {sample_rate} Hz, canais: {channels}");

    // Cria o sintetizador com o sample rate real do dispositivo
    let synth = Arc::new(Mutex::new(Synthesizer::new(sample_rate, 16)));
    // Clona o Arc pra mover pro callback de áudio
    let synth_for_stream = Arc::clone(&synth);

    let stream = device
        .build_output_stream(
            &config.into(),
            move |data: &mut [f32], _info: &cpal::OutputCallbackInfo|{
                let num_frames = data.len() / channels;
                let mut mono_buffer = vec![0.0f32; num_frames];
                {
                    let mut synth = synth_for_stream
                        .lock()
                        .expect("Mutex for Synthesizer poisoned");

                    synth.process_block(&mut mono_buffer);
                }

                for(fame, &sample) in data.chunks_mut(channels).zip(mono_buffer.iter()){
                    for out in fame.iter_mut() {
                        *out = sample
                    }
                }
            },
            move |err| {
                std::eprintln!("Stream audio Error!: {err}")
            },
            None).expect("Failed to create stream");
            
            stream.play().expect("Failed to initialize stream");


            {
                let mut synth = synth.lock().expect("Mutex do Synthesizer envenenado");
                synth.note_on(440.0, 0.5);
                synth.note_on(86.0, 0.5);
                synth.note_on(432.0, 0.5);


            }
            std::thread::sleep(std::time::Duration::from_secs(2));
            {
                let mut synth = synth.lock().expect("Mutex do Synthesizer envenenado");
                synth.note_off(440.0);
            }

            // Segura um pouco mais pra não cortar o áudio abruptamente
            std::thread::sleep(std::time::Duration::from_secs(2));


}