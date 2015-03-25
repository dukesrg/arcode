Enter file contents here#include "spider.h"
#include "fs.h"

void ReadMemory(unsigned int Offset);
void WriteMemory(unsigned int Offset);
void Write32(unsigned int Offset, unsigned int Data);
void Write16(unsigned int Offset, unsigned int Data);
void Write8(unsigned int Offset, unsigned int Data);
unsigned int Read32(unsigned int Offset);

int uvl_entry()
{
	FILE *fin = (void *)0x08F10000;
	unsigned int *buf = 0x18410000;
	int *read_len = 0x08F10020;
	int *written = 0x08F01000;
	unsigned int LineCount = 0;
	unsigned int ProcessedLines = 0;
	unsigned int First8 = 0;
	unsigned int Second8 = 0;
	unsigned int CodeOffset = 0x14000000;
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

		switch ((First8 & 0xF0000000) >> 0x1C)
		{
			case 0x00://32-bit Write
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Write32(Offset, Second8);
				WriteMemory(Offset);
			}
			break;

			case 0x01://16-bit Write
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Write16(Offset, Second8);
				WriteMemory(Offset);
			}
			break;

			case 0x02://8-bit Write
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Write8(Offset, Second8);
				WriteMemory(Offset);
			}
			break;

			case 0x03://32-bit Greater Than
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Data = Read32(Offset);

				if (Second8 <= Data)
				{
					while (((First8 != 0xD0000000) && (First8 != 0xD2000000)) && (ProcessedLines < LineCount))
					{
						IFile_Read(fin, read_len, buf, 0x08);
						First8 = buf[0];
						Second8 = buf[1];
						ProcessedLines++;
					}

					if (First8 == 0xD2000000)
					{
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = 0x04 + (ProcessedLines * 0x08);
						}
						CodeOffset = 0x14000000;
						CodeData = 0x00;
					}
				}
			}
			break;

			case 0x04://32-bit Less Than
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Data = Read32(Offset);

				if (Second8 >= Data)
				{
					while (((First8 != 0xD0000000) && (First8 != 0xD2000000)) && (ProcessedLines < LineCount))
					{
						IFile_Read(fin, read_len, buf, 0x08);
						First8 = buf[0];
						Second8 = buf[1];
						ProcessedLines++;
					}

					if (First8 == 0xD2000000)
					{
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = 0x04 + (ProcessedLines * 0x08);
						}
						CodeOffset = 0x14000000;
						CodeData = 0x00;
					}
				}
			}
			break;

			case 0x05://32-bit Equal To
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Data = Read32(Offset);

				if (Second8 != Data)
				{
					while (((First8 != 0xD0000000) && (First8 != 0xD2000000)) && (ProcessedLines < LineCount))
					{
						IFile_Read(fin, read_len, buf, 0x08);
						First8 = buf[0];
						Second8 = buf[1];
						ProcessedLines++;
					}

					if (First8 == 0xD2000000)
					{
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = 0x04 + (ProcessedLines * 0x08);
						}
						CodeOffset = 0x14000000;
						CodeData = 0x00;
					}
				}
			}
			break;

			case 0x06://32-bit Not Equal To
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Data = Read32(Offset);

				if (Second8 == Data)
				{
					while (((First8 != 0xD0000000) && (First8 != 0xD2000000)) && (ProcessedLines < LineCount))
					{
						IFile_Read(fin, read_len, buf, 0x08);
						First8 = buf[0];
						Second8 = buf[1];
						ProcessedLines++;
					}

					if (First8 == 0xD2000000)
					{
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = 0x04 + (ProcessedLines * 0x08);
						}
						CodeOffset = 0x14000000;
						CodeData = 0x00;
					}
				}
			}
			break;

			case 0x07://16-bit Greater Than
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Data = Read32(Offset) & 0xFFFF;

				if (Second8 <= Data)
				{
					while (((First8 != 0xD0000000) && (First8 != 0xD2000000)) && (ProcessedLines < LineCount))
					{
						IFile_Read(fin, read_len, buf, 0x08);
						First8 = buf[0];
						Second8 = buf[1];
						ProcessedLines++;
					}

					if (First8 == 0xD2000000)
					{
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = 0x04 + (ProcessedLines * 0x08);
						}
						CodeOffset = 0x14000000;
						CodeData = 0x00;
					}
				}
			}
			break;

			case 0x08://16-bit Less Than
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Data = Read32(Offset) & 0xFFFF;

				if (Second8 >= Data)
				{
					while (((First8 != 0xD0000000) && (First8 != 0xD2000000)) && (ProcessedLines < LineCount))
					{
						IFile_Read(fin, read_len, buf, 0x08);
						First8 = buf[0];
						Second8 = buf[1];
						ProcessedLines++;
					}

					if (First8 == 0xD2000000)
					{
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = 0x04 + (ProcessedLines * 0x08);
						}
						CodeOffset = 0x14000000;
						CodeData = 0x00;
					}
				}
			}
			break;

			case 0x09://16-bit Equal To
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Data = Read32(Offset) & 0xFFFF;

				if (Second8 != Data)
				{
					while (((First8 != 0xD0000000) && (First8 != 0xD2000000)) && (ProcessedLines < LineCount))
					{
						IFile_Read(fin, read_len, buf, 0x08);
						First8 = buf[0];
						Second8 = buf[1];
						ProcessedLines++;
					}

					if (First8 == 0xD2000000)
					{
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = 0x04 + (ProcessedLines * 0x08);
						}
						CodeOffset = 0x14000000;
						CodeData = 0x00;
					}
				}
			}
			break;

			case 0x0A://16-bit Not Equal To
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Data = Read32(Offset) & 0xFFFF;

				if (Second8 == Data)
				{
					while (((First8 != 0xD0000000) && (First8 != 0xD2000000)) && (ProcessedLines < LineCount))
					{
						IFile_Read(fin, read_len, buf, 0x08);
						First8 = buf[0];
						Second8 = buf[1];
						ProcessedLines++;
					}

					if (First8 == 0xD2000000)
					{
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = 0x04 + (ProcessedLines * 0x08);
						}
						CodeOffset = 0x14000000;
						CodeData = 0x00;
					}
				}
			}
			break;

			case 0x0B://Load Offset
			{
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				ReadMemory(Offset);
				Data = Read32(Offset);

				if (Data >= 0x14000000)
				{
					CodeOffset = Data;
				}
				else
				{
					CodeOffset = Data + 0x14000000;
				}
			}
			break;

			case 0x0C://Loop Code
			{
				RepeatCount = Second8;
				RepeatStart = ProcessedLines;
			}
			break;

			case 0x0D://Various
			{
				switch ((First8 & 0xF000000) >> 0x18)
				{
					case 0x00://Terminator
					{
						//Nothing to see here
					}
					break;

					case 0x01://Loop execute variant
					{
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = 0x04 + (ProcessedLines * 0x08);
						}
					}
					break;

					case 0x02://Loop Execute Variant / Full Terminator
					{
						if (RepeatCount > 0x00)
						{
							RepeatCount--;
							ProcessedLines = RepeatStart;
							fin->pos = 0x04 + (ProcessedLines * 0x08);
						}
						CodeOffset = 0x14000000;
						CodeData = 0x00;
					}
					break;

					case 0x03://Set Offset
					{
						if (Second8 >= 0x14000000)
						{
							CodeOffset = Second8;
						}
						else
						{
							CodeOffset = Second8 + 0x14000000;
						}
					}
					break;

					case 0x04://Add Value
					{
						CodeData += Second8;
					}
					break;

					case 0x05://Set Value
					{
						CodeData = Second8;
					}
					break;

					case 0x06://32-Bit Incrementive Write
					{
						Offset = CodeOffset + (Second8 & 0xFFFFFFF);
						ReadMemory(Offset);
						Write32(Offset, CodeData);
						WriteMemory(Offset);
						CodeOffset += 0x04;
					}
					break;

					case 0x07://16-Bit Incrementive Write
					{
						Offset = CodeOffset + (Second8 & 0xFFFFFFF);
						ReadMemory(Offset);
						Write16(Offset, CodeData);
						WriteMemory(Offset);
						CodeOffset += 0x02;
					}
					break;

					case 0x08://8-Bit Incrementive Write
					{
						Offset = CodeOffset + (Second8 & 0xFFFFFFF);
						ReadMemory(Offset);
						Write8(Offset, CodeData);
						WriteMemory(Offset);
						CodeOffset++;
					}
					break;

					case 0x09://32-Bit Load
					{
						Offset = CodeOffset + (Second8 & 0xFFFFFFF);
						ReadMemory(Offset);
						CodeData = Read32(Offset);
					}
					break;

					case 0x0A://16-Bit Load
					{
						Offset = CodeOffset + (Second8 & 0xFFFFFFF);
						ReadMemory(Offset);
						CodeData = Read32(Offset) & 0xFFFF;
					}
					break;

					case 0x0B://8-Bit Load
					{
						Offset = CodeOffset + (Second8 & 0xFFFFFFF);
						ReadMemory(Offset);
						CodeData = Read32(Offset) & 0xFF;
					}
					break;

					case 0x0C://Add Offset
					{
						CodeOffset += Second8;
					}
					break;
				}
			}
			break;

			case 0x0E://Patch Code
			{
				unsigned int tempcount = 0x00;
				unsigned int tempdata = 0x00;
				Offset = CodeOffset + (First8 & 0xFFFFFFF);
				Data = Second8;

				while ((Data > 0x00) && (ProcessedLines < LineCount))
				{
					IFile_Read(fin, read_len, buf, 0x08);
					First8 = buf[0];
					Second8 = buf[1];
					ProcessedLines++;

					if (Data >= 0x04)//Double 32bit
					{
						ReadMemory(Offset);
						Write32(Offset, First8);
						WriteMemory(Offset);
						Offset += 0x04;

						ReadMemory(Offset);
						Write32(Offset, Second8);
						WriteMemory(Offset);
						Offset += 0x04;

						Data -= 0x08;
						tempcount += 0x02;
					}
					else if (Data >= 0x04)//32bit
					{
						ReadMemory(Offset);
						Write32(Offset, First8);
						WriteMemory(Offset);
						Offset += 0x04;
						Data -= 0x04;
						tempcount++;
					}
					else if (Data >= 0x02)//16bit
					{
						if ((tempcount & 0x01) == 0x00)
						{
							tempdata = First8;
						}
						else
						{
							tempdata = Second8;
						}

						ReadMemory(Offset);
						Write16(Offset, tempdata);
						WriteMemory(Offset);
						Offset += 0x02;
						Data -= 0x02;
					}
					else//8bit
					{
						if ((tempcount & 0x01) == 0x00)
						{
							tempdata = First8;
						}
						else
						{
							tempdata = Second8;
						}

						ReadMemory(Offset);
						Write8(Offset, tempdata);
						WriteMemory(Offset);
						Offset += 0x01;
						Data -= 0x01;
					}
				}
			}
			break;

			case 0x0F://Memory Copy Code
			{
				unsigned int tempdata = 0x00;
				unsigned int tempoffset = CodeOffset;
				Offset = (First8 & 0xFFFFFFF) + 0x14000000;

				while (Second8 > 0x00)
				{
					if (Second8 >= 0x04)//32bit
					{
						ReadMemory(tempoffset);
						tempdata = Read32(Offset);
						ReadMemory(Offset);
						Write32(Offset, tempdata);
						WriteMemory(Offset);
						tempoffset += 0x04;
						Offset += 0x04;
						Second8 -= 0x04;
					}
					else if (Second8 >= 0x02)//16bit
					{
						ReadMemory(tempoffset);
						tempdata = Read32(Offset) & 0xFFFF;
						ReadMemory(Offset);
						Write16(Offset, tempdata);
						WriteMemory(Offset);
						tempoffset += 0x02;
						Offset += 0x02;
						Second8 -= 0x02;
					}
					else//8bit
					{
						ReadMemory(tempoffset);
						tempdata = Read32(Offset) & 0xFF;
						ReadMemory(Offset);
						Write8(Offset, tempdata);
						WriteMemory(Offset);
						tempoffset += 0x01;
						Offset += 0x01;
						Second8 -= 0x01;
					}
				}
			}
			break;
		}
	}

	return 0;
}

void ReadMemory(unsigned int Offset)
{
	unsigned int *buf = 0x18410000;
	GSPGPU_FlushDataCache(Offset & 0xFFFFFFF0, 0x20);
	GX_SetTextureCopy(Offset & 0xFFFFFFF0, buf, 0x20, 0, 0, 0, 0, 8);
	GSPGPU_FlushDataCache(buf, 0x20);
	svcSleepThread(0x200000LL);
}

void WriteMemory(unsigned int Offset)
{
	unsigned int *buf = 0x18410000;
	GSPGPU_FlushDataCache(buf, 0x20);
	GX_SetTextureCopy(buf, Offset & 0xFFFFFFF0, 0x20, 0, 0, 0, 0, 8);
	GSPGPU_FlushDataCache(Offset & 0xFFFFFFF0, 0x20);
	svcSleepThread(0x200000LL);
}

void Write32(unsigned int Offset, unsigned int Data)
{
	unsigned int *buf = 0x18410000;

	switch (Offset % 0x04)
	{
		case 0:
		{
			buf[(Offset & 0x0F) / 0x04] = Data;
		}
		break;

		case 1:
		{
			buf[(Offset & 0x0F) / 0x04] = (buf[(Offset & 0x0F) / 0x04] & 0xFF) + ((Data & 0xFFFFFF) << 0x08);
			buf[((Offset & 0x0F) / 0x04) + 0x01] = (buf[((Offset & 0x0F) / 0x04) + 0x01] & 0xFFFFFF00) + ((Data & 0xFF000000) >> 0x18);
		}
		break;

		case 2:
		{
			buf[(Offset & 0x0F) / 0x04] = (buf[(Offset & 0x0F) / 0x04] & 0xFFFF) + ((Data & 0xFFFF) << 0x10);
			buf[((Offset & 0x0F) / 0x04) + 0x01] = (buf[((Offset & 0x0F) / 0x04) + 0x01] & 0xFFFF0000) + ((Data & 0xFFFF0000) >> 0x10);
		}
		break;

		case 3:
		{
			buf[(Offset & 0x0F) / 0x04] = (buf[(Offset & 0x0F) / 0x04] & 0xFFFFFF) + ((Data & 0xFF) << 0x18);
			buf[((Offset & 0x0F) / 0x04) + 0x01] = (buf[((Offset & 0x0F) / 0x04) + 0x01] & 0xFF000000) + ((Data & 0xFFFFFF00) >> 0x08);
		}
		break;
	}
}

void Write16(unsigned int Offset, unsigned int Data)
{
	unsigned int *buf = 0x18410000;

	switch (Offset % 0x04)
	{
		case 0:
		{
			buf[(Offset & 0x0F) / 0x04] = (buf[(Offset & 0x0F) / 0x04] & 0xFFFF0000) + (Data & 0xFFFF);
		}
		break;

		case 1:
		{
			buf[(Offset & 0x0F) / 0x04] = (buf[(Offset & 0x0F) / 0x04] & 0xFF0000FF) + ((Data & 0xFFFF) << 0x08);
		}
		break;

		case 2:
		{
			buf[(Offset & 0x0F) / 0x04] = (buf[(Offset & 0x0F) / 0x04] & 0xFFFF) + ((Data & 0xFFFF) << 0x10);
		}
		break;

		case 3:
		{
			buf[(Offset & 0x0F) / 0x04] = (buf[(Offset & 0x0F) / 0x04] & 0xFFFFFF) + ((Data & 0xFF) << 0x18);
			buf[((Offset & 0x0F) / 0x04) + 0x01] = (buf[((Offset & 0x0F) / 0x04) + 0x01] & 0xFFFFFF00) + ((Data & 0xFF00) >> 0x08);
		}
		break;
	}
}

void Write8(unsigned int Offset, unsigned int Data)
{
	unsigned int *buf = 0x18410000;

	switch (Offset % 0x04)
	{
		case 0:
		{
			buf[(Offset & 0x0F) / 0x04] = (buf[(Offset & 0x0F) / 0x04] & 0xFFFFFF00) + (Data & 0xFF);
		}
		break;

		case 1:
		{
			buf[(Offset & 0x0F) / 0x04] = (buf[(Offset & 0x0F) / 0x04] & 0xFFFF00FF) + ((Data & 0xFF) << 0x08);
		}
		break;

		case 2:
		{
			buf[(Offset & 0x0F) / 0x04] = (buf[(Offset & 0x0F) / 0x04] & 0xFF00FFFF) + ((Data & 0xFF) << 0x10);
		}
		break;

		case 3:
		{
			buf[(Offset & 0x0F) / 0x04] = (buf[(Offset & 0x0F) / 0x04] & 0xFFFFFF) + ((Data & 0xFF) << 0x18);
		}
		break;
	}
}

unsigned int Read32(unsigned int Offset)
{
	unsigned int *buf = 0x18410000;
	unsigned int value = 0;

	switch (Offset % 0x04)
	{
		case 0:
		{
			value = buf[(Offset & 0x0F) / 0x04];
		}
			break;

		case 1:
		{
			value = ((buf[(Offset & 0x0F) / 0x04] & 0xFFFFFF00) >> 0x08) + ((buf[((Offset & 0x0F) / 0x04) + 0x01] & 0xFF) << 0x18);
		}
			break;

		case 2:
		{
			value = ((buf[(Offset & 0x0F) / 0x04] & 0xFFFF0000) >> 0x10) + ((buf[((Offset & 0x0F) / 0x04) + 0x01] & 0xFFFF) << 0x10);
		}
			break;

		case 3:
		{
			value = ((buf[(Offset & 0x0F) / 0x04] & 0xFF000000) >> 0x18) + ((buf[((Offset & 0x0F) / 0x04) + 0x01] & 0xFFFFFF) << 0x08);
		}
		break;
	}

	return value;
}
