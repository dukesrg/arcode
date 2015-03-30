#include "spider.h"
#include "fs.h"

#define BUFFER		((void*)0x18410000)
#define CODE_OFFSET			0x14000000
#define ARCODE_POS			0x00000001

void CopyMem(void *src, void *dst){
	GSPGPU_FlushDataCache(src, 0x20);
	GX_SetTextureCopy(src, dst, 0x20, 0, 0, 0, 0, 8);
	GSPGPU_FlushDataCache(dst, 0x20);
	svcSleepThread(0x200000LL);
}

void Write(unsigned int offset, unsigned char data){
	CopyMem((void *)offset, BUFFER);
	*((unsigned char*)BUFFER + (offset & 0x0F)) = data;
	CopyMem(BUFFER, (void *)offset);
}

void Write(unsigned int offset, unsigned short data){
	CopyMem((void *)offset, BUFFER);
	*(unsigned short*)((unsigned char*)BUFFER + (offset & 0x0F)) = data;
	CopyMem(BUFFER, (void *)offset);
}

void Write(unsigned int offset, unsigned int data){
	CopyMem((void *)offset, BUFFER);
	*(unsigned int*)((unsigned char*)BUFFER + (offset & 0x0F)) = data;
	CopyMem(BUFFER, (void *)offset);
}

unsigned int Read(unsigned int offset){
	CopyMem((void*)Offset, BUFFER);
	return *(unsigned int*)((unsigned char*)BUFFER + (offset & 0x0F));
}

int uvl_entry(){
	unsigned int *arcode = (unsigned int *)L"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	unsigned int Index = 0;
	unsigned int WordCount;
	unsigned int Type;
	unsigned int First8;
	unsigned int Second8;
	unsigned int CodeOffset = CODE_OFFSET;
	unsigned int CodeData;
	unsigned int Offset;
	unsigned int Data;
	unsigned int RepeatCount;
	unsigned int RepeatStart;

	WordCount = arcode[Index++] << 1;

	while (Index < WordCount){
		First8 = arcode[Index++];
		Second8 = arcode[Index++];
		Offset = CodeOffset + (First8 & 0x0FFFFFFF);
		switch (Type = First8 & 0xF0000000){
			case 0x00000000://32-bit Write
				Write(Offset, Second8);
				break;
			case 0x10000000://16-bit Write
				Write(Offset, (unsigned short)Second8);
				break;
			case 0x20000000://8-bit Write
				Write(Offset, (unsigned char)Second8);
				break;
			case 0x70000000://16-bit Greater Than
			case 0x80000000://16-bit Less Than
			case 0x90000000://16-bit Equal To
			case 0xA0000000://16-bit Not Equal To
				Second8 &= ~Second8 >> 16;
			case 0x30000000://32-bit Greater Than
			case 0x40000000://32-bit Less Than
			case 0x50000000://32-bit Equal To
			case 0x60000000://32-bit Not Equal To
				Data = Read(Offset);
				if (Type >= 0x70000000){
					Data = (unsigned short)Data;
				}
				Type &= 0x30000000;
				if (Type == 0x30000000 && Second8 <= Data || Type == 0x00000000 && Second8 >= Data || Type == 0x10000000 && Second8 != Data || Type == 0x20000000 && Second8 == Data){
					while (First8 != 0xD0000000 && First8 != 0xD2000000 && Index < WordCount){
						First8 = arcode[Index];
						Index += 2;
					}
					if (First8 == 0xD2000000){
						if (RepeatCount > 0){
							RepeatCount--;
							Index = ARCODE_POS + (RepeatStart << 1);
						}
						CodeOffset = CODE_OFFSET;
						CodeData = 0;
					}
				}
				break;
			case 0xB0000000://Load Offset
				if ((CodeOffset = Read(Offset)) < CODE_OFFSET){
					CodeOffset += CODE_OFFSET;
				}
				break;
			case 0xC0000000://Loop Code
				RepeatCount = Second8;
				RepeatStart = (Index - ARCODE_POS) >> 1;
				break;
			case 0xD0000000://Various
				Offset = CodeOffset + Second8;
				switch (First8 & 0x0F000000){
					case 0x01000000://Loop execute variant
						if (RepeatCount > 0){
							RepeatCount--;
							Index = ARCODE_POS + (RepeatStart << 1);
						}
						break;
					case 0x02000000://Loop Execute Variant / Full Terminator
						if (RepeatCount > 0){
							RepeatCount--;
							Index = ARCODE_POS + (RepeatStart << 1);
						}
						CodeOffset = CODE_OFFSET;
						CodeData = 0;
						break;
					case 0x03000000://Set Offset
						if ((CodeOffset = Second8) < CODE_OFFSET){
								CodeOffset += CODE_OFFSET;
						}
						break;
					case 0x04000000://Add Value
						CodeData += Second8;
						break;
					case 0x05000000://Set Value
						CodeData = Second8;
						break;
					case 0x06000000://32-Bit Incrementive Write
						Write(Offset, CodeData);
						CodeOffset += 4;
						break;
					case 0x07000000://16-Bit Incrementive Write
						Write(Offset, (unsigned short)CodeData);
						CodeOffset += 2;
					case 0x08000000://8-Bit Incrementive Write
						Write(Offset, (unsigned char)CodeData);
						CodeOffset++ ;
						break;
					case 0x09000000://32-Bit Load
						CodeData = Read(Offset);
						break;
					case 0x0A000000://16-Bit Load
						CodeData = (unsigned short)Read(Offset);
						break;
					case 0x0B000000://8-Bit Load
						CodeData = (unsigned char)Read(Offset);
						break;
 					case 0x0C000000://Add Offset
						CodeOffset += Second8;
						break;
				}
				break;
			case 0xE0000000://Patch Code
				while (Second8 > 0 && Index < WordCount){
					First8 = arcode[Index++];
					if (Second8 >= 8){
						Write(Offset, First8);
						Offset += 4;
						Write(Offset, arcode[Index++]);
						Offset += 4;
						Second8 -= 8;
					}else{
						if (Second8 >= 4){
							Write(Offset, First8);
							Offset += 4;
							Second8 -= 4;
							First8 = arcode[Index];
						}
						Index++;
						if (Second8 >= 2){
							Write(Offset, (unsigned short)First8);
							Offset += 2;
							Second8 -= 2;
							First8 >>= 16;
						}
						if (Second8 == 1){
							Write(Offset, (unsigned char)First8);
							Offset++;
							Second8--;
						}
					}
				}
				break;
			case 0xF0000000://Memory Copy Code
				Offset = CODE_OFFSET + (First8 & 0x0FFFFFFF);
				unsigned int tempoffset = CodeOffset;
				while (Second8 >= 4){
						Write(Offset, Read(tempoffset));
						tempoffset += 4;
						Offset += 4;
						Second8 -= 4;
				}
				if (Second8 >= 2){
					Write(Offset, (unsigned short)Read(tempoffset));
					tempoffset += 2;
					Offset += 2;
					Second8 -= 2;
				}
				if (Second8 == 1){
					Write(Offset, (unsigned char)Read(tempoffset));
				}
			break;
		}
	}
	return 0;
}
