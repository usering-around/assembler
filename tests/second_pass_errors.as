; this file contains all the errors which are unique to the second pass
; symbol undefined 
help: add r1, r2
.entry help
.entry doesNotExist

add doesNotExist, r3
inc doesNotExist

; extern and entry for the same symbol in the same file 
.extern someCoolSymbol
.entry someCoolSymbol