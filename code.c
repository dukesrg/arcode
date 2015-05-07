#include "spider.h"
#include "fs.h"

#define CODE_OFFSET	0x14000000
#define ARCODE_POS	0x00000004
#define BUF_SIZE	0x00000800
#define SLEEP_DEFAULT	0x000000LL
#define MEM_OFFS_REF	0x18410000
#define MEM_WRITE_REF	0x18410004
#define BUF_LOC		0x18410010
#define WRITE_BACK_ALL	0xFFFFFFFF

void Write32(unsigned int Offset, unsigned Data);
void Write16(unsigned int Offset, unsigned short Data);
void Write8(unsigned int Offset, unsigned char Data);
void WriteBack(unsigned int Offset, unsigned Size);
unsigned Read32(unsigned int Offset);
unsigned short Read16(unsigned int Offset);
unsigned char Read8(unsigned int Offset);

unsigned int *buf = (unsigned int *)0x18410008;

int uvl_entry()
{
	FILE *fin = (void *)0x08F10000;
	int *read_len = (int *)0x08F10020;
	unsigned int LineCount = 0;
	unsigned int ProcessedLines = 0;
	unsigned int Type=0;
	unsigned int First8 = 0;
	unsigned int Second8 = 0;
	unsigned int CodeOffset = CODE_OFFSET;
	unsigned int CodeData = 0;
	unsigned int Offset = 0;
	unsigned int Data = 0;
	unsigned int RepeatCount = 0;
	unsigned int RepeatStart = 0;
	*(unsigned*)MEM_OFFS_REF = WRITE_BACK_ALL;
	*(unsigned*)MEM_WRITE_REF = 0;

	IFile_Open(fin, L"dmc:/arcode.cht\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", FILE_R);
	fin->pos = 0x00;
	IFile_Read(fin, read_len, buf, 0x04);
	LineCount = buf[0];

	while (ProcessedLines < LineCount)
	{
		IFile_Read(fin, read_len, buf, 0x08);
		First8 = buf[0];
		Second8 = buf[1];
		ProcessedLines++;
		Offset = CodeOffset + (First8 & 0x0FFFFFFF);
		switch (Type = First8 & 0xF0000000)
		{
			case 0x00000000://32-bit Write
				Write32(Offset, Second8);
				break;
			case 0x10000000://16-bit Write
				Write16(Offset, Second8);
				break;
			case 0x20000000://8-bit Write
				Write8(Offset, Second8);
				break;
			case 0x30000000://32-bit Greater Than
			case 0x40000000://32-bit Less Than
			case 0x50000000://32-bit Equal To
			case 0x60000000://32-bit Not Equal To
			case 0x70000000://16-bit Greater Than
			case 0x80000000://16-bit Less Than
			case 0x90000000://16-bit Equal To
			case 0xA0000000://16-bit Not Equal To
				Data = Read32(Offset);
				if(
					((Type == 0x30000000) && (Second8 <= Data)) ||
					((Type == 0x40000000) && (Second8 >= Data)) ||
					((Type == 0x50000000) && (Second8 != Data)) ||
					((Type == 0x60000000) && (Second8 == Data)) ||
					((Type == 0x70000000) && ((Second8 & 0x0000FFFF) <= (Data & 0x0000FFFF & ~Second8 >> 16 ))) ||
					((Type == 0x80000000) && ((Second8 & 0x0000FFFF) >= (Data & 0x0000FFFF & ~Second8 >> 16))) ||
					((Type == 0x90000000) && ((Second8 & 0x0000FFFF) != (Data & 0x0000FFFF & ~Second8 >> 16))) ||
					((Type == 0xA0000000) && ((Second8 & 0x0000FFFF) == (Data & 0x0000FFFF & ~Second8 >> 16)))
				){
					while ((First8 != 0xD0000000) && (First8 != 0xD2000000) && (ProcessedLines < LineCount))
					{
						IFile_Read(fin, read_len, buf, 0x08);
						First8 = buf[0];
						Second8 = buf[1];
						ProcessedLines++;
						if ((First8 & 0xF0000000) == 0xE00000000){
							ProcessedLines += Second8 + 7 >> 3;
							fin->pos = ARCODE_POS + (ProcessedLines << 3);
							IFile_Read(fin, read_len, buf, 0x08);
							First8 = buf[0];
							Second8 = buf[1];
							ProcessedLines++;
						}
					}

					if (First8 == 0xD2000000)
					{
						ProcessedLines--;
						fin->pos = ARCODE_POS + (ProcessedLines << 3);
					}
				}
				break;
			case 0xB0000000://Load Offset
				CodeOffset = Read32(Offset);
				CodeOffset += (CodeOffset < CODE_OFFSET) ? CODE_OFFSET : 0;
				break;
			case 0xC0000000://Loop Code
				RepeatCount = Second8;
				RepeatStart = ProcessedLines;
				break;
			case 0xD0000000://Various
				Offset = CodeOffset + Second8;
				switch (First8 & 0x0F000000)
				{
					case 0x01000000://Loop execute variant
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = ARCODE_POS + (ProcessedLines << 3);
						}
						break;
					case 0x02000000://Loop Execute Variant / Full Terminator
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = ARCODE_POS + (ProcessedLines << 3);
						}
						else
						{
							CodeOffset = CODE_OFFSET;
							CodeData = 0x00;
						}
						break;
					case 0x03000000://Set Offset
						CodeOffset = Second8;
						CodeOffset += (CodeOffset < CODE_OFFSET) ? CODE_OFFSET : 0;
						break;
					case 0x04000000://Add Value
						CodeData += Second8;
						break;
					case 0x05000000://Set Value
						CodeData = Second8;
						break;
					case 0x06000000://32-Bit Incrementive Write
						Write32(Offset, CodeData);
						CodeOffset += 0x04;
						break;
					case 0x07000000://16-Bit Incrementive Write
						Write16(Offset, CodeData);
						CodeOffset += 0x02;
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
 					case 0x0D000000://Dump to file
 						WriteBack(WRITE_BACK_ALL, 0);
						unsigned int tempoffset = CodeOffset + (short)(First8 & 0x000FFFFF);
						IFile_Open(FILE_HANDLE, &arcode[Index], FILE_W); 
						Index += First8 >> 19 & 0x0000001E;
						while (Second8 > 0){
							CopyMem((void*)tempoffset, (void*)BUF_LOC, BUF_SIZE, SLEEP_DEFAULT); 
							IFile_Write(FILE_HANDLE, BUF_RW_LEN, (unsigned*)(BUF_LOC + (tempoffset & 0x0000000F)), BUF_SIZE - (tempoffset & 0x0000000F));
							Second8 -= BUF_SIZE - (tempoffset & 0x0000000F);
						}
						break;
 					case 0x0E000000://Patch from file
 						WriteBack(WRITE_BACK_ALL, 0);
						unsigned int tempoffset = CodeOffset + (short)(First8 & 0x000FFFFF);
						IFile_Open(FILE_HANDLE, &arcode[Index], FILE_R); 
						Index += First8 >> 19 & 0x0000001E;
						do
						{
							CopyMem((void*)tempoffset, (void*)BUF_LOC, BUF_SIZE, SLEEP_DEFAULT); 
							IFile_Read(FILE_HANDLE, BUF_RW_LEN, (unsigned*)(BUF_LOC + (tempoffset & 0x0000000F)), BUF_SIZE - (tempoffset & 0x0000000F));
							CopyMem((void*)BUF_LOC, (void*)tempoffset, BUF_SIZE, SLEEP_DEFAULT); 
						}
						while (*(unsigned*)BUF_RW_LEN > 0);
						break;						
				}
				break;
			case 0xE0000000://Patch Code
				Data = Second8;
				unsigned int tempcount = 0x00;

				while ((Data > 0x00) && (ProcessedLines < LineCount))
				{
					IFile_Read(fin, read_len, buf, 0x08);
					First8 = buf[0];
					Second8 = buf[1];
					ProcessedLines++;

					if (Data >= 0x08)//Double 32bit
					{
						Write32(Offset, First8);
						Offset += 0x04;

						Write32(Offset, Second8);
						Offset += 0x04;

						Data -= 0x08;
						tempcount += 0x02;
					}
					else if (Data >= 0x04)//32bit
					{
						Write32(Offset, First8);
						Offset += 0x04;
						Data -= 0x04;
						tempcount++;
					}
					else if (Data >= 0x02)//16bit
					{
						Write16(Offset, ((tempcount & 0x01) == 0x00) ? First8 : Second8);
						Offset += 0x02;
						Data -= 0x02;
					}
					else//8bit
					{
						Write8(Offset, ((tempcount & 0x01) == 0x00) ? First8 : Second8);
						Offset += 0x01;
						Data -= 0x01;
					}
				}
				break;
			case 0xF0000000://Memory Copy Code
				Offset = CODE_OFFSET + (First8 & 0x0FFFFFFF);
				unsigned int tempoffset = CodeOffset;

				while (Second8 > 0x00)
				{
					if (Second8 >= 0x04)//32bit
					{
						Write32(Offset, Read32(tempoffset));
						tempoffset += 0x04;
						Offset += 0x04;
						Second8 -= 0x04;
					}
					else if (Second8 >= 0x02)//16bit
					{
						Write16(Offset, Read16(tempoffset));
						tempoffset += 0x02;
						Offset += 0x02;
						Second8 -= 0x02;
					}
					else//8bit
					{
						Write8(Offset, Read8(tempoffset));
						tempoffset += 0x01;
						Offset += 0x01;
						Second8 -= 0x01;
					}
				}
			break;
		}
	}
	WriteBack(WRITE_BACK_ALL, 0);
	return 0;
}

void CopyMem(void *src, void *dst, unsigned int size, unsigned long long sleep){
	GSPGPU_FlushDataCache(src, size);
	GX_SetTextureCopy(src, dst, size, 0, 0, 0, 0, 8);
	GSPGPU_FlushDataCache(dst, size);
	if(sleep > 0){
		svcSleepThread(sleep);
	}
}

void WriteBack(unsigned int Offset, unsigned Size)
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
