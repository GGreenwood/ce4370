; Subroutine for calculating the factorial of an input
; INPUT: The input parameter is 
;   R12 -- The number to find the factorial of
; OUTPUT: The output parameter is
;   R13 -- The factorial

    PUBLIC factorial
    RSEG CODE

; R12 holds the loop counter and starts at the value of R12
; R13 holds the factorial value and is multiplied by R12 every loop
; The loop ends after R12 is one. 

factorial:
    MOV R12, R13    ; Load the initial value into R13
loop:
    MULT R12, R13   ; Multiply the loop counter and the factorial
    DEC R12         ; Decrement the loop counter
    JNZ #loop       ; If we aren't at 0 yet, go back to loop
end:
    RET             ; Otherwise, return
