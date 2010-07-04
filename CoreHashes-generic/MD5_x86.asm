;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Distributed Hash Cracker v2.0														;
;																					;
; Copyright (c) 2009 RPISEC															;
; All rights reserved.																;
;																					;
; Redistribution and use in source and binary forms, with or without				;
; modification, are permitted provided that the following conditions are met:		;
;     *Redistributions of source code must retain the above copyright				;
;      notice, this list of conditions and the following disclaimer.				;
;     *Redistributions in binary form must reproduce the above copyright			;
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
; MD5_x86.asm - x86 SSE implementation of MD5										;
;																					;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 
BITS 32
 
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
andps			xmm4,		%3				; xmm4 = b & c
movaps		xmm5,		%2				; xmm5 = b
andnps		xmm5,		%4				; xmm5 = ~b & d
orps			xmm4,		xmm5		; xmm4 = F*/		
 
movaps		xmm5,		[eax + 16*%5]		; prefetch x[i] 
paddd			%1,			xmm4				      ; a = a + F
paddd			%1,			[_MD5tbuf + 16*%7]	; a = a + F + T[i]*
paddd			%1,			xmm5				      ; a = a + F + T[i] + x[i]*
 
movaps		xmm4,		%1					        ; rotate a left shamt bits
pslld			%1,			%6		        								
psrld			xmm4,		(32-%6)									
orps			%1,			xmm4										
 
paddd			%1,			%2					        ; a = ROTL(~~~,N) + b) 
 
%endmacro
 
;//#define G(b,c,d) (((b) & (d)) | (~(d) & (c)))
%macro sse_md5round_g 7
movaps		xmm4,		%2					    ; xmm4 = b
andps			xmm4,		%4			  ; xmm4 = b & d
movaps		xmm5,		%4				; xmm5 = d
andnps		xmm5,		%3				; xmm5 = ~d & c
orps			xmm4,		xmm5		; xmm4 = G
movaps		xmm5,		[eax + 16*%5]		; prefetch x[i] 
paddd			%1,			xmm4				      ; a = a + F
paddd			%1,			[_MD5tbuf + 16*%7]	; a = a + F + T[i]*
paddd			%1,			xmm5				      ; a = a + F + T[i] + x[i]*
 
movaps		xmm4,		%1					        ; rotate a left shamt bits
pslld			%1,			%6		        								
psrld			xmm4,		(32-%6)									
orps			%1,			xmm4										
 
paddd			%1,			%2					        ; a = ROTL(~~~,N) + b) 
%endmacro
 
 
;//#define H(b,c,d) ((b) ^ (c) ^ (d))
;(a,b,c,d,index,shamt,stepnum) __asm							
%macro sse_md5round_h 7
movaps		xmm4,		%2			 ;  xmm4 = b
xorps			xmm4,		%3			 ;	xmm4 = b ^ c
xorps			xmm4,		%4			 ;	xmm4 = H
movaps		xmm5,		[eax + 16*%5]		; prefetch x[i] 
paddd			%1,			xmm4				      ; a = a + F
paddd			%1,			[_MD5tbuf + 16*%7]	; a = a + F + T[i]*
paddd			%1,			xmm5				      ; a = a + F + T[i] + x[i]*
 
movaps		xmm4,		%1					        ; rotate a left shamt bits
pslld			%1,			%6		        								
psrld			xmm4,		(32-%6)									
orps			%1,			xmm4										
 
paddd			%1,			%2					        ; a = ROTL(~~~,N) + b) 
%endmacro
 
;//#define I(b,c,d) ((c) ^ ((b) | ~(d)))
;(a,b,c,d,index,shamt,stepnum) 
%macro sse_md5round_i 7
movaps		xmm4,		%4					  ;xmm4 = d
xorps			xmm4,		[_MD5all_ones]		;xmm4 = ~d
orps			xmm4,		%2					  ;xmm4 = b | ~d
xorps			xmm4,		%3					  ;xmm4 = I
movaps		xmm5,		[eax + 16*%5]		; prefetch x[i] 
paddd			%1,			xmm4				      ; a = a + F
paddd			%1,			[_MD5tbuf + 16*%7]	; a = a + F + T[i]*
paddd			%1,			xmm5				      ; a = a + F + T[i] + x[i]*
 
movaps		xmm4,		%1					        ; rotate a left shamt bits
pslld			%1,			%6		        								
psrld			xmm4,		(32-%6)									
orps			%1,			xmm4										
 
paddd			%1,			%2					        ; a = ROTL(~~~,N) + b) 
%endmacro
 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;void MD5SSE2Hash(unsigned char* in,    unsigned char* out, int len);
global _MD5SSE2Hash:function
_MD5SSE2Hash:
  push ebp
  mov ebp, esp
  push edi
  push esi
  push ebx
  push esp
  and esp, 0xfffffff0
  sub esp, 320 
  ;if(len>50)
  ;	return;
 
  mov dword eax, [16+ebp]
  cmp eax, 50
  jb .ok
    mov esp, [ebp-16]
    pop ebx
    pop esi
    pop edi
    leave
    ret
.ok:
 
; interleave input 
; copy dword blocks first
; input[0] points to hash1 data, input[1] to hash2 data, ... 
; len = len & ~3
  mov ecx, [ebp+16]
  mov ebx, ecx
  and ecx, 0xfffffffc
  mov edx, [ebp+8]
  cmp ecx, 0
  je .endcopydata
  mov esi, esp
  add esi, 64
  lea esi, [esi+ecx*4]
  sub esi, 16
  shr ecx, 2
 
  .copydata:
    ; input[0]+0
    lea eax, [edx]
    mov eax, [edx + ecx*4 -4]
    mov [esi], eax
 
    ; input[1]+len
    lea eax, [edx+ebx*1]
    mov eax, [eax + ecx*4 -4]
    mov [esi+4], eax
 
    ; input[2][] +len*2
    lea eax, [edx+ebx*2]
    mov eax, [eax + ecx*4 -4]
    mov [esi+8], eax
 
    ; input[3][] + len*3
    lea eax, [edx+ebx*2]
    add eax, ebx
    mov eax, [eax + ecx*4 -4]
    mov [esi+12], eax
    sub esi, 16
  loop .copydata
 
.endcopydata:
; ///////////////////////////////////
; fill in remaining if any
  mov dword ebx, [ebp+16]	; ebx = len
  mov ecx, ebx
  and ecx, 3			; ecx = len & 3 (number of bytes to copy)
  and ebx, 0xfffffffc	; ebx = len & ~3 (bytes copied so far)
  lea esi, [esp + 64]	; esi = xbuf
  lea esi, [esi + ebx*4]; esi = xbuf + base*sizeof(DWORD). This is the last row that needs our data..
  mov dword [esi], 0	; Wipe any junk that might be sitting after the end of our last copy.
  mov dword [esi+4], 0
  mov dword [esi+8], 0
  mov dword [esi+12], 0
  push ebx				; save (len & ~3)
  mov dword ebx, [ebp+16]	; ebx = len
  cmp ecx, 0			; see if we need to do a byte copy
  je .step1				; if not, skip everything until step 1
  dec ecx
   .copybytes:
		;ecx = byte being copied
		;ebx = len
	
	lea edi, [ecx + edx]	;edi = &in[iByte]
	add edi, [esp]			;edi = &in[base + iByte]

	mov byte al, [edi]		; Copy from in[base + iByte]
	mov byte [esi+ecx], al	
	
	mov byte al, [edi + ebx] ; Copy from in[base + len + iByte]
	mov byte [esi+ecx+4], al
	
	mov byte al, [edi + 2*ebx]	; Copy from in[base + 2*len + iByte]
	mov byte [esi+ecx+8], al
	
	lea edi, [edi + 2*ebx]		;no way to easily add 3*len
	mov byte al, [edi + ebx]	; Copy from in[base + 3*len + iByte]
	mov byte [esi+ecx+12], al
    
	dec ecx
    cmp ecx, 0
  jge .copybytes 
 
; STEP 1
;appending padding
.step1:
; '1' bit goes @ (len&~3)*4 + (len&3) + xbuf
  pop ecx				;ecx = len & ~3
  and ebx, 3			;ebx = len & 3

  mov eax, ebx
  sal ecx, 2
  add eax, ecx
 
  mov esi, esp
  add esi, 64
  mov byte [esi+eax], 0x80
  mov byte [esi+eax+4], 0x80
  mov byte [esi+eax+8], 0x80
  mov byte [esi+eax+12], 0x80
  add eax, 16
  ; append '0' bits up until 64-bits from a 512 bit boundary (56 bytes from xbuf)
 
  lea ecx, [esi+224] ; ecx is the end boundary for the '0' bits
  lea ebx, [esi+eax]
  and ebx, 0xfffffffc
  .zeropad:
    mov dword [ebx], 0
    mov dword [ebx+4], 0
    mov dword [ebx+8], 0
    mov dword [ebx+12], 0
    add ebx, 16
    cmp ebx, ecx
  jne .zeropad
 
.step2:
; //STEP 2: Append Length - length is a 64-bit little endian number
  mov eax, [ebp+16]
  sal eax, 3
  mov esi, esp
  add esi, 64
  mov dword [esi+56*4], eax
  mov dword [esi+57*4], eax
  mov dword [esi+58*4], eax
  mov dword [esi+59*4], eax
; add NULL most significant words
  mov dword [esi+60*4], 0
  mov dword [esi+61*4], 0
  mov dword [esi+62*4], 0
  mov dword [esi+63*4], 0
 
  mov eax, esp
  add eax, 64
 
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
movaps				[esp],				xmm0
movaps				[esp+16],				xmm1
movaps				[esp+32],				xmm2
movaps				[esp+48],				xmm3
 
;	//Copy output buffers
mov   ebx, [ebp+12]
mov   ecx, [ebp+16]
;//	*reinterpret_cast<__int32*>(outbuf[0]+0) = oa[0];
;//*reinterpret_cast<__int32*>(outbuf[0]+4) = ob[0];
;//*reinterpret_cast<__int32*>(outbuf[0]+8) = oc[0];
;//*reinterpret_sscast<__int32*>(outbuf[0]+12) = od[0];
lea   esi, [ebx]        
mov   eax, [esp]
mov   [esi], eax
mov   eax, [esp+16]
mov   [esi+4], eax
mov   eax, [esp+32]
mov   [esi+8], eax
mov   eax, [esp+48]
mov   [esi+12], eax
 
;//*reinterpret_cast<__int32*>(outbuf[1]+0) = oa[1];
;//*reinterpret_cast<__int32*>(outbuf[1]+4) = ob[1];
;//*reinterpret_cast<__int32*>(outbuf[1]+8) = oc[1];
;//*reinterpret_cast<__int32*>(outbuf[1]+12) = od[1];
lea   esi, [ebx+16]
mov   eax,  [esp+4]
mov   [esi], eax
mov   eax,  [esp+16+4]
mov   [esi+4], eax
mov   eax,  [esp+32+4]
mov   [esi+8], eax
mov   eax,  [esp+48+4]
mov   [esi+12], eax
 
;//	*reinterpret_cast<__int32*>(outbuf[2]+0) = oa[2];
;//	*reinterpret_cast<__int32*>(outbuf[2]+4) = ob[2];
;//	*reinterpret_cast<__int32*>(outbuf[2]+8) = oc[2];
;//	*reinterpret_cast<__int32*>(outbuf[2]+12) = od[2];
lea   esi, [ebx+16*2]
mov   eax, [esp+8]
mov   [esi], eax
mov   eax, [esp+16+8]
mov   [esi+4], eax
mov   eax, [esp+32+8]
mov   [esi+8], eax
mov   eax, [esp+48+8]
mov   [esi+12], eax
 
;//	*reinterpret_cast<__int32*>(outbuf[3]+0) = oa[3];
;//	*reinterpret_cast<__int32*>(outbuf[3]+4) = ob[3];
;//	*reinterpret_cast<__int32*>(outbuf[3]+8) = oc[3];
;//	*reinterpret_cast<__int32*>(outbuf[3]+12) = od[3];
lea   esi, [ebx+16*3]
mov   eax, [esp+12]
mov   [esi], eax
mov   eax, [esp+16+12]
mov   [esi+4], eax
mov   eax, [esp+32+12]
mov   [esi+8], eax
mov   eax, [esp+48+12]
mov   [esi+12], eax
 
.end:
mov esp, [ebp-16]
pop ebx
pop esi
pop edi
leave
ret
