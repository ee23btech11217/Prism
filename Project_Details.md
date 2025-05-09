# PRISM: Custom Handheld Gaming Console

![Prism Logo](logo.png)

*A custom-built handheld gaming console with 3D-capable GPU and gyroscopic control*

## Overview

Prism is a handheld gaming console featuring a custom 3D-capable GPU implemented on a Zynq SoC FPGA, complete with programmable graphics pipeline and gyroscopic motion controls. Designed by Y𝜄𝜋10𝜕 Labs as part of ESDP Lab at IIT Hyderabad, Prism combines custom hardware architecture with modern game development frameworks.

## Technical Specifications

### Display
- **Resolution**: During development timeline, we plan on testing with the following resolutions
  - Mid-Spec: 400 × 240 @ 60fps
  - Low-Spec: 200 × 120 @ 30fps
- **Screen Size**: 5-inch LCD Panel @ 60Hz
- **Pixel Density**: 151 PPI
- **Color Support**: 15-bit RGB (32,768 colors)

### Controls
- **Gyroscopic Motion**: 6-axis IMU sensor (LSM6DSL)
- **Analog Joystick**: Full 360° with variable pressure
- **D-Pad**: 4-element directional pad
- **Buttons**: Multiple action and menu buttons

### Hardware Architecture
- **SoC**: Xilinx Zynq-7000 Series
- **Memory**: DDR3 SDRAM
- **Storage**: SD Card / ROM
- **Graphics**: Custom GPU pipeline implemented in FPGA fabric
  - Vertex Shader
  - Shape Assembly
  - Rasterization
  - Pixel Shader
- **Custom Processor**: 16-bit Nyx architecture with 40 processing cores
  - 80 simultaneous threads (2 per core)
  - 18-bit instruction set
  - Barrel shifter-based ALU
  - Hardware-accelerated multiplication

### Graphics Pipeline
- Custom graphics processing unit for 3D rendering
- Frame buffer stored in on-chip memory
- Parallel processing for vertex and pixel operations
- Support for textured rendering
- Matrix-based 3D transformations

## Software Architecture

### Programming Interface
- C++ API for game development
- OpenGL-compatible interface
- GPU command scheduler
- Model, texture, and scene management

### Libraries
- **Driver Layer**: Hardware abstraction for:
  - Display output
  - Control inputs
  - Storage access
  - Sensors integration
- **Graphics Layer**:
  - OpenGL-compatible API
  - 3D model loading (OBJ file support)
  - Scene graph management
  - Camera controls

### Game Development Tools
- Emulation environment for desktop platforms
- Custom assembler for Nyx architecture (using Python)
- Model importing tools

## Assembly Architecture

The Prism console uses a custom 16-bit instruction set architecture (ISA) called Nyx for its core processing:

- **Instruction Format**: 18-bit wide instructions
- **Register File**: 15 general-purpose registers per thread
- **Addressing Modes**: Register-direct and immediate
- **Key Instructions**:
  - Arithmetic: `add`, `sub`, `mul`
  - Logical: `and`, `or`, `not`
  - Memory: `ldb`, `str`
  - Flow Control: `jmp`, `beq`, `ble`
  - Special: `bsh` (barrel shift), `hlt` (halt)
- **Extended Instructions**:
  - Convenience ops: `mov`, `cmp`, `test`, `lds`, `ldc`, `inc`

## Project Structure

- **Hardware Design**: PCB and schematic files for Zynq FPGA-based board
- **FPGA Development**: Verilog sources for processor core and GPU
- **Drivers**: C++ access layers for hardware components
- **Graphics Library**: GL-compatible rendering system
- **Emulation**: OpenGL-based desktop emulator

## Development Timeline

The project follows a phased development approach:
1. **Phase 1**: Architecture design and specifications freeze
2. **Phase 2**: Low-spec implementation (200 × 120 @ 60fps)
3. **Phase 3**: Mid-spec implementation (400 × 240 @ 60fps)
4. **Phase 4**: PCB design/fabrication and hardware assembly
5. **Phase 5**: Game development and testing


### Emulation
The emulator provides a desktop environment for testing games and applications before deploying to the hardware device.

## Team

Y𝜄𝜋10𝜕 Labs:
- Adarsh A (EE23BTECH11203)
- Manohar K (EE23BTECH11208)
- Prajwal M (EE23BTECH11217)
- Mihir Divyansh Eachempati (EE23BTECH11017)
- Soham Prabhakar More (EE23BTECH11223)
- Sri Krishna Prabhas Yadla (EE23BTECH11224)
