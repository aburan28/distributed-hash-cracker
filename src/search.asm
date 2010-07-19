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

;int HashSearch(unsigned int* hash /* rdi */, unsigned int* list /* rsi */, int count /* rdx */);
global HashSearch:function
HashSearch:
	mov		r8, [rdi]			;r8, r9 = search hash
	mov		r9, [rdi + 8]

	mov		rax, rsi			;save orig list ptr
	
	sal		rdx, 4				;count*16 = max byte offset
	add		rdx, rsi			;rdx = max offset
	
.hashloop:
	cmp		[rsi], r8			;check first half
	jne		.next				;if not found, skip
	cmp		[rsi + 8], r9
	je		.hit

.next:
	add		rsi, 16				;go to next hash
	cmp		rsi, rdx			;we done yet?
	jl		.hashloop

.miss:							;if no hit, fall through to here
	mov		rax, -1
	ret
.hit:
	sub		rsi, rax			;final pointer - offset
	sar		rsi, 4				;divide by 16 to get index
	mov		rax, rsi
	ret
