#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define	SIGN_BIT	(0x80)
#define	QUANT_MASK	(0xf)
#define	NSEGS		(8)
#define	SEG_SHIFT	(4)
#define	SEG_MASK	(0x70)

clock_t begin, end;

typedef struct header_file{
    char chunk_id[4];
    int chunk_size;
    char format[4];
    char subchunk1_id[4];
    int subchunk1_size;
    short int audio_format;
    short int num_channels;
    int sample_rate;
    int byte_rate;
    short int block_align;
    short int bits_per_sample;
    char subchunk2_id[4];
    int subchunk2_size;
} header;


static short seg_aend[8] = {0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF};

static short search(short val, short *table, short size){
	short i;
	for (i = 0; i < size; i++) {
		if (val <= *table++)
			return (i);
	}
	return (size);
}

typedef struct header_file* header_p;

/*
 *	    Linear Input Code	    Compressed Code
 *	  ----------------------	---------------
 *	        0000000wxyza			000wxyz
 *	        0000001wxyza			001wxyz
 *	        000001wxyzab			010wxyz
 *	        00001wxyzabc			011wxyz
 *	        0001wxyzabcd			100wxyz
 *	        001wxyzabcde			101wxyz
 *	        01wxyzabcdef			110wxyz
 *	        1wxyzabcdefg			111wxyz
 */

unsigned char Snack_Lin2Alaw(short	pcm_val){
	short		mask;
	short		seg;
	unsigned char	aval;

	pcm_val = pcm_val >> 3;

	if (pcm_val >= 0) {
		mask = 0xD5;
	} else {
		mask = 0x55;
		pcm_val = -pcm_val - 1;
	}

	seg = search(pcm_val, seg_aend, 8);

	if (seg >= 8)
		return (unsigned char) (0x7F ^ mask);
	else {
		aval = (unsigned char) seg << SEG_SHIFT;
		if (seg < 2)
			aval |= (pcm_val >> 1) & QUANT_MASK;
		else
			aval |= (pcm_val >> seg) & QUANT_MASK;
		return (aval ^ mask);
	}
}

short Snack_Alaw2Lin(unsigned char	a_val){
	short		t;
	short		seg;

	a_val ^= 0x55;

	t = (a_val & QUANT_MASK) << 4;
	seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
	switch (seg) {
	case 0:
		t += 8;
		break;
	case 1:
		t += 0x108;
		break;
	default:
		t += 0x108;
		t <<= seg - 1;
	}
	return ((a_val & SIGN_BIT) ? t : -t);
}

/** funkcija konkatenira dva stringa
  * char *s1 prvi string
  * char *s2 drugi string
  * result *prvi*drugi
  */
char* concat(char *s1, char *s2){
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}


int main(){
	int BUFSIZE = 1;
	short int buff16[BUFSIZE];
	header_p meta = (header_p)malloc(sizeof(header));
	int nb;
    int z = 0;

    char *preunc = "testcase";
    char *posunc = ".wav";
    char *prec = "compressed";
    char *posc = ".alaw";
    char *predec = "decompressedalaw";

    char *filenameunc;
    char *filenamec;
    char mid[3];
    mid[2] = '\0';

	freopen("output.txt", "w", stdout);

    for(z = 1; z <= 20; z++){
        begin = clock();

        mid[0] = '0' + z / 10;
        mid[1] = '0' + z % 10;

        filenameunc = concat(concat(preunc,mid), posunc);
        filenamec = concat(concat(prec,mid), posc);

        printf("%s - %s\t", filenameunc , filenamec);

        FILE * infile = fopen(filenameunc,"rb");
        FILE * outfile = fopen(filenamec,"wb");

        if (infile){
            fread(meta, 1, sizeof(header), infile);
            fwrite(meta,1, sizeof(*meta), outfile);

		    while (!feof(infile)){
			    nb = fread(buff16,2,BUFSIZE,infile);

                short int val16 = buff16[0];
                unsigned char val8 = Snack_Lin2Alaw(val16);

			    fwrite(&val8,1,nb,outfile);
		    }
        }

        end = clock();
        printf("%f\n", (double)(end - begin) / CLOCKS_PER_SEC);
    }

    for(z = 1; z <= 20; z++){
        begin = clock();

        mid[0] = '0' + z / 10;
        mid[1] = '0' + z % 10;

        filenameunc = concat(concat(prec,mid), posc);
        filenamec = concat(concat(predec,mid), posunc);

        printf("%s - %s\t", filenameunc , filenamec);

        FILE * infile = fopen(filenameunc,"rb");
        FILE * outfile = fopen(filenamec,"wb");

        if (infile){

            fread(meta, 1, sizeof(header), infile);
            fwrite(meta,1, sizeof(*meta), outfile);

            while (!feof(infile)){
                unsigned char val8;

                nb = fread(&val8,1,BUFSIZE,infile);
                short int val16 = Snack_Alaw2Lin(val8);

                fwrite(&val16,2,nb,outfile);
            }
        }
        end = clock();
        printf("%f\n", (double)(end - begin) / CLOCKS_PER_SEC);
    }
return 0;
}
