#include "spider.h"
#include "fs.h"
#ifdef print
#include "print.h"
#endif

#define CODE_OFFSET	0x14000000
#define ARCODE_POS	0x00000001
#define BUF_SIZE	0x00000020
#define SLEEP_DEFAULT	0x200000LL
#define MEM_OFFS_REF	0x18410000
#define MEM_WRITE_REF	0x18410004
#define BUF_LOC		0x18410010
#define WRITE_BACK_ALL	0xFFFFFFFF

void CopyMem(void *src, void *dst, unsigned size, unsigned long long sleep){
	GSPGPU_FlushDataCache(src, size);
	GX_SetTextureCopy(src, dst, size, 0, 0, 0, 0, 8);
	GSPGPU_FlushDataCache(dst, size);
	svcSleepThread(sleep);
}

void WriteBack(unsigned Offset, unsigned Size)
{
	if((Offset < *(unsigned*)MEM_OFFS_REF) || (*(unsigned*)MEM_OFFS_REF + BUF_SIZE - Size < Offset)){
		if(*(unsigned*)MEM_WRITE_REF == 1){ 
			CopyMem((void*)BUF_LOC, (void *)(*(unsigned*)MEM_OFFS_REF), BUF_SIZE, SLEEP_DEFAULT); 
			*(unsigned*)MEM_WRITE_REF = 0; 
		} 
		CopyMem((void*)Offset, (void*)BUF_LOC, BUF_SIZE, SLEEP_DEFAULT); 
		*(unsigned*)MEM_OFFS_REF = Offset & 0xFFFFFFF0;
	}
}

void Write32(unsigned Offset, unsigned Data)
{
	WriteBack(Offset, sizeof(Data));
	*(unsigned*)(BUF_LOC + Offset - *(unsigned*)MEM_OFFS_REF) = Data;
	*(unsigned*)MEM_WRITE_REF = 1;
}

void Write16(unsigned int Offset, unsigned short Data)
{
	WriteBack(Offset, sizeof(Data));
	*(unsigned short*)(BUF_LOC + Offset - *(unsigned*)MEM_OFFS_REF) = Data;
	*(unsigned*)MEM_WRITE_REF = 1;
}

void Write8(unsigned int Offset, unsigned char Data)
{
	WriteBack(Offset, sizeof(Data));
	*(unsigned char*)(BUF_LOC + Offset - *(unsigned*)MEM_OFFS_REF) = Data;
	*(unsigned*)MEM_WRITE_REF = 1;
}

unsigned int Read32(unsigned int Offset)
{
	WriteBack(Offset, sizeof(unsigned));
	return *(unsigned*)(BUF_LOC + Offset - *(unsigned*)MEM_OFFS_REF);
}

unsigned short Read16(unsigned Offset)
{
	WriteBack(Offset, sizeof(unsigned short));
	return *(unsigned short*)(BUF_LOC + Offset - *(unsigned*)MEM_OFFS_REF);
}

unsigned char Read8(unsigned Offset)
{
	WriteBack(Offset, sizeof(unsigned char));
	return *(unsigned char*)(BUF_LOC + Offset - *(unsigned*)MEM_OFFS_REF);
}

int uvl_entry()
{
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
	unsigned int Mask;

	WordCount = arcode[Index++] << 1;

	while (Index < WordCount)
	{
		First8 = arcode[Index++];
		Second8 = arcode[Index++];
#ifdef print
		print("\f\v", COLOR_TRANS, COLOR_TRANS);
		print(" Code string:\r\n\n Code word 1:\r\n\n Code word 2:\r\n\n Offset value:\r\n\n Dx data value:\r\n\n Dx repeat value:", COLOR_FG_DEF, COLOR_BG_DEF);
		print("\f\v\t\t\t", COLOR_TRANS, COLOR_TRANS);
		printint(Index>>1, COLOR_FG_DEF, COLOR_BG_DEF);
		print("\r\n\n\t\t\t", COLOR_TRANS, COLOR_TRANS);
		printint(First8, COLOR_FG_DEF, COLOR_BG_DEF);
		print("\r\n\n\t\t\t", COLOR_TRANS, COLOR_TRANS);
		printint(Second8, COLOR_FG_DEF, COLOR_BG_DEF);
		print("\r\n\n\t\t\t", COLOR_TRANS, COLOR_TRANS);
		printint(Offset, COLOR_FG_DEF, COLOR_BG_DEF);
		print("\r\n\n\t\t\t", COLOR_TRANS, COLOR_TRANS);
		printint(Data, COLOR_FG_DEF, COLOR_BG_DEF);
		print("\r\n\n\t\t\t", COLOR_TRANS, COLOR_TRANS);
		printint(RepeatCount, COLOR_FG_DEF, COLOR_BG_DEF);
#endif
		Offset = CodeOffset + (First8 & 0x0FFFFFFF);
		Mask = 0xFFFFFFFF;
		switch (Type = First8 & 0xF0000000)
		{
			case 0x00000000://32-bit Write
				Write32(Offset, Second8);
				break;
			case 0x10000000://16-bit Write
				Write8(Offset, Second8);
				break;
			case 0x20000000://8-bit Write
				Write16(Offset, Second8);
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
				Type &= 0x30000000;
				Data = Read32(Offset) & Mask;
				if (Type == 0x30000000 && Second8 <= Data || Type == 0x00000000 && Second8 >= Data || Type == 0x10000000 && Second8 != Data || Type == 0x20000000 && Second8 == Data)
				{
					while (First8 != 0xD0000000 && First8 != 0xD2000000 && Index < WordCount)
					{
						First8 = arcode[Index++];
						if ((First8 & 0xF0000000) == 0xE00000000){
							Index += (arcode[Index++] + 7) >> 3 << 1;
							First8 = arcode[Index++];
						}
						Index++;
					}
					if (First8 == 0xD2000000)
					{
						Index -= 2;
					}
				}
				break;
			case 0xB0000000://Load Offset
				if ((CodeOffset = Read32(Offset)) < CODE_OFFSET)
				{
					CodeOffset += CODE_OFFSET;
				}
				break;
			case 0xC0000000://Loop Code
				RepeatCount = Second8;
				RepeatStart = (Index - ARCODE_POS) >> 1;
				break;
			case 0xD0000000://Various
				Offset = CodeOffset + Second8;
				switch (First8 & 0x0F000000)
				{
					case 0x01000000://Loop execute variant
						if (RepeatCount > 0)
						{
							RepeatCount--;
							Index = ARCODE_POS + (RepeatStart << 1);
						}
						break;
					case 0x02000000://Loop Execute Variant / Full Terminator
						if (RepeatCount > 0)
						{
							RepeatCount--;
							Index = ARCODE_POS + (RepeatStart << 1);
						}
						else
						{
							CodeOffset = CODE_OFFSET;
							CodeData = 0;
						}
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
						Write32(Offset, CodeData);
						CodeOffset += 4;
						break;
					case 0x07000000://16-Bit Incrementive Write
						Write16(Offset, CodeData);
						CodeOffset += 2;
						break;
					case 0x08000000://8-Bit Incrementive Write
						Write8(Offset, CodeData);
						CodeOffset++;
						break;
					case 0x09000000://32-Bit Load
						CodeData = Read32(Offset);
						break;
					case 0x0A000000://16-Bit Load
						CodeData = Read16(Offset);
						break;
					case 0x0B000000://8-Bit Load
						CodeData = Read8(Offset);
						break;
 					case 0x0C000000://Add Offset
						CodeOffset += Second8;
						break;
				}
				break;
			case 0xE0000000://Patch Code
				Data = Second8;
				while (Data > 0 && Index < WordCount)
				{
					First8 = arcode[Index++];
					Second8 = arcode[Index++];

					if (Data >= 8)//Double 32bit
					{
						Write32(Offset, First8);
						Offset += 4;
						Write32(Offset, Second8);
						Offset += 4;
						Data -= 8;
					}
					else
					{
						if (Data >= 4)//32bit
						{
							Write32(Offset, First8);
							Offset += 4;
							Data -= 4;
							First8 = Second8;
						}
						if (Data >= 2)//16bit
						{
							Write16(Offset, First8);
							Offset += 2;
							Data -= 2;
							First8 >>= 16;
						}
						if (Data == 1)//8bit
						{
							Write8(Offset, First8);
							Data--;
						}
					}
				}
				break;
			case 0xF0000000://Memory Copy Code
				Offset = CODE_OFFSET + (First8 & 0x0FFFFFFF);
				unsigned int tempoffset = CodeOffset;
				while (Second8 > 0)
				{
					if (Second8 >= 4)//32bit
					{
						Write32(Offset, Read32(tempoffset));
						tempoffset += 4;
						Offset += 4;
						Second8 -= 4;
					}
					else if (Second8 >= 2)//16bit
					{
						Write16(Offset, Read16(tempoffset));
						tempoffset += 2;
						Offset += 2;
						Second8 -= 2;
					}
					else//8bit
					{
						Write8(Offset, Read8(tempoffset));
						tempoffset += 1;
						Offset += 1;
						Second8 -= 1;
					}
				}
			break;
		}
	}
	WriteBack(WRITE_BACK_ALL, 0);
#ifdef print
	print("\r\nDone!", COLOR_FG_DEF, COLOR_TRANS);
#endif
	return 0;
}
