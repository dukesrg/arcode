#include "spider.h"
#include "fs.h"

#define CODE_OFFSET	0x14000000
#define ARCODE_POS	0x00000004

void Write32(unsigned int Offset, unsigned int Data);
void Write16(unsigned int Offset, unsigned int Data);
void Write8(unsigned int Offset, unsigned int Data);
unsigned int Read32(unsigned int Offset);

unsigned int *buf = (unsigned int *)0x18410000;

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
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = ARCODE_POS + (ProcessedLines << 3);
						}
						CodeOffset = CODE_OFFSET;
						CodeData = 0x00;
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
						CodeOffset = CODE_OFFSET;
						CodeData = 0x00;
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
						CodeData = Read32(Offset) & 0xFFFF;
						break;
					case 0x0B000000://8-Bit Load
						CodeData = Read32(Offset) & 0xFF;
						break;
					case 0x0C000000://Add Offset
						CodeOffset += Second8;
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
						Write16(Offset, Read32(tempoffset) & 0xFFFF);
						tempoffset += 0x02;
						Offset += 0x02;
						Second8 -= 0x02;
					}
					else//8bit
					{
						Write8(Offset, Read32(tempoffset) & 0xFF);
						tempoffset += 0x01;
						Offset += 0x01;
						Second8 -= 0x01;
					}
				}
			break;
		}
	}

	return 0;
}

void CopyMem(void *src, void *dst){
	GSPGPU_FlushDataCache(src, 0x20);
	GX_SetTextureCopy(src, dst, 0x20, 0, 0, 0, 0, 8);
	GSPGPU_FlushDataCache(dst, 0x20);
	svcSleepThread(0x200000LL);
}

void Write32(unsigned int Offset, unsigned int Data)
{
	CopyMem((void *)(Offset & 0xFFFFFFF0), buf);
	unsigned int offs = (Offset & 0x0F) >> 2;
	unsigned int shift = (Offset & 0x03) << 3;
	unsigned int mask = 0xFFFFFFFF << shift;
	buf[offs] = (buf[offs] & ~mask) | ((Data << shift ) & mask);
	buf[offs+1] = (buf[offs+1] & mask) | ((Data >> (0x20 - shift)) & ~mask);
	CopyMem(buf, (void *)(Offset & 0xFFFFFFF0));
}

void Write16(unsigned int Offset, unsigned int Data)
{
	CopyMem((void *)(Offset & 0xFFFFFFF0), buf);
	Data &= 0x0000FFFF;
	unsigned int offs = (Offset & 0x0F) >> 2;
	unsigned int shift = (Offset & 0x03) << 3;
	unsigned int mask = 0x0000FFFF << shift;
	unsigned int mask1 = (mask == 0xFF000000) ? 0x000000FF : 0x00000000;
	buf[offs] = (buf[offs] & ~mask) | (Data << shift);
	buf[offs+1] = (buf[offs+1] & ~mask1) | ((Data >> (0x20 - shift)) & mask1);
	CopyMem(buf, (void *)(Offset & 0xFFFFFFF0));
}

void Write8(unsigned int Offset, unsigned int Data)
{
	CopyMem((void *)(Offset & 0xFFFFFFF0), buf);
	unsigned int offs = (Offset & 0x0F) >> 2;
	unsigned int shift = (Offset & 0x03) << 3;
	buf[offs] = (buf[offs] & ~(0x000000FF << shift)) | ((Data & 0x000000FF) << shift);
	CopyMem(buf, (void *)(Offset & 0xFFFFFFF0));
}

unsigned int Read32(unsigned int Offset)
{
	CopyMem((void *)(Offset & 0xFFFFFFF0), buf);
	unsigned int offs = (Offset & 0x0F) >> 2;
	unsigned int shift = (Offset & 0x03) << 3;
	unsigned int mask = 0xFFFFFFFF >> shift;
	return ((buf[offs] >> shift) & mask) | ((buf[offs+1] << (0x20 - shift)) & ~mask);
}
