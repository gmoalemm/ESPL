section .data
    stdin dd 0
    stdout dd 1
    linefeed dd 10
    infile dd -1
    infile_flag dd "-i", 0
    outfile dd -1
    outfile_flag dd "-o", 0
    file_not_found dd "file not found", 0
    buffer dd 1

section .text
global _start
global system_call
extern strlen
extern strncmp

_start:
    pop     dword ecx    ; ecx = argc
    mov     esi, esp     ; esi = argv
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

    ; set default streams
    mov eax, [stdin]
    mov [infile], eax

    mov eax, [stdout]
    mov [outfile], eax

    print_next_arg:
        cmp     ebx, 0
        jz      done_printing_args

        ; calculate the current arg's length
        push    dword ecx  

        push    dword [ecx]     
        call    strlen
        add     esp, 4

        pop     ecx

        ; print the string
        push    dword eax     ; length
        push    dword [ecx]   ; pointer
        push    dword 1       ; stdout
        push    dword 4       ; print syscall
        call    system_call
        add     esp, 16
        call    print_new_line

        ; check if it is a "-i" flag
        get_input:
            ; compare the first two bytes
            push    dword ecx

            push    dword 2   
            push    dword infile_flag
            push    dword [ecx]
            call    strncmp
            add     esp, 12

            pop     ecx

            ; if it is not a "-i" flag, try "-o"
            cmp     eax, 0
            jne     get_output

            ; open the file
            mov eax, [ecx]          ; eax is not the pointer itself
            add eax, 2              ; move 2 forward to skip "-i"

            push    dword 1411      ; permissions, ignored
            push    dword 0         ; read access
            push    dword eax       ; path
            push    dword 5         ; open syscall
            call    system_call
            add     esp, 16
            
            mov     [infile], eax

            ; if got a negative value, print an error end exit
            cmp     dword [infile], 0
            js      print_err
            je      next_arg

        get_output:
            ; compare the first two bytes
            push    dword ecx

            push    dword 2   
            push    dword outfile_flag
            push    dword [ecx]
            call    strncmp
            add     esp, 12

            pop     ecx

            ; if it is not a "-o" flag, continue
            cmp     eax, 0
            jne     next_arg

            ; open the file
            mov eax, [ecx]          ; eax is not the pointer itself
            add eax, 2              ; move 2 forward to skip "-o"

            push    dword 1411      ; permissions, all (777 octal)
            push    dword 65        ; write/create access
            push    dword eax       ; path
            push    dword 5         ; open syscall
            call    system_call
            add     esp, 16
            
            mov     [outfile], eax

        next_arg:
            ; done one string
            dec     ebx     
            add     ecx, 4
            jmp     print_next_arg

    print_err:
        ; calculate the length of the error message
        push    dword file_not_found   
        call    strlen
        add     esp, 4

        ; print the message to stdout
        push    dword eax           
        push    dword file_not_found         
        push    dword [stdout]     
        push    dword 4   
        call    system_call
        add     esp, 16
        call    print_new_line

    done_printing_args:
        ; read a char from the input file
        push    dword 1           
        push    dword buffer         
        push    dword [infile]     
        push    dword 3   
        call    system_call
        add     esp, 16

        push    dword [buffer]
        call    encode_char
        add     esp, 4

    call print_new_line

    close_files:
        push    dword [infile]
        push    6
        call    system_call
        add     esp, 8

        push    dword [outfile]
        push    6
        call    system_call
        add     esp, 8

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
        mov     ebx, ebp
        add     ebx, 8

        push    dword 1         ; length
        push    dword ebx       ; pointer
        push    dword [outfile]
        push    dword 4         ; print syscall
        call    system_call
        add     esp, 16

    popad
    mov     eax, 0
    pop     ebp
    ret


print_new_line:
    push    ebp
    mov     ebp, esp

    push    dword 2           
    push    dword linefeed           
    push    dword [stdout]   
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
