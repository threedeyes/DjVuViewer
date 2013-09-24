BITS 32

True EQU -1
False EQU 0

;//Exports

GLOBAL AsmRender

;//Data

SEGMENT .data align=16

;//Code
SkyLine1 dd  0
SkyLine2 dd  0

yk1			 dd  0
yk2			 dd  0

SEGMENT .text align=16


%define scrBuff    [ebp+08h]
%define SkyBmp     [ebp+0Ch]

%define D					 [ebp+10h]

%define H1				 [ebp+14h]
%define k1				 [ebp+18h]
%define dX1				 [ebp+1Ch]
%define dY1				 [ebp+20h]

%define H2				 [ebp+24h]
%define k2				 [ebp+28h]
%define dX2				 [ebp+2Ch]
%define dY2				 [ebp+30h]


%define Horiz			 [ebp+34h]

%define SkyW			 [ebp+38h]
%define SkyH			 [ebp+3Ch]
%define SkrW			 [ebp+40h]
%define SkrWHalf	 [ebp+44h]

ALIGN 16

AsmRender:

push ebp
mov ebp,esp
pushad
pushfd
	mov edi,scrBuff
	cld

	mov ecx,Horiz
by_y:
	push ecx
;---------------------------------H1----------------------------------
		mov eax,ecx				
		add eax,10
		mul dword k1			; y * k1
		mov ebx,eax
		mov [yk1],eax

		mov eax,D					; D * H1
		mul dword H1

		xor edx,edx	
		div ebx						; (D*H1)/(y*k1)
		add eax,dY1				; yshift1 + (D*H1)/(y*k1)
	
		xor edx,edx	
		div dword SkyH		; (yshift1 + (D*H1)/(y*k1)) % SkyH
		mov eax,edx

		xor edx,edx	
		mul dword SkyW		; ((yshift1 + (D*H1)/(y*k1)) % SkyH) * SkyW
		shl eax,2
		add eax,SkyBmp
		mov [SkyLine1],eax
;---------------------------------H2---------------------------------
		mov eax,ecx				
		add eax,10
		mul dword k2			; y * k2
		mov ebx,eax
		mov [yk2],eax

		mov eax,D					; D * H2
		mul dword H2

		xor edx,edx	
		div ebx						; (D*H2)/(y*k2)
		add eax,dY2				; yshift2 + (D*H2)/(y*k2)
	
		xor edx,edx	
		div dword SkyH		; (yshift2 + (D*H2)/(y*k2)) % SkyH
		mov eax,edx

		xor edx,edx	
		mul dword SkyW		; ((yshift2 + (D*H2)/(y*k2)) % SkyH) * SkyW
		shl eax,2
		add eax,SkyBmp
		mov [SkyLine2],eax
;-------------------------------------------------------------------
		
		 mov ecx,SkrWHalf
		 neg ecx
by_x:
;-------------------------------------------------------------------
		 mov eax,ecx
		 imul dword H1
		 idiv dword [yk1]
		 add eax,dword dX1

		 jge x_positive
		 mov ebx,SkyW
		 sub ebx,eax
		 mov eax,ebx
x_positive:		 
		 xor edx,edx
		 div dword SkyW

		 shl edx,2		 
		 mov esi,[SkyLine1]
		 add esi,edx 
;-------------------------------------------------------------------
		 mov eax,ecx
		 imul dword H2
		 idiv dword [yk2]
		 add eax,dword dX2
		 jge x_positive2
		 mov ebx,SkyW
		 sub ebx,eax
		 mov eax,ebx
x_positive2:		 
		 xor edx,edx
		 div dword SkyW

		 shl edx,2		 

		 mov ebx,[SkyLine2]
		 add ebx,edx 		 
;-------------------------------------------------------------------

	   mov al,byte [esi]
	   shr al,1
	   mov dl,byte [ebx]
	   shr dl,1
	   add al,dl
	   stosb
	   inc ebx
	   inc esi
	   
	   mov al,byte [esi]
	   shr al,1
	   mov dl,byte [ebx]
	   shr dl,2
	   add al,dl
	   stosb
	   inc ebx
	   inc esi

	   mov al,byte [esi]
	   shr al,1
	   mov dl,byte [ebx]
	   shr dl,1
	   add al,dl
	   stosb
	   inc edi
	   
 		 inc ecx
 		 cmp ecx,SkrWHalf
		 jl by_x

  pop ecx		
  dec cx
  jz AllY	
	jmp by_y
AllY:

popfd
popad
pop ebp
ret

end
