/* version 1.2 - generates a CSV file on the desktop for another application to create a reverse engineered hcc file */
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
                char additional[10]


};


void printint(char *buf, FILE *fpcsv,int size);
void dumpat(FILE *fp,FILE *fpcsv, int start);

int main(int argc, char **argv) {
                FILE *fp,*fpcsv;
                union header header;


                if ((fpcsv=fopen("C:\\Users\\Hello\\Desktop\\2015.csv", "w"))==NULL) {
                        perror("C:\\Users\\Hello\\Desktop\\2015.csv"); return 1;
                }
                if ((fp=fopen("C:\\Users\\Hello\\Desktop\\2015.hcc", "rb"))==NULL) {
                        perror("C:\\Users\\Hello\\Desktop\\2015.hcc"); return 1;
                }
                fseek(fp, 0x3be5-sizeof(header), 0);
                while (fread(&header, sizeof header, 1, fp)!=0) {
                                if ((header.data.magic[0]&0xff)!=0x81
                                ||   header.data.magic[1]!=0x00) {
                                                fprintf(stderr, "Bad header magic number at %08lx\n",
                                                                ftell(fp)-sizeof(header));
                                                exit(1);
                                }
                                fprintf(fpcsv,"Expect %d rows\n", header.data.nrows);
                                dumpat(fp,fpcsv, header.data.nrows);
                // dumpat(fp, 0x3be5);
                // dumpat(fp, 0xde1a);
                // dumpat(fp, 0x1e169);
                // dumpat(fp, 0x2e4a5);
                // dumpat(fp, 0x3bd7e);
                // dumpat(fp, 0x55c07);
                }
                fclose(fp);
                fclose(fpcsv);
}

void dumpat(FILE *fp,FILE *fpcsv, int nrows) {
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
                                fprintf(fpcsv,"%s,%lf,%lf,%lf,%lf,", timebuf, data.open,
                                                data.high, data.low, data.close);
                                pos=0;
                                printint(data.additional+pos,fpcsv, extra3); pos+=extra3;
                                fprintf(fpcsv,",");
                                printint(data.additional+pos,fpcsv, extra2); pos+=extra2;
                                fprintf(fpcsv,",");
                                printint(data.additional+pos,fpcsv, extra1); pos+=extra1;
                                fprintf(fpcsv,"\n");

                }
}

void printint(char *buf,FILE *fpcsv, int size) {
                int val=0, scale=1;
                while (size--) {
                                val+=(*buf++&0xff)*scale;
                                scale<<=8;
                }
                 fprintf(fpcsv,"%5d", val);
}
