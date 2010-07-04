;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Distributed Hash Cracker v2.0														;
;																					;
; Copyright (c) 2009 RPISEC															;
; All rights reserved.																;
;																					;
; Rrdistribution and use in source and binary forms, with or without				;
; modification, are permitted provided that the following conditions are met:		;
;     *Rrdistributions of source code must retain the above copyright				;
;      notice, this list of conditions and the following disclaimer.				;
;     *Rrdistributions in binary form must reproduce the above copyright			;
;      notice, this list of conditions and the following disclaimer in the			;
;      documentation and/or other materials provided with the distribution.			;
;     *Neither the name of RPISEC nor the											;
;      names of its contributors may be used to endorse or promote products			;
;      derived from this software without specific prior written permission.		;
;																					;
; THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY								;
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED			;
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE			;
; DISCLAIMED. IN NO EVENT SHALL RPISEC BE LIABLE FOR ANY							;
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES		;
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;		;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND		;
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT		;
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS		;
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.						;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;																					;
; MD5_amd64.asm - x86 SSE implementation of MD5										;
;																					;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 
BITS 64
 
SECTION .rodata
extern _MD5init_a
extern _MD5init_b
extern _MD5init_c
extern _MD5init_d
extern _MD5all_ones
extern _MD5tbuf
 
SECTION .text 
 
;//#define F(b,c,d) (((b) & (c)) | (~(b) & (d)))
%macro  sse_md5round_f 7
movaps		xmm4,		%2				; xmm4 = b
andps		xmm4,		%3				; xmm4 = b & c
movaps		xmm5,		%2				; xmm5 = b
andnps		xmm5,		%4				; xmm5 = ~b & d
orps		xmm4,		xmm5			; xmm4 = F*/		
 
movaps		xmm5,		[rax + 16*%5]		; prefetch x[i] 
paddd		%1,			xmm4				; a = a + F
paddd		%1,			[_MD5tbuf + 16*%7]	; a = a + F + T[i]*
paddd		%1,			xmm5				; a = a + F + T[i] + x[i]*
 
movaps		xmm4,		%1					; rotate a left shamt bits
pslld		%1,			%6		        								
psrld		xmm4,		(32-%6)									
orps		%1,			xmm4										
 
paddd		%1,			%2					; a = ROTL(~~~,N) + b) 
 
%endmacro
 
;//#define G(b,c,d) (((b) & (d)) | (~(d) & (c)))
%macro sse_md5round_g 7
movaps		xmm4,		%2					; xmm4 = b
andps		xmm4,		%4					; xmm4 = b & d
movaps		xmm5,		%4					; xmm5 = d
andnps		xmm5,		%3					; xmm5 = ~d & c
orps		xmm4,		xmm5				; xmm4 = G
movaps		xmm5,		[rax + 16*%5]		; prefetch x[i] 
paddd		%1,			xmm4				; a = a + F
paddd		%1,			[_MD5tbuf + 16*%7]	; a = a + F + T[i]*
paddd		%1,			xmm5				; a = a + F + T[i] + x[i]*
 
movaps		xmm4,		%1					; rotate a left shamt bits
pslld		%1,			%6		        								
psrld		xmm4,		(32-%6)									
orps		%1,			xmm4										
 
paddd		%1,			%2					; a = ROTL(~~~,N) + b) 
%endmacro
 
 
;//#define H(b,c,d) ((b) ^ (c) ^ (d))
;(a,b,c,d,index,shamt,stepnum) __asm							
%macro sse_md5round_h 7
movaps		xmm4,		%2			 		;  xmm4 = b
xorps		xmm4,		%3					;	xmm4 = b ^ c
xorps		xmm4,		%4			 		;	xmm4 = H
movaps		xmm5,		[rax + 16*%5]		; prefetch x[i] 
paddd		%1,			xmm4				; a = a + F
paddd		%1,			[_MD5tbuf + 16*%7]	; a = a + F + T[i]*
paddd		%1,			xmm5				; a = a + F + T[i] + x[i]*
 
movaps		xmm4,		%1					; rotate a left shamt bits
pslld		%1,			%6		        								
psrld		xmm4,		(32-%6)									
orps		%1,			xmm4										
 
paddd		%1,			%2					; a = ROTL(~~~,N) + b) 
%endmacro
 
;//#define I(b,c,d) ((c) ^ ((b) | ~(d)))
;(a,b,c,d,index,shamt,stepnum) 
%macro sse_md5round_i 7
movaps		xmm4,		%4					;xmm4 = d
xorps		xmm4,		[_MD5all_ones]		;xmm4 = ~d
orps		xmm4,		%2					;xmm4 = b | ~d
xorps		xmm4,		%3					;xmm4 = I
movaps		xmm5,		[rax + 16*%5]		; prefetch x[i] 
paddd		%1,			xmm4				; a = a + F
paddd		%1,			[_MD5tbuf + 16*%7]	; a = a + F + T[i]*
paddd		%1,			xmm5				; a = a + F + T[i] + x[i]*
 
movaps		xmm4,		%1					; rotate a left shamt bits
pslld		%1,			%6		        								
psrld		xmm4,		(32-%6)									
orps		%1,			xmm4										
 
paddd		%1,			%2				    ; a = ROTL(~~~,N) + b) 
%endmacro
 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;void MD5SSE2Hash(unsigned char* in,    unsigned char* out, int len);
global _MD5SSE2Hash:function
_MD5SSE2Hash:
  push rbp
  mov  rbp, rsp
  push rdi
  push rsi
  push rbx
  push rsp
  and  rsp, 0xfffffffffffffff0
  sub  rsp, 320 
  ;if(len>50)
  ;	return;
 
  mov qword rax, [16+rbp]
  cmp rax, 50
  jb .ok
    mov rsp, [rbp-16]
    pop rbx
    pop rsi
    pop rdi
    leave
    ret
.ok:
 
; interleave input 
; copy qword blocks first
; input[0] points to hash1 data, input[1] to hash2 data, ... 
; len = len & ~3
  mov rcx, [ebp+16]
  mov rbx, rcx
  and rcx, 0xfffffffffffffffc
  mov rdx, [ebp+8]
  cmp rcx, 0
  je .endcopydata
  mov rsi, rsp
  add rsi, 64
  lea rsi, [rsi+rcx*4]
  sub rsi, 16
  shr rcx, 2
 
  .copydata:
    ; input[0]+0
    lea rax, [rdx]
    mov eax, [rdx + rcx*4 -4]
    mov [rsi], eax
 
    ; input[1]+len
    lea rax, [rdx+rbx*1]
    mov eax, [rax + rcx*4 -4]
    mov [rsi+4], eax
 
    ; input[2][] +len*2
    lea rax, [rdx+rbx*2]
    mov eax, [rax + rcx*4 -4]
    mov [rsi+8], eax
 
    ; input[3][] + len*3
    lea rax, [rdx+rbx*2]
    add rax, rbx
    mov eax, [rax + rcx*4 -4]
    mov [rsi+12], eax
    sub rsi, 16
  loop .copydata
 
.endcopydata:
; ///////////////////////////////////
; fill in remaining if any
  mov qword rbx, [rbp+16]	; rbx = len
  mov rcx, rbx
  and rcx, 3				; rcx = len & 3 (number of bytes to copy)
  and rbx, 0xfffffffffffffffc		; rbx = len & ~3 (bytes copied so far)
  lea rsi, [rsp + 64]		; rsi = xbuf
  lea rsi, [rsi + rbx*4]	; rsi = xbuf + base*sizeof(qword). This is the last row that needs our data..
  mov qword [rsi], 0		; Wipe any junk that might be sitting after the end of our last copy.
  mov qword [rsi+4], 0
  mov qword [rsi+8], 0
  mov qword [rsi+12], 0
  push rbx					; save (len & ~3)
  mov qword rbx, [rbp+16]	; rbx = len
  cmp rcx, 0				; see if we need to do a byte copy
  je .step1					; if not, skip everything until step 1
  dec rcx
   .copybytes:
		;rcx = byte being copied
		;rbx = len
	
	lea rdi, [rcx + rdx]	;rdi = &in[iByte]
	add rdi, [rsp]			;rdi = &in[base + iByte]

	mov byte al, [rdi]		; Copy from in[base + iByte]
	mov byte [rsi+rcx], al	
	
	mov byte al, [rdi + rbx] ; Copy from in[base + len + iByte]
	mov byte [rsi+rcx+4], al
	
	mov byte al, [rdi + 2*rbx]	; Copy from in[base + 2*len + iByte]
	mov byte [rsi+rcx+8], al
	
	lea rdi, [rdi + 2*rbx]		;no way to easily add 3*len
	mov byte al, [rdi + rbx]	; Copy from in[base + 3*len + iByte]
	mov byte [rsi+rcx+12], al
    
	dec rcx
    cmp rcx, 0
  jge .copybytes 
 
; STEP 1
;appending padding
.step1:
; '1' bit goes @ (len&~3)*4 + (len&3) + xbuf
  pop rcx				;rcx = len & ~3
  and rbx, 3			;rbx = len & 3

  mov rax, rbx
  sal rcx, 2
  add rax, rcx
 
  mov rsi, rsp
  add rsi, 64
  mov byte [rsi+rax], 0x80
  mov byte [rsi+rax+4], 0x80
  mov byte [rsi+rax+8], 0x80
  mov byte [rsi+rax+12], 0x80
  add rax, 16
  ; append '0' bits up until 64-bits from a 512 bit boundary (56 bytes from xbuf)
 
  lea rcx, [rsi+224] ; rcx is the end boundary for the '0' bits
  lea rbx, [rsi+rax]
  and rbx, 0xfffffffffffffffc
  .zeropad:
    mov qword [rbx], 0
    mov qword [rbx+4], 0
    mov qword [rbx+8], 0
    mov qword [rbx+12], 0
    add rbx, 16
    cmp rbx, rcx
  jne .zeropad
 
.step2:
; //STEP 2: Append Length - length is a 64-bit little endian number
  mov rax, [ebp+16]
  sal rax, 3
  mov rsi, rsp
  add rsi, 64
  mov qword [rsi+56*4], rax
  mov qword [rsi+57*4], rax
  mov qword [rsi+58*4], rax
  mov qword [rsi+59*4], rax
; add NULL most significant words
  mov qword [rsi+60*4], 0
  mov qword [rsi+61*4], 0
  mov qword [rsi+62*4], 0
  mov qword [rsi+63*4], 0
 
  mov rax, rsp
  add rax, 64
 
  .step3:
  ;//STEP 3: Initialize Constants
  
  movaps				xmm0,			[_MD5init_a]
  movaps				xmm1,			[_MD5init_b]
  movaps				xmm2,			[_MD5init_c]
  movaps				xmm3,			[_MD5init_d]
 
;//STEP 4: MD5 Rounds
 
	;Round 1
    sse_md5round_f xmm0,xmm1,xmm2,xmm3,0,7,0;
    sse_md5round_f xmm3,xmm0,xmm1,xmm2,1,12,1 ;
    sse_md5round_f xmm2,xmm3,xmm0,xmm1,2,17,2 ;
    sse_md5round_f xmm1,xmm2,xmm3,xmm0,3,22,3 ;
    sse_md5round_f xmm0,xmm1,xmm2,xmm3,4,7,4 ;
    sse_md5round_f xmm3,xmm0,xmm1,xmm2,5,12,5 ;
    sse_md5round_f xmm2,xmm3,xmm0,xmm1,6,17,6 ;
    sse_md5round_f xmm1,xmm2,xmm3,xmm0,7,22,7 ;
    sse_md5round_f xmm0,xmm1,xmm2,xmm3,8,7,8 ;
    sse_md5round_f xmm3,xmm0,xmm1,xmm2,9,12,9 ;
    sse_md5round_f xmm2,xmm3,xmm0,xmm1,10,17,10 ;
    sse_md5round_f xmm1,xmm2,xmm3,xmm0,11,22,11 ;
    sse_md5round_f xmm0,xmm1,xmm2,xmm3,12,7,12 ;
    sse_md5round_f xmm3,xmm0,xmm1,xmm2,13,12,13 ;
    sse_md5round_f xmm2,xmm3,xmm0,xmm1,14,17,14 ;
    sse_md5round_f xmm1,xmm2,xmm3,xmm0,15,22,15 ;
 
	;Round 2
    sse_md5round_g xmm0,xmm1,xmm2,xmm3,1,5,16 ;
    sse_md5round_g xmm3,xmm0,xmm1,xmm2,6,9,17 ;
    sse_md5round_g xmm2,xmm3,xmm0,xmm1,11,14,18 ;
    sse_md5round_g xmm1,xmm2,xmm3,xmm0,0,20,19 ;
    sse_md5round_g xmm0,xmm1,xmm2,xmm3,5,5,20 ;
    sse_md5round_g xmm3,xmm0,xmm1,xmm2,10,9,21 ;
    sse_md5round_g xmm2,xmm3,xmm0,xmm1,15,14,22 ;
    sse_md5round_g xmm1,xmm2,xmm3,xmm0,4,20,23 ;
    sse_md5round_g xmm0,xmm1,xmm2,xmm3,9,5,24 ;
    sse_md5round_g xmm3,xmm0,xmm1,xmm2,14,9,25 ;
    sse_md5round_g xmm2,xmm3,xmm0,xmm1,3,14,26 ;
    sse_md5round_g xmm1,xmm2,xmm3,xmm0,8,20,27 ;
    sse_md5round_g xmm0,xmm1,xmm2,xmm3,13,5,28 ;
    sse_md5round_g xmm3,xmm0,xmm1,xmm2,2,9,29 ;
    sse_md5round_g xmm2,xmm3,xmm0,xmm1,7,14,30 ;
    sse_md5round_g xmm1,xmm2,xmm3,xmm0,12,20,31 ;
 
    ; Round 3
    sse_md5round_h xmm0,xmm1,xmm2,xmm3,5,4,32 ;
    sse_md5round_h xmm3,xmm0,xmm1,xmm2,8,11,33 ;
    sse_md5round_h xmm2,xmm3,xmm0,xmm1,11,16,34 ;
    sse_md5round_h xmm1,xmm2,xmm3,xmm0,14,23,35 ;
    sse_md5round_h xmm0,xmm1,xmm2,xmm3,1,4,36 ;
    sse_md5round_h xmm3,xmm0,xmm1,xmm2,4,11,37 ;
    sse_md5round_h xmm2,xmm3,xmm0,xmm1,7,16,38 ;
    sse_md5round_h xmm1,xmm2,xmm3,xmm0,10,23,39 ;
    sse_md5round_h xmm0,xmm1,xmm2,xmm3,13,4,40 ;
    sse_md5round_h xmm3,xmm0,xmm1,xmm2,0,11,41 ;
    sse_md5round_h xmm2,xmm3,xmm0,xmm1,3,16,42 ;
    sse_md5round_h xmm1,xmm2,xmm3,xmm0,6,23,43 ;
    sse_md5round_h xmm0,xmm1,xmm2,xmm3,9,4,44 ;
    sse_md5round_h xmm3,xmm0,xmm1,xmm2,12,11,45 ;
    sse_md5round_h xmm2,xmm3,xmm0,xmm1,15,16,46 ;
    sse_md5round_h xmm1,xmm2,xmm3,xmm0,2,23,47 ;
 
    ; Round 4
    sse_md5round_i xmm0,xmm1,xmm2,xmm3,0,6,48 ;
    sse_md5round_i xmm3,xmm0,xmm1,xmm2,7,10,49 ;
    sse_md5round_i xmm2,xmm3,xmm0,xmm1,14,15,50 ;
    sse_md5round_i xmm1,xmm2,xmm3,xmm0,5,21,51 ;
    sse_md5round_i xmm0,xmm1,xmm2,xmm3,12,6,52 ;
    sse_md5round_i xmm3,xmm0,xmm1,xmm2,3,10,53 ;
    sse_md5round_i xmm2,xmm3,xmm0,xmm1,10,15,54 ;
    sse_md5round_i xmm1,xmm2,xmm3,xmm0,1,21,55 ;
    sse_md5round_i xmm0,xmm1,xmm2,xmm3,8,6,56 ;
    sse_md5round_i xmm3,xmm0,xmm1,xmm2,15,10,57 ;
    sse_md5round_i xmm2,xmm3,xmm0,xmm1,6,15,58 ;
    sse_md5round_i xmm1,xmm2,xmm3,xmm0,13,21,59 ;
    sse_md5round_i xmm0,xmm1,xmm2,xmm3,4,6,60 ;
    sse_md5round_i xmm3,xmm0,xmm1,xmm2,11,10,61 ;
    sse_md5round_i xmm2,xmm3,xmm0,xmm1,2,15,62 ;
    sse_md5round_i xmm1,xmm2,xmm3,xmm0,9,21,63 ;
 
;//////////////////////////////////////////////////////////////////////////////
; //STEP 5: Output
 
; //Add initial values back to registers
paddd				xmm0,			[_MD5init_a]
paddd				xmm1,			[_MD5init_b]
paddd				xmm2,			[_MD5init_c]
paddd				xmm3,			[_MD5init_d]
 
; //and move to output
movaps				[rsp],				xmm0
movaps				[rsp+16],				xmm1
movaps				[rsp+32],				xmm2
movaps				[rsp+48],				xmm3
 
;	//Copy output buffers
mov   rbx, [ebp+12]
mov   rcx, [ebp+16]
;//	*reinterpret_cast<__int32*>(outbuf[0]+0) = oa[0];
;//*reinterpret_cast<__int32*>(outbuf[0]+4) = ob[0];
;//*reinterpret_cast<__int32*>(outbuf[0]+8) = oc[0];
;//*reinterpret_sscast<__int32*>(outbuf[0]+12) = od[0];
lea   rsi, [rbx]        
mov   rax, [rsp]
mov   [rsi], rax
mov   rax, [rsp+16]
mov   [rsi+4], rax
mov   rax, [rsp+32]
mov   [rsi+8], rax
mov   rax, [rsp+48]
mov   [rsi+12], rax
 
;//*reinterpret_cast<__int32*>(outbuf[1]+0) = oa[1];
;//*reinterpret_cast<__int32*>(outbuf[1]+4) = ob[1];
;//*reinterpret_cast<__int32*>(outbuf[1]+8) = oc[1];
;//*reinterpret_cast<__int32*>(outbuf[1]+12) = od[1];
lea   rsi, [rbx+16]
mov   rax,  [rsp+4]
mov   [rsi], rax
mov   rax,  [rsp+16+4]
mov   [rsi+4], rax
mov   rax,  [rsp+32+4]
mov   [rsi+8], rax
mov   rax,  [rsp+48+4]
mov   [rsi+12], rax
 
;//	*reinterpret_cast<__int32*>(outbuf[2]+0) = oa[2];
;//	*reinterpret_cast<__int32*>(outbuf[2]+4) = ob[2];
;//	*reinterpret_cast<__int32*>(outbuf[2]+8) = oc[2];
;//	*reinterpret_cast<__int32*>(outbuf[2]+12) = od[2];
lea   rsi, [rbx+16*2]
mov   rax, [rsp+8]
mov   [rsi], rax
mov   rax, [rsp+16+8]
mov   [rsi+4], rax
mov   rax, [rsp+32+8]
mov   [rsi+8], rax
mov   rax, [rsp+48+8]
mov   [rsi+12], rax
 
;//	*reinterpret_cast<__int32*>(outbuf[3]+0) = oa[3];
;//	*reinterpret_cast<__int32*>(outbuf[3]+4) = ob[3];
;//	*reinterpret_cast<__int32*>(outbuf[3]+8) = oc[3];
;//	*reinterpret_cast<__int32*>(outbuf[3]+12) = od[3];
lea   rsi, [rbx+16*3]
mov   rax, [rsp+12]
mov   [rsi], rax
mov   rax, [rsp+16+12]
mov   [rsi+4], rax
mov   rax, [rsp+32+12]
mov   [rsi+8], rax
mov   rax, [rsp+48+12]
mov   [rsi+12], rax
 
.end:
mov rsp, [ebp-16]
pop rbx
pop rsi
pop rdi
leave
ret
