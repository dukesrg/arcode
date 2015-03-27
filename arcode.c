#include "spider.h"
#include "fs.h"

#define CODE_OFFSET	0x14000000
#define ARCODE_POS	0x00000001
#define MASK_32		0xFFFFFFFF
#define MASK_16		0x0000FFFF
#define MASK_8		0x000000FF

unsigned int Read(unsigned int Offset, unsigned int Mask);
void Write(unsigned int Offset, unsigned int Data, unsigned int Mask);

unsigned int *buf = (unsigned int *)0x18410000;

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
	unsigned int Increment;

	WordCount = arcode[Index++] << 1;

	while (WordCount > 0)
	{
		First8 = arcode[Index++];
		Second8 = arcode[Index++];
		WordCount -= 2;
		Offset = CodeOffset + (First8 & 0x0FFFFFFF);
		Mask = MASK_32;
		Increment = 4;
		switch (Type = First8 & 0xF0000000)
		{
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
				Mask >>= 16;
				Second8 &= ~Second8 >> 16;
			case 0x30000000://32-bit Greater Than
			case 0x40000000://32-bit Less Than
			case 0x50000000://32-bit Equal To
			case 0x60000000://32-bit Not Equal To
				Type &= 0x30000000;
				Data = Read(Offset, Mask);
				if (Type == 0x30000000 && Second8 <= Data || Type == 0x00000000 && Second8 >= Data || Type == 0x10000000 && Second8 != Data || Type == 0x20000000 && Second8 == Data)
				{
					while (First8 != 0xD0000000 && First8 != 0xD2000000 && WordCount > 0)
					{
						First8 = arcode[Index];
						Index += 2;
						WordCount -= 2;
					}
					if (First8 == 0xD2000000)
					{
						if (RepeatCount > 0)
						{
							RepeatCount--;
							Index = ARCODE_POS + (RepeatStart << 1);
						}
						CodeOffset = CODE_OFFSET;
						CodeData = 0;
					}
				}
				break;
			case 0xB0000000://Load Offset
				if ((CodeOffset = Read(Offset, Mask)) < CODE_OFFSET)
				{
					CodeOffset += CODE_OFFSET;
				}
				break;
			case 0xC0000000://Loop Code
				RepeatCount = Second8;
				RepeatStart = (Index - ARCODE_POS) >> 1;
				break;
			case 0xD0000000://Various
				Offset = CodeOffset + (Second8 & 0xFFFFFFF);
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
				Data = Second8;
				while (Data > 0 && WordCount > 0)
				{
					First8 = arcode[Index++];
					Second8 = arcode[Index++];
					WordCount -= 2;

					if (Data >= 8)//Double 32bit
					{
						Write(Offset, First8, MASK_32);
						Offset += 4;
						Write(Offset, Second8, MASK_32);
						Offset += 4;
						Data -= 8;
					}
					else
					{
						if (Data >= 4)//32bit
						{
							Write(Offset, First8, MASK_32);
							Offset += 4;
							Data -= 4;
							First8 = Second8;
						}
						if (Data >= 2)//16bit
						{
							Write(Offset, First8, MASK_16);
							Offset += 2;
							Data -= 2;
							First8 >>= 16;
						}
						if (Data == 1)//8bit
						{
							Write(Offset, First8,  MASK_8);
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
						Write(Offset, Read(tempoffset, MASK_32), MASK_32);
						tempoffset += 4;
						Offset += 4;
						Second8 -= 4;
					}
					else if (Second8 >= 2)//16bit
					{
						Write(Offset, Read(tempoffset, MASK_16) , MASK_16);
						tempoffset += 2;
						Offset += 2;
						Second8 -= 2;
					}
					else//8bit
					{
						Write(Offset, Read(tempoffset, MASK_8), MASK_8);
						tempoffset += 1;
						Offset += 1;
						Second8 -= 1;
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

unsigned int Read(unsigned int Offset, unsigned int Mask)
{
	unsigned int shift = Offset << 3 & 24;
	unsigned int offs = Offset >> 2 & 3;
	CopyMem((void *)(Offset & 0xFFFFFFF0), buf);
	return (buf[offs] >> shift) | (buf[offs+1] << (32 - shift)) & Mask;
}

void Write(unsigned int Offset, unsigned int Data, unsigned int Mask)
{
	unsigned int shift = Offset << 3 & 24;
	unsigned int offs = Offset >> 2 & 3;
	CopyMem((void *)(Offset & 0xFFFFFFF0), buf);
	Data &= Mask;
	buf[offs] &= ~(Mask << shift);
	buf[offs] |= Data << shift;
	offs++;
	shift = 32 - shift;
	buf[offs] &= ~(Mask >> shift);
	buf[offs] |= Data >> shift;
	CopyMem(buf, (void *)(Offset & 0xFFFFFFF0));
}
