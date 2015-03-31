f#include "spider.h"
#include "fs.h"

#define BUFFER_LOC		0x18410000
#define CODE_OFFSET		0x14000000
#define ARCODE_POS		0x00000001

void CopyMem(void *src, void *dst){
	GSPGPU_FlushDataCache(src, 0x20);
	GX_SetTextureCopy(src, dst, 0x20, 0, 0, 0, 0, 8);
	GSPGPU_FlushDataCache(dst, 0x20);
	svcSleepThread(0x200000LL);
}

unsigned Read(unsigned Offset, unsigned Mask) 
{ 
	CopyMem((void*)Offset, (void*)BUFFER_LOC);
	return *(unsigned*)(BUFFER_LOC + (Offset & 0x0000000F)) & Mask;
} 

void Write(unsigned Offset, unsigned Data, unsigned Mask) 
{ 
	CopyMem((void*)Offset, (void*)BUFFER_LOC);
	*(unsigned*)(BUFFER_LOC + (Offset & 0x0000000F)) &= ~Mask; 
	*(unsigned*)(BUFFER_LOC + (Offset & 0x0000000F)) |= Data & Mask; 
	CopyMem((void*)BUFFER_LOC, (void*)Offset); 
} 

int uvl_entry(){
	unsigned *arcode = (unsigned*)L"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	unsigned Index = 0;
	unsigned WordCount;
	unsigned Type;
	unsigned First8;
	unsigned Second8;
	unsigned CodeOffset = CODE_OFFSET;
	unsigned CodeData;
	unsigned Offset;
	unsigned Data;
	unsigned RepeatCount;
	unsigned RepeatStart;
	unsigned Mask;
	unsigned Increment;

	WordCount = arcode[Index++] << 1;

	while (Index < WordCount){
		First8 = arcode[Index++];
		Second8 = arcode[Index++];
		Offset = CodeOffset + (First8 & 0x0FFFFFFF);
		Mask = 0xFFFFFFFF;
		Increment = 4;
		switch (Type = First8 & 0xF0000000){
			case 0x20000000://8-bit Write
				Mask >>= 8;
			case 0x10000000://16-bit Write
				Mask >>= 16;
			case 0x00000000://32-bit Write
				Write(Offset, Second8, Mask);
				break;
			case 0x70000000://16-bit Greater Than
			case 0x80000000://16-bit Less Than
			case 0x90000000://16-bit Equal To
			case 0xA0000000://16-bit Not Equal To
				Mask &= ~Second8 >> 16;
				Second8 &= 0x0000FFFF;
			case 0x30000000://32-bit Greater Than
			case 0x40000000://32-bit Less Than
			case 0x50000000://32-bit Equal To
			case 0x60000000://32-bit Not Equal To
				Data = Read(Offset, Mask);
				Type &= 0x30000000;
				if (Type == 0x30000000 && Second8 <= Data || Type == 0x00000000 && Second8 >= Data || Type == 0x10000000 && Second8 != Data || Type == 0x20000000 && Second8 == Data){
					while (First8 != 0xD0000000 && First8 != 0xD2000000 && Index < WordCount){
						First8 = arcode[Index++];
						if ((First8 & 0xF0000000) == 0xE00000000){
							Index += (arcode[Index++] + 7) >> 3 << 1;
							First8 = arcode[Index++];
						}
						Index++;
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
				if ((CodeOffset = Read(Offset, Mask)) < CODE_OFFSET){
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
					case 0x08000000://8-Bit Incrementive Write
						Mask >>= 8;
						Increment >>= 1;
					case 0x07000000://16-Bit Incrementive Write
						Mask >>= 16;
						Increment >>= 1;
					case 0x06000000://32-Bit Incrementive Write
						Write(Offset, CodeData, Mask);
						CodeOffset += Increment;
						break;
					case 0x0B000000://8-Bit Load
						Mask >>= 8;
					case 0x0A000000://16-Bit Load
						Mask >>= 16;
					case 0x09000000://32-Bit Load
						CodeData = Read(Offset, Mask);
						break;
 					case 0x0C000000://Add Offset
						CodeOffset += Second8;
						break;
				}
				break;
			case 0xE0000000://Patch Code
				while (Second8 > 0 && Index < WordCount){
					First8 = arcode[Index++];
					if (Second8 >= 4){
						Write(Offset, First8, Mask);
						Offset += Increment;
						Second8 -= Increment;
						First8 = arcode[Index];
					}
					Index++;
					if (Second8 < 4){
						Increment = Second8;
						Mask = ~(Mask << (Second8 << 3));
					}
					Write(Offset, First8, Mask);
					Offset += Increment;
					Second8 -= Increment;
				}
				break;
			case 0xF0000000://Memory Copy Code
				Offset = CODE_OFFSET + (First8 & 0x0FFFFFFF);
				unsigned tempoffset = CodeOffset;
				while (Second8 > 0){
					if (Second8 < 4){
						Increment = Second8;
						Mask = ~(Mask << (Second8 << 3));
					}
					Write(Offset, Read(tempoffset, Mask), Mask);
					tempoffset += Increment;
					Offset += Increment;
					Second8 -= Increment;
				}
			break;
		}
	}
	return 0;
}
