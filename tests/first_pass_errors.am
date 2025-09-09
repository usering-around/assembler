; this file contains all of the possible errors
; which are exclusive to the first pass (except memory overflown) 
; expected an instruction or directive after label
someLabel:
; invalid directive
.ignore
        .hi
; empty data
.data
; invalid data
.data hello, world!
.data 3, "what is up!!!"
; invalid character after integer
.data 12, -23, 31 s
.data 31 yellow
; comma after last integer
.data 13, 24,
.data -25,
; integer too big
.data 123131231
.data 16777216
; integer too small
.data -16777217
; string should start with a quote
.string hello
.string hello," 
; string should end with a quote
.string "hello
; no symbol in entry
.entry
; invalid symbol in entry
.entry 3hello
.entry hello$
.entry jfieajfieojfiojaoifejiwoajfeiowa
; no symbol in extern
.extern 
;invalid symbol in extern
.extern 3hello
.extern hello$
.extern jfieaojfoiejfiowajfiojwiaofjioweajf
; invalid instruction
hello
    goodbye
; no integer in immediate 
add #, r1
inc #
; immediate is too big
add #123131231, r1
; immediate is too small
add #-16777217, r1
; invalid cahracter after operand
add r1, r3 hello 
; too many operands
add r1, r2, r3
inc r1, r2
stop r6
; too little operands
add r1
inc
sub r5, 
; comma after operand
add r1, r3,
; expected a different dest operand
hello: mov #1, r3
mov &hello, r4
jmp #1
jmp r5
prn &hello
; expected a different src operand
mov r4, &hello
lea r3, #31
; empty first operand
add , r4
inc ,
; invalid symbols in instructions
add 3hello, r4
add hello$$, r7
sub r4, 1nvalid
; invalid labels
3invalid: add r3, r4
random$$: stop
jfeiwajfioewjafioejwaiofjwaiofjioajfiowajfeiao: jmp &label
; label defined twice
label: add r2, r7
label: .data 1,2,3
; symbol is an instruction
add: .entry hello
.entry stop
; symbol is an directive
data: .data 1, 2, 3
.extern entry
; symbol is a register
r7: add r1,r2
.extern r0
; no space after label
labell:stop
; symbol is empty 
: add r5, r3





