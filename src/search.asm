;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                             ;
; Distributed Hash Cracker v3.0 DEFCON EDITION                                ;
;                                                                             ;
; Copyright (c) 2009-2010 RPISEC.                                             ;
; All rights reserved.                                                        ;
;                                                                             ;
; Redistribution and use in source and binary forms, with or without modifi-  ;
; cation, are permitted provided that the following conditions are met:       ;
;                                                                             ;
;    ; Redistributions of source code must retain the above copyright notice  ;
;      this list of conditions and the following disclaimer.                  ;
;                                                                             ;
;    ; Redistributions in binary form must reproduce the above copyright      ;
;      notice, this list of conditions and the following disclaimer in the    ;
;      documentation and/or other materials provided with the distribution.   ;
;                                                                             ;
;    ; Neither the name of RPISEC nor the names of its contributors may be    ;
;      used to endorse or promote products derived from this software without ;
;      specific prior written permission.                                     ;
;                                                                             ;
; THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY EXPRESS OR IMPLIED      ;
; WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        ;
; MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN     ;
; NO EVENT SHALL RPISEC BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  ;
; SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    ;
; TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      ;
; PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      ;
; LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        ;
; NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          ;
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                ;
;                                                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

bits		64
section		.text
default		rel

;%define takehint db 3eh
;%define skiphint db 2eh

;int HashSearch(unsigned int* hash /* rdi */, unsigned int* list /* rsi */, int count /* rdx */);
global HashSearch:function
HashSearch:
	mov		rcx, [rdi]			;r8, r9 = search hash
	mov		r9, [rdi + 8]

	mov		eax, edx			;count = upper bound
	dec		eax
.hashloop:
	lea		rdi, [rax*4]		;compute address
	lea		rdi, [rdi*4 + rsi]

	cmp		[rdi], rcx			;check first half
	jne		.unroll2			;if not found, skip
	cmp		[rdi + 8], r9		;check second half
	je		.hit
	
.unroll2:
	dec		eax
	cmp		[rdi - 16], rcx		;check first half
	jne		.next			;if not found, skip
	cmp		[rdi - 8], r9		;check second half
	je		.hit

.next:
	dec		eax
	jnz		.hashloop
	
	;still have to test last iteration at this point
	cmp		[rsi], rcx			;check first half
	jne		.miss				;if not found, skip
	cmp		[rsi + 8], r9		;check second half
	je		.hit

.miss:							;if no hit, fall through to here
	mov		rax, -1
	ret
.hit:
	ret
