import os
import sys
from bank import AudioBank

def get_all_wavs(folder_path):
    wav_files = []
    for root, _, files in os.walk(folder_path):
        for file in files:
            if file.endswith('.wav'):
                wav_files.append(os.path.join(root, file))
    return wav_files

def validate_wav(file_path):
    import wave
    try:
        with wave.open(file_path, 'rb') as wav:
            if wav.getsampwidth() != 2: 
                print(f"Warning: {file_path} is not 16-bit audio")
                return False
            if wav.getnchannels() != 2:
                print(f"Warning: {file_path} is not stereo")
                return False
            return True
    except Exception as e:
        print(f"Error validating {file_path}: {e}")
        return False

def create_audio_bank(input_folder: str, output_folder: str, bank_name: str, config_path: str) -> bool:
    try:
        os.makedirs(output_folder, exist_ok=True)
        
        bank = AudioBank(config_path)
        wav_files = get_all_wavs(input_folder)
        failed_files = []
        
        print(f"Found {len(wav_files)} WAV files in {input_folder}")
        
        for wav_file in wav_files:
            if not validate_wav(wav_file):
                failed_files.append(wav_file)
                continue
                
            if not bank.add_audio(wav_file):
                failed_files.append(wav_file)
        
        if failed_files:
            print(f"Warning: Failed to add {len(failed_files)} files to bank")
            
        audio_path = os.path.join(output_folder, bank_name)
        if not bank.write_bank(audio_path):
            print("Failed to write audio bank")
            return False
        
        print(f"Successfully created audio bank with {bank.get_audio_count()} files")
        if failed_files:
            print("Failed files:")
            for file in failed_files:
                print(f" - {os.path.basename(file)}")
        
        return True
        
    except Exception as e:
        print(f"Error creating audio bank: {str(e)}")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 create_bank.py [input_folder] [output_folder] [bank_name] [config_path]")
        print("Using defaults...")
        
    input_folder = sys.argv[1] if len(sys.argv) > 1 else "files"
    output_folder = sys.argv[2] if len(sys.argv) > 2 else "AudioBank"
    bank_name = sys.argv[3] if len(sys.argv) > 3 else "audio_bank.pab"
    config_path = sys.argv[4] if len(sys.argv) > 4 else "configs/gameAudioHeirarchy.json"
    
    if create_audio_bank(input_folder, output_folder, bank_name, config_path):
        print("Audio bank creation completed successfully")
    else:
        print("Audio bank creation failed")