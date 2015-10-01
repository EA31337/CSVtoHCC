#include <stdio.h>
#include <time.h>
 
#pragma pack(1)
struct record {
        int separator;
        int time;
        double open, high, low, close;
        char additional[10];
};
 
int main(void) {
        FILE *fp;
        struct record data;
        int i, extra1, extra2;
        time_t time;
        struct tm *tm;
        char timebuf[80];
 
        if ((fp=fopen("2015.hcc", "rb"))==NULL) {
                perror("2015.hcc"); exit(1);
        }
        // fseek(fp, 0x3be5, 0);
        fseek(fp, 0xde1a, 0);
        for (;;) {
                if (fread(&data, sizeof(int), 1, fp)==0)
                        break;
                if ((data.separator & 0x00188884) != 0x00188884) {
                        fprintf(stderr, "bad separator at %lx\n", ftell(fp));
                        break;
                }
                extra1=data.separator>>28;
                extra2=((data.separator>>24)&0x0f)-1;
                fread(&data.time, sizeof(data)
                                -sizeof(data.separator)
                                -sizeof(data.additional)
                                +2 // 2 bytes seem always to be there
                                +extra1+extra2
                        , 1, fp);
                time=data.time;
                tm=localtime(&time);
                strftime(timebuf, sizeof timebuf, "%Y-%m-%d %H:%M:%S", tm);
                printf("%s %lf %lf %lf %lf ", timebuf, data.open,
                        data.high, data.low, data.close);
                for (i=0; i<extra1+extra2+2; i++) {
                        printf("%3d ", data.additional[i]&0xff);
                }
                putchar('\n');
        }
        fclose(fp);
}
