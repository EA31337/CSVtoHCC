/* version 1.1 - includes header format and detects the number of data rows */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma pack(1)
union header {
	struct {
		char magic[2]; // 0x81 0x00
		char label[64];
		char something[18];
		int nrows;
	} data;
	char rawbytes[189];
};

struct record {
	int separator;
	int time;
	double open, high, low, close;
	char additional[10];
};

void printint(char *buf, int size);
void dumpat(FILE *fp, int start);

int main(int argc, char **argv) {
	FILE *fp;
	union header header;

	if (argc!=2) {
		fprintf(stderr, "Usage: %s file.hcc\n", argv[0]);
		exit(1);
	}
	if ((fp=fopen(argv[1], "rb"))==NULL) {
		perror(argv[1]); exit(1);
	}
	fseek(fp, 0x3be5-sizeof(header), 0);
	while (fread(&header, sizeof header, 1, fp)!=0) {
		if ((header.data.magic[0]&0xff)!=0x81
		||   header.data.magic[1]!=0x00) {
			fprintf(stderr, "Bad header magic number at %08lx\n",
				ftell(fp)-sizeof(header));
			exit(1);
		}
		printf("Expect %d rows\n", header.data.nrows);
		dumpat(fp, header.data.nrows);
	// dumpat(fp, 0x3be5);
	// dumpat(fp, 0xde1a);
	// dumpat(fp, 0x1e169);
	// dumpat(fp, 0x2e4a5);
	// dumpat(fp, 0x3bd7e);
	// dumpat(fp, 0x55c07);
	}
	fclose(fp);
}

void dumpat(FILE *fp, int nrows) {
	struct record data;
	int i, extra1, extra2, extra3, pos;
	time_t time;
	struct tm *tm;
	char timebuf[80];

	for (i=0; i<nrows; i++) {
		if (fread(&data, sizeof(int), 1, fp)==0)
			break;
		if ((data.separator & 0x00088884) != 0x00088884) {
			fprintf(stdout, "bad separator %08x at %lx\n",
				data.separator, ftell(fp));
			break;
		}
		extra1=data.separator>>28;
		extra2=((data.separator>>24)&0x0f);
		extra3=((data.separator>>20)&0x0f);
		fread(&data.time,  4 + 8 + 8 + 8 + 8 + extra3 + extra2 + extra1
			, 1, fp);
		time=data.time;
		tm=gmtime(&time);
		strftime(timebuf, sizeof timebuf, "%Y-%m-%d %H:%M:%S", tm);
		printf("%s %lf %lf %lf %lf ", timebuf, data.open,
			data.high, data.low, data.close);
		pos=0;
		printint(data.additional+pos, extra3); pos+=extra3;
		printint(data.additional+pos, extra2); pos+=extra2;
		printint(data.additional+pos, extra1); pos+=extra1;
		putchar('\n');
	}
}

void printint(char *buf, int size) {
	int val=0, scale=1;
	while (size--) {
		val+=(*buf++&0xff)*scale;
		scale<<=8;
	}
	printf("%5d ", val);
}
