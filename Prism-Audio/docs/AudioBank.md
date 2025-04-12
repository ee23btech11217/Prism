# The PrismAudioBank Format

## Overview

The PrismAudio Bank (.pab) format is a binary container designed to store multiple audio files (wave) for real-time playback on embedded systems. It uses a hierarchical categorization system (Category → Subcategory → Index) to organize sounds

## File Structure

A PrismAudio Bank file consists of three main sections:

1. **Main Header** (20 bytes) - Contains file metadata
2. **Entry Headers** (22 bytes each) - Contains metadata for each audio file
3. **Audio Data** - Contains the raw audio sample data


### Main Header (20 bytes)

| Offset | Size | Type     | Description                                  |
|--------|------|----------|----------------------------------------------|
| 0      | 9    | char[9]  | Magic identifier ("PRSMAUBNK")              |
| 9      | 1    | uint8_t  | Format version                               |
| 10     | 4    | uint32_t | Number of entries                            |
| 14     | 4    | uint32_t | Total header size (20 + 22×entries)          |
| 18     | 4    | uint32_t | Total file size                              |

### Entry Header (22 bytes)

Each entry header contains metadata for one audio sample:

| Offset | Size | Type     | Description                                  |
|--------|------|----------|----------------------------------------------|
| 0      | 1    | uint8_t  | Category ID                                  |
| 1      | 1    | uint8_t  | Subcategory ID                               |
| 2      | 2    | uint16_t | Sound index                                  |
| 4      | 4    | uint32_t | Offset to audio data from start of file      |
| 8      | 4    | uint32_t | Length of audio data in bytes                |
| 12     | 4    | uint32_t | Number of frames (samples)                   |
| 16     | 2    | uint16_t | Number of channels (1=mono, 2=stereo)        |
| 18     | 4    | uint32_t | Sample rate in Hz                            |

### Audio Data Blocks

The audio data section contains raw audio data for each entry:

- Each block contains raw PCM audio data (16-bit samples)
- Blocks are 4-byte aligned for optimized memory access
- Block sizes match the lengths specified in the corresponding entry headers
- Blocks appear in the same order as their entry headers

## Sound Identification

Sounds are identified by a three-part hierarchical ID:

- **Category** (uint8_t): Primary classification (e.g., 1=Ambient, 2=Character)
- **Subcategory** (uint8_t): Secondary classification (e.g., 1=Background, 2=Weather)
- **Index** (uint16_t): Specific sound within the subcategory

These three values can be combined to form a unique identifier for each sound in the bank.

## Flow Diagram