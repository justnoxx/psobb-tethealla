#include	<windows.h>
#include	<stdio.h>
#include	<string.h>
#include	<time.h>
#include	<math.h>
#include	"prs.cpp"

//#define WORD_WRAP_LEN 26

unsigned short unitxt_buffer[512*1024];
unsigned short new_unitxt_buffer[512*1024];
unsigned short real_line_buffer[4096];
unsigned char final_buffer[512*1024];
unsigned char line_buffer[4096];
unsigned short twobyte_buffer[4096];
unsigned uni_offsets[8192] = {0};
unsigned uni_sizes[8192] = {0};
unsigned uni_categories[256] = {0};
unsigned total_unicount;
//unsigned char uni_header[0x128];
unsigned prs_size;

void main(int argc)
{
	FILE *fp,*tp;
	unsigned ch,ch2,ch3,ch4,ch5,ch6,ch7,uni_size,line_count,offset;
	int do_wrap;

	printf ("\nUnitxt preparation tool v0.02 by Sodaboy\nPRS compression/decompression by Fuzziqersoftware\n");
	printf ("---\n");
	if (argc > 1)
	{
		printf ("Converting unitxt_j.prs into unitxt_unicode.txt\n");
		printf ("Press [ENTER] to begin or CTRL+C to abort...");
		gets   (&line_buffer[0]);
		printf ("\n");
		fp = fopen ("unitxt_j.prs", "rb");
		if (!fp)
		{
			printf ("Could not locate unitxt_j.prs for reading...\n");
			printf ("(Hit Enter)");
			gets (&line_buffer[0]);
			exit (1);
		}
		fseek (fp, 0, SEEK_END);
		uni_size = ftell (fp);
		fseek (fp, 0, SEEK_SET);
		fread (&final_buffer[0], 1, uni_size, fp);
		fclose (fp);
		printf ("Decompressing unitxt_j.prs into memory...\n");
		uni_size = prs_decompress (&final_buffer[0], &unitxt_buffer[0]);
		memcpy (&uni_categories[0], &unitxt_buffer[0], 4);
		memcpy (&uni_categories[1], &unitxt_buffer[2], uni_categories[0] * 4);
		total_unicount = 0;
		for (ch=1;ch<=uni_categories[0];ch++)
			total_unicount += uni_categories[ch];
		printf ("Number of categories in unitxt file: %u\n", uni_categories[0]);
		printf ("Number of entries in unitxt file: %u\n", total_unicount);
		printf ("Size of unitxt file: %u\n", uni_size);
		memcpy (&uni_offsets[0], &unitxt_buffer[(4 + (uni_categories[0] * 4))>>1], total_unicount * 4);
		ch  = 0;
		ch2 = 0;
		line_count = 0;
		tp = fopen ("unitxt_unicode.txt", "wb");
		twobyte_buffer[0] = 0xFEFF;
		fwrite (&twobyte_buffer[0], 1, 2, tp);
		final_buffer[0] = 0;
		strcat (&final_buffer[0], "##### CATEGORY #####\r\n");
		ch4 = 0;
		for (ch3=0;ch3<strlen(&final_buffer[0]);ch3++)
		{
			line_buffer[ch4++] = final_buffer[ch3];
			line_buffer[ch4++] = 0;
		}
		fwrite (&line_buffer[0], 1, ch4, tp);
		ch6 = 1;
		ch7 = uni_categories[1];
		for (ch5=0;ch5<total_unicount;ch5++)
		{
			ch  = uni_offsets[ch5] >> 1;
			ch2 = 0;
			while (unitxt_buffer[ch] != 0x0000)
			{
				switch (unitxt_buffer[ch])
				{
				case 9: // Tab
					twobyte_buffer[ch2++] = 92;
					twobyte_buffer[ch2++] = 116;
					break;
				case 10: // Line break
					twobyte_buffer[ch2++] = 92;
					twobyte_buffer[ch2++] = 110;
					break;
				case 92: // Another slash...
					twobyte_buffer[ch2++] = 92;
					twobyte_buffer[ch2++] = 92;
					break;
				default:
					twobyte_buffer[ch2++] = unitxt_buffer[ch];
					break;
				}
				ch++;
			}
			line_count++;
			do_wrap = 0;
			final_buffer[0] = 0;
			strcat (&final_buffer[0], "#ENTRY");
			_itoa (line_count, &final_buffer[strlen(&final_buffer[0])], 10);
			strcat (&final_buffer[0], "\r\n");
			ch4 = 0;
			for (ch3=0;ch3<strlen(&final_buffer[0]);ch3++)
			{
				line_buffer[ch4++] = final_buffer[ch3];
				line_buffer[ch4++] = 0;
			}
			fwrite (&line_buffer[0], 1, ch4, tp);
			fwrite (&twobyte_buffer[0], 1, ch2*2, tp);
			line_buffer[0] = 0x0D;
			line_buffer[1] = 0x00;
			line_buffer[2] = 0x0A;
			line_buffer[3] = 0x00;
			fwrite (&line_buffer[0], 1, 4, tp);
			ch7--;
			if ((!ch7) && (ch5 < total_unicount - 1))
			{
				ch6++;
				ch7 = uni_categories[ch6];
				final_buffer[0] = 0;
				strcat (&final_buffer[0], "##### CATEGORY #####\r\n");
				ch4 = 0;
				for (ch3=0;ch3<strlen(&final_buffer[0]);ch3++)
				{
					line_buffer[ch4++] = final_buffer[ch3];
					line_buffer[ch4++] = 0;
				}
				fwrite (&line_buffer[0], 1, ch4, tp);
			}
			printf ("Wrote line %u to file\n", line_count);
		}
		fclose (tp);
	}
	else
	{
		printf ("Making a new newunitxt.prs file from unitxt_unicode.txt\n");
		printf ("Press [ENTER] to begin or CTRL+C to abort...");
		gets   (&line_buffer[0]);
		printf ("\n");
		fp = fopen ("unitxt_unicode.txt", "rb");
		if (!fp)
		{
			printf ("Could not locate unitxt_unicode.txt for reading...\n");
			printf ("(Hit Enter)");
			gets (&line_buffer[0]);
			exit (1);
		}
		line_count = 0; // Total amount of lines
		ch2 = 0; // New Unitxt Offset
		ch5 = 0; // Category Count
		fseek (fp,0,SEEK_END);
		uni_size = ftell (fp);
		fseek (fp,0,SEEK_SET);
		fread (&unitxt_buffer[0], 1, uni_size, fp);
		if (unitxt_buffer[0] == 0xFEFF)
			ch = 1; // Unitxt Offset
		else
			ch = 0;
		uni_size >>= 1;
		ch6 = 0;
		while (ch < uni_size)
		{
			ch3 = 0;

			while (ch < uni_size)
			{
				if (unitxt_buffer[ch] != 0x0D)
				{
					real_line_buffer[ch3] = unitxt_buffer[ch++];
					if (real_line_buffer[ch3] == 92)
						switch (unitxt_buffer[ch++])
					{
						case 110:
							real_line_buffer[ch3] = 10;
							break;
						case 116:
							real_line_buffer[ch3] = 9;
							break;
					}
					ch3++;
				}
				else
				{
					// End of entry
					ch+=2;
					real_line_buffer[ch3++] = 0x00;
					break;
				}
			}

			if (real_line_buffer[0] == 35) // Entry or category
			{
				if (real_line_buffer[1] == 35) // Category increase
					ch5++;
			}
			else
			{
				memcpy ( &new_unitxt_buffer[ch2], &real_line_buffer[0], ch3 * 2 );
				uni_categories[ch5]++;
				uni_offsets[line_count++] = ( ch2 * 2 );
				ch2 += ch3;
			}
		}

		fclose (fp);

		new_unitxt_buffer[ch2++] = 0x00;
		new_unitxt_buffer[ch2++] = 0x3F;
		uni_categories[0] = ch5;
		printf ("Number of categories read: %u\n", ch5 );
		printf ("Number of lines read: %u\n", line_count );
		total_unicount = 0;
		for (ch=1;ch<=ch5;ch++)
			total_unicount += uni_categories[ch];
		printf ("Number of lines check: %u\n", total_unicount );
		memcpy (&final_buffer[0], &uni_categories[0], ( ch5 * 4 ) + 4 );
		offset = ( ( ch5 * 4 ) + 4 ) + ( line_count * 4 );
		for (ch=0;ch<line_count;ch++)
			uni_offsets[ch] += offset;
		memcpy (&final_buffer[( ch5 * 4) + 4], &uni_offsets[0], line_count * 4 );
		memcpy (&final_buffer[offset], &new_unitxt_buffer[0], ch2 * 2 );
		offset += ch2 * 2;
		fp = fopen ("newunitxt.bin", "wb");
		fwrite (&final_buffer[0], 1, offset, fp);
		fclose (fp);
		printf ("Compressing data (%u bytes)... (This may take awhile...)", offset);
		prs_size = prs_compress (&final_buffer[0], &unitxt_buffer[0], offset);
		fp = fopen ("newunitxt.prs", "wb");
		fwrite (&unitxt_buffer[0], 1, prs_size, fp);
		fclose (fp);
		printf ("\nnewunitxt.prs wrote (%u bytes)! (Hit Enter)", prs_size);
		gets (&line_buffer[0]);
		printf ("\n\nHint: Using ANY command line parameter makes this program convert\n");
		printf ("unitxt_j_prs into unitxt.txt (Hit Enter)");
		gets (&line_buffer[0]);
	}
}
