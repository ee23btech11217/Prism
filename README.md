# PrisM
Repository for PrisM, a custom FPGA-based handheld gaming console, built from scratch.

## General HLS Info
1. Use `#pragma` commands found ()[here] to optimise any steps in your code. For example
```C
#pragma HLS INTERFACE m_axi port=A bundle=mem0 // Called Compiler directives
#pragma HLS INTERFACE m_axi port=x bundle=mem1 // This specifies the interface to the synthesizer --> No need to write interfacing code
int a;
for(int row = 0; row < N; row++)
{
    for (int col1 = 0; col < N/2; col++)
    {
        #pragma HLS pipeline II=1 //II stands for cycle count / Task Interval
        for(...)
        {
            .
            .
            .

        }
    }
}

```
This syncs the multiple slices and can do matrix vector multiplication.

2. `printf` is not a thing, instead use `hls::print()` with similar syntax


### Supported Standard libraries
- An approximate version of math.h, accessible as <hls_math.h>
    - Difference of 1-4 Units of Least Precision
    - Be careful with comparision functions (isinf, isnan, isnormal ...) as they may introduce unnecessary float->double blocks in hardware

### No gos
- No dynamic memory allocation
- No sys calls (printf, exit ...)
- No recursion
- Don't use too many pointers




# Starter code 
Just for reference ()[host.cpp]