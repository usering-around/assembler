; this file consists of possible edge cases for the assembler

; should be fine
   inc        r7                                  
 ; this comment should be read as an instruction and thus be invalid
; this should be invalid since there is no instruction
HELP: 
; should be invalid due to having random characters after keywords
addd r3, r4
.dataa 1, 2, 3
; should not be an error
addd: inc r4
; defining the same label in the same line (should be ane error)
label: .extern label
; 2 errors in 1 line
add: inc &bye
sub: .extern addd
; should be an invalid instruction error
help: help: help: 

; empty label 
: add r1, r2

; just below/above the int limit 
.data -2097153
.data 2097152
; at the int limit
.data -2097152
.data 2097151


; should be fine
.string ""

; should be fine
r8: add r1, r2

; should give errors about integer too big 
add #19218309218390218390293019032139213902130921930, r1
add #-19218309218390218390293019032139213902130921930, r1
.data 19218309218390218390293019032139213902130921930
.data -19218309218390218390293019032139213902130921930

; should give symbol too long error 
add r12839123891389213812312312312321, r3

; second pass error when first pass fails 
.entry doesNotEXIST
ignoreMe: inc doesNotExist
.extern someCoolSymbol
.entry someCoolSymbol