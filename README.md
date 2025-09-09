An assembler for a simple imaginary assembly language. See the tests folder for examples.

To build, simply run:<br>
`make` <br>
to clean the build artifacts, run:<br>
`make clean` <br>
To build it as optimized or in debug mode, clean any build artifacts and then run either:<br>
`make opt` <br>
for optimized, or<br> 
`make dbg` <br>
for debug. <br>

Note: it was required to use ansi C and to check every allocation, hence almost every function returns a boolean indicating whether or not malloc returned NULL.
