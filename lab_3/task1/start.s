section .rodata
    linefeed db 10

section .text
global _start
global system_call
extern strlen

_start:
    pop     dword ecx    ; ecx = argc
    mov     esi, esp      ; esi = argv
    mov     eax, ecx     ; put the number of arguments into eax

    ; shift num of args two bytes to the left (mult by pointer size (= 4))
    shl     eax, 2       ; compute the size of argv in bytes

    add     eax, esi     ; add the size to the address of argv 
    add     eax, 4       ; skip NULL at the end of argv

    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc
 
    call    main        ; int main( int argc, char *argv[], char *envp[] )

    ; exit
    mov     ebx, eax
    mov     eax, 1
    int     0x80
    nop

main:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state
            
    mov     ebx, [ebp+8]    ; argc
    mov     ecx, [ebp+12]   ; argv
    mov     edx, [ebp+16]   ; envp

    print_next_arg:
        cmp     ebx, 0
        jz      done

        push    dword [ecx]     ; pointer to the current string, compute its length
        call    strlen
        add     esp, 4

        ; print the string
        push    dword eax     ; length
        push    dword [ecx]   ; pointer
        push    dword 1       ; stdout
        push    dword 4       ; print syscall

        call system_call

        add esp, 16

        ; print a new line
        push     dword 1           
        push     dword linefeed           
        push     dword 1     
        push     dword 4           
        call system_call

        add esp, 16

        ; done one string
        dec     ebx     
        add     ecx, 4
        jmp     print_next_arg

    done:
        popad           ; Restore caller state (registers)
        mov     eax, 0  ; place returned value where caller can see it
        pop     ebp     ; Restore caller state
        ret             ; Back to caller

system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller
