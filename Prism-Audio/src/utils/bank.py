import wave
import struct
import os
import json
from typing import Dict, List, Tuple, Optional

class AudioBank:
    HEADER_MAGIC = b'PRSMAUBNK'  
    HEADER_VERSION = 1
    
    def __init__(self, config_path: str = "configs/gameAudioHeirarchy.json"):
        self.audio_files = {}  # Contains the id:file mappings
        self.audio_data = {}   # Contains the actual audio data
        self.config = self._load_config(config_path)
        self._generate_file_mapping()

    def _load_config(self, config_path: str) -> Dict:
        with open(config_path, 'r') as f:
            return json.load(f)

    def _generate_file_mapping(self):
        for cat_id, category in self.config.items():
            for subcat_id, subcategory in category["subcategories"].items():
                for sound_id, sound in subcategory["sounds"].items():
                    identifier = f"{cat_id}.{subcat_id}.{sound_id}"
                    self.audio_files[identifier] = {
                        "file": sound["file"],
                        "loopStart": sound.get("loopStart", 0),
                        "loopEnd": sound.get("loopEnd", 0),
                        "loopCount": sound.get("loopCount", 0)
                    }

    def add_audio(self, wave_file: str) -> bool:
        file_name = os.path.basename(wave_file)
        audio_id = None
        file_info = None
        for id, info in self.audio_files.items():
            if info["file"] == file_name:
                audio_id = id
                file_info = info
                break
        
        if not audio_id:
            print(f"Warning: No matching ID found for file {file_name}")
            return False

        try:
            with wave.open(wave_file, 'rb') as wav:
                if wav.getnchannels() != 2:
                    print(f"Warning: {file_name} is not stereo, skipping")
                    return False
                
                params = wav.getparams()
                frames = wav.readframes(params.nframes)
                
                parts = audio_id.split('.')
                category = int(parts[0])
                subcategory = int(parts[1])
                index = int(parts[2])
                
                loop_start = 0
                loop_end = 0
                loop_count = file_info.get("loopCount", 0)

                if "loopStart" in file_info and file_info["loopStart"]:
                    loop_start_str = str(file_info["loopStart"])
                    try:
                        if "." in loop_start_str:
                            loop_start = int(float(loop_start_str) * params.framerate)
                        else:
                            loop_start = int(loop_start_str)
                    except ValueError:
                        print(f"Warning: Invalid loopStart format for {file_name}: {loop_start_str}")
                
                if "loopEnd" in file_info and file_info["loopEnd"]:
                    loop_end_str = str(file_info["loopEnd"])
                    try:
                        if "." in loop_end_str:
                            loop_end = int(float(loop_end_str) * params.framerate)
                        else:
                            loop_end = int(loop_end_str)
                    except ValueError:
                        print(f"Warning: Invalid loopEnd format for {file_name}: {loop_end_str}")
                
                if loop_end == 0:
                    loop_end = params.nframes
                
                if loop_start >= loop_end or loop_end > params.nframes:
                    print(f"Warning: Invalid loop points for {file_name}: start={loop_start}, end={loop_end}, frames={params.nframes}")
                    loop_start = 0
                    loop_end = 0
                
                self.audio_data[audio_id] = {
                    'data': frames,
                    'category': category,
                    'subcategory': subcategory,
                    'index': index,
                    'framerate': params.framerate,
                    'frames': params.nframes,
                    'loopStart': loop_start,
                    'loopEnd': loop_end,
                    'loopCount': loop_count
                }
                
                return True
                
        except Exception as e:
            print(f"Error adding audio file {wave_file}: {str(e)}")
            return False

    def write_bank(self, output_path: str) -> bool:
        try:
            # Sort entries for consistent output
            sorted_entries = []
            for audio_id, data in self.audio_data.items():
                sorted_entries.append((
                    data['category'], 
                    data['subcategory'], 
                    data['index'], 
                    audio_id, 
                    data
                ))
            
            sorted_entries.sort()  # Sort by (category, subcategory, index)
            
            with open(output_path, 'wb') as f:
                # Write magic header and version
                f.write(self.HEADER_MAGIC)
                f.write(struct.pack('<B', self.HEADER_VERSION))
                
                # Calculate header size
                # Each entry: cat(1) + subcat(1) + idx(2) + offset(4) + length(4) + 
                # frames(4) + sampleRate(4) + loopStart(4) + loopEnd(4) + loopCount(2) = 30 bytes
                entry_count = len(sorted_entries)
                entry_size = 30
                header_size = 22 + (entry_count * entry_size)  # 22 = magic(9) + version(1) + counts/sizes(12)
                
                padding = (4 - (header_size % 4)) % 4
                header_size += padding
                
                f.write(struct.pack('<I', entry_count))
                f.write(struct.pack('<I', header_size))
                
                total_size_pos = f.tell()
                f.write(struct.pack('<I', 0))
                
                if padding > 0:
                    f.write(b'\x00' * padding)
                    
                data_start = header_size
                current_offset = data_start
                
                # First pass: write entry headers, calculate offsets
                for cat, subcat, idx, audio_id, data in sorted_entries:
                    data_size = len(data['data'])
                    
                    f.write(struct.pack('<B', cat))
                    f.write(struct.pack('<B', subcat))
                    f.write(struct.pack('<H', idx))
                    
                    f.write(struct.pack('<I', current_offset))
                    f.write(struct.pack('<I', data_size))
                    f.write(struct.pack('<I', data['frames']))
                    
                    f.write(struct.pack('<I', data['framerate']))
                    
                    f.write(struct.pack('<I', data['loopStart']))
                    f.write(struct.pack('<I', data['loopEnd']))
                    f.write(struct.pack('<H', data['loopCount']))
                
                    padding = (4 - (data_size % 4)) % 4
                    current_offset += data_size + padding
                
                # Second pass: write actual audio data
                for cat, subcat, idx, audio_id, data in sorted_entries:
                    data_size = len(data['data'])
                    f.write(data['data'])
                    
                    padding = (4 - (data_size % 4)) % 4
                    if padding > 0:
                        f.write(b'\x00' * padding)
                
                f.seek(total_size_pos)
                f.write(struct.pack('<I', current_offset))
            
            print(f"Successfully wrote bank with {entry_count} sounds to {output_path}")
            print(f"Total size: {current_offset} bytes")
            
            loop_count = sum(1 for _, _, _, _, data in sorted_entries if data['loopStart'] > 0)
            if loop_count > 0:
                print(f"Number of sounds with loop points: {loop_count}")
                
            return True
            
        except Exception as e:
            print(f"Error writing bank: {str(e)}")
            return False

    def get_audio_info(self, audio_id: str) -> Optional[Dict]:
        if audio_id in self.audio_data:
            return self.audio_data[audio_id]
        return None

    def list_audio_files(self) -> List[str]:
        return list(self.audio_data.keys())

    def get_audio_count(self) -> int:
        return len(self.audio_data)

    def clear(self):
        self.audio_data.clear()