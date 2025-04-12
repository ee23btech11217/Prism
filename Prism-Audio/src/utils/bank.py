import wave
import struct
import os
from typing import Dict, List, Tuple, Optional

class AudioBank:
    HEADER_MAGIC = b'PRSMAUBNK'  
    HEADER_VERSION = 1
    
    def __init__(self, config_path: str = "configs/gameAudioHeirarchy.json"):
        import json
        self.audio_files = {}    # contains the id:file mappings
        self.audio_data = {}     
        self.config = self._load_config(config_path)
        self._generate_file_mapping()

    def _load_config(self, config_path: str) -> Dict:
        import json
        with open(config_path, 'r') as f:
            return json.load(f)

    def _generate_file_mapping(self):
        for cat_id, category in self.config.items():
            for subcat_id, subcategory in category["subcategories"].items():
                for sound_id, sound in subcategory["sounds"].items():
                    identifier = f"{cat_id}.{subcat_id}.{sound_id}"
                    self.audio_files[identifier] = sound["file"]

    def add_audio(self, wave_file: str) -> bool:
        file_name = os.path.basename(wave_file)
        
        audio_id = None
        for id, _file in self.audio_files.items():
            if _file == file_name:
                audio_id = id
                break
        
        if not audio_id:
            print(f"Warning: No matching ID found for file {file_name}")
            return False

        try:
            with wave.open(wave_file, 'rb') as wav:
                params = wav.getparams()
                frames = wav.readframes(params.nframes)
                
                parts = audio_id.split('.')
                category = int(parts[0])
                subcategory = int(parts[1])
                index = int(parts[2])
                
                self.audio_data[audio_id] = {
                    'data': frames,
                    'category': category,
                    'subcategory': subcategory,
                    'index': index,
                    'channels': params.nchannels,
                    'sample_width': params.sampwidth,
                    'framerate': params.framerate,
                    'frames': params.nframes
                }
                
                return True
                
        except Exception as e:
            print(f"Error adding audio file {wave_file}: {str(e)}")
            return False

    def write_bank(self, output_path: str) -> bool:
        try:
            sorted_entries = []
            for audio_id, data in self.audio_data.items():
                sorted_entries.append((
                    data['category'], 
                    data['subcategory'], 
                    data['index'], 
                    audio_id, 
                    data
                ))
            
            sorted_entries.sort()
            
            with open(output_path, 'wb') as f:
                f.write(self.HEADER_MAGIC)
                f.write(struct.pack('<B', self.HEADER_VERSION))
                
                # Each entry needs 20 bytes: cat(1) + subcat(1) + idx(2) + offset(4) + length(4) + 
                # frames(4) + sampleRate(4)
                entry_count = len(sorted_entries)
                entry_size = 20
                header_size = 22 + (entry_count * entry_size)  # 22 = magic(9) + version(1) + counts/sizes(12)
                
                f.write(struct.pack('<I', entry_count))
                f.write(struct.pack('<I', header_size))
                
                total_size_pos = f.tell()
                f.write(struct.pack('<I', 0))
                
                data_start = header_size
                offsets = {}
                current_offset = data_start
                
                # write entry headers, calculate offsets
                for cat, subcat, idx, audio_id, data in sorted_entries:
                    data_size = len(data['data'])
                    
                    f.write(struct.pack('<B', cat))
                    f.write(struct.pack('<B', subcat))
                    f.write(struct.pack('<H', idx))
                    f.write(struct.pack('<I', current_offset))
                    f.write(struct.pack('<I', data_size))
                    f.write(struct.pack('<I', data['frames']))
                    f.write(struct.pack('<I', data['framerate']))
                    
                    offsets[audio_id] = current_offset
                    
                    padding = (4 - (data_size % 4)) % 4
                    current_offset += data_size + padding
                
                # write the audio data
                for cat, subcat, idx, audio_id, data in sorted_entries:
                    data_size = len(data['data'])
                    f.write(data['data'])
                    
                    padding = (4 - (data_size % 4)) % 4
                    if padding > 0:
                        f.write(b'\x00' * padding)
                
                f.seek(total_size_pos)
                f.write(struct.pack('<I', current_offset))
            
            print(f"Successfully wrote bank with {entry_count} sounds to {output_path}")
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