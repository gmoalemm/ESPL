section .text
    msg: db "Hello, Infected File", 10, 0

global _start
global system_call
global infection
global infector
extern main
extern strlen

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc
 
    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
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

code_start:

infection:
    push    ebp
    mov     ebp, esp
    pushad

    push    dword msg    
    call    strlen
    add     esp, 4

    ; print the message

    mov     edx, eax
    mov     eax, 4  
    mov     ebx, 1
    mov     ecx, msg

    int     0x80

    cmp     eax, 0
    jle     infection_error    ; eax <= 0 (0 or -1)

    popad
    pop     ebp
    ret

    infection_error:
        popad
        pop     ebp
        mov     eax, 1
        mov     ebx, 0x55
        int 0x80


infector:
    push    ebp
    mov     ebp, esp
    sub     esp, 4      ; fd
    pushad

    ; print the given filename

    push    dword [ebp + 8]    
    call    strlen
    add     esp, 4

    mov     edx, eax
    mov     eax, 4  
    mov     ebx, 1
    mov     ecx, [ebp + 8]  
    int     0x80

    cmp     eax, 0
    jle     infector_error

    ; open the file

    mov     eax, 5
    mov     ebx, [ebp + 8]
    mov     ecx, 1025       ; write and append
    mov     edx, 511        ; all permissions
    int     0x80

    cmp     eax, -1
    je      infector_error

    mov     [ebp - 4], eax

    ; add the code

    mov     eax, 4
    mov     ebx, [ebp - 4]
    mov     ecx, code_start
    mov     edx, code_end
    sub     edx, code_start
    int     0x80

    cmp     eax, 0
    jle     infector_error

    ; close the file

    mov     eax, 6
    mov     ebx, [ebp - 4]
    int     0x80

    cmp     eax, -1
    je      infector_error

    popad
    add     esp, 4
    pop     ebp
    ret

    infector_error:
        popad
        add     esp, 4
        pop     ebp
        mov     eax, 1
        mov     ebx, 0x55
        int 0x80

code_end:
