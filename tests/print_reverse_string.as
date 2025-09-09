; this program prints a string the user inputs in the reverse order

; we don't have any load or store operations which is quite 
; problematic for this task, hence we just assume that they 
; exist through functions
; throughout this entire file we assume r0, r1, r2 are scratch registers

; assume there exists some magical function store
; which takes r1 and r2 as arguements, 
; and stores the value of r2 in the address in r1
.extern STORE
; and finally that there exists some magical function load 
; which takes r1 and r2 as arguements and 
; loads the value in the address r1 into r2 
.extern LOAD

userInputMsg: .string "please input a string of 10 characters to reverse: "
reverseMsg: .string "the string in reverse is: "
; space to store the str in 
str: .data 0,0,0,0,0,0,0,0,0,0,0

main: lea userInputMsg, r1
jsr printStr
lea str, r1

; macro to read 1 character into the address in r1 and increment the addr
mcro push_char
red r2
jsr STORE
add #1, r1
mcroend 

; read 10 characters
push_char
push_char
push_char
push_char
push_char
push_char
push_char
push_char
push_char
push_char

mov r1, r3
lea reverseMsg, r1
jsr printStr

; now time to print the reverse
mcro pop_char
sub #1, r1
jsr LOAD
prn r2
mcroend

mov r3, r1
; pop char 10 times to print in reverse 
pop_char
pop_char
pop_char
pop_char
pop_char
pop_char
pop_char
pop_char
pop_char
pop_char
; we're done! 
stop 

; this function takes an address in r1 representing a string
; and prints the string 
printStr: jsr LOAD
cmp #0, r2
bne &doLoop
jmp &printStrExit

doLoop: prn r2
add #1, r1
jmp &printStr

printStrExit: rts

; export the function for other files 
.entry printStr
