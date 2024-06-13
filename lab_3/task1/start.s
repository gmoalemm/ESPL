section .data
    linefeed db 10
    infile db 0
    outfile db 1

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
        jz      done_printing_args

        push    dword [ecx]     ; pointer to the current string, compute its length
        call    strlen
        add     esp, 4

        ; print the string
        push    dword eax     ; length
        push    dword [ecx]   ; pointer
        push    dword 1       ; stdout
        push    dword 4       ; print syscall
        call    system_call
        add     esp, 16

        call    print_new_line

        ; done one string
        dec     ebx     
        add     ecx, 4
        jmp     print_next_arg

    done_printing_args:
        push    dword 'A'
        call    encode_char
        add     esp, 4
        
        call    print_new_line

    done_main:
        popad           ; Restore caller state (registers)
        mov     eax, 0  ; place returned value where caller can see it
        pop     ebp     ; Restore caller state
        ret             ; Back to caller

; reads a character, encodes it by adding 1 to the character value if it is in 
; the range 'A' to 'z' (no encoding otherwise), and outputs it.
encode_char:
    push    ebp
    mov     ebp, esp
    pushad

    cmp     [ebp + 8], dword 'A'
    js      print_encoded           ; eax < 'A'

    cmp     [ebp + 8], dword 'Z'
    ja      print_encoded           ; 'Z' < eax

    inc     dword [ebp + 8] 

    print_encoded: 
        ; print the encoded char
        mov     ebx, ebp
        add     ebx, 8

        push    dword 1         ; length
        push    dword ebx       ; pointer
        push    dword [outfile]   ; stdout
        push    dword 4         ; print syscall
        call    system_call
        add     esp, 16

    popad
    mov     eax, 0
    pop     ebp
    ret

print_new_line:
    push    ebp

    push    dword 1           
    push    dword linefeed           
    push    dword [outfile]     
    push    dword 4   
    call    system_call
    add     esp, 16

    pop     ebp
    ret

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
