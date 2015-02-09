#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "huffman.h"

clock_t begin, end;                 // vreme pocetka i kraja obradjivanja nekog fajla


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


typedef struct header_file* header_p;


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
	int k = 0;                              // iterator kroz semplove
	int i,z;                                // brojaci za for cikluse
	int no_of_semples;                      // ukupan broj semplova
	short int *buff16_temp;                 // trenutna vrednost semplova
    short int *buff16_pre;                  // prethodna vrednost semplova
    short dif;                              // razlika izmedju trenutne i prethodne vrednosti semplova
    int outsize, insize;                    // velicine baffera nekompresovanih-kompresovanih
    unsigned char *buff_for_comp;           // buffer spremnih podataka za kompresiju
    unsigned char *buff_comp;               // buffer kompresovanih podataka spreman za upisivanje
    unsigned char *buff_uncomp;             // buffer unkompresovanih podataka spreman za upisivanje
    unsigned char *buff_for_uncomp;         // buffer podataka spreminh za unkompresovanje
	header_p meta = (header_p)malloc(sizeof(header)); // u njoj cuvamo header
	int nb;                                 // koliko bajtova je procitano

    // promenljive za gradjenje fajlova
    char *preunc = "testcase";              // prefix ulaznog fajla
    char *posunc = ".wav";                  // extenzija ulaznog (dekompresovanog fajla)
    char *prec = "compressed";              // prefix kompresovanog fajla
    char *posc = ".dpcm";                   // extenzija kompresovanog fajla
    char *predec = "decompresseddpcm";          // prefix dekompresovanog fajla
    char *filenameunc;                      // ime fajla za citanje
    char *filenamec;                        // ime fajla za upisivanje
    char mid[3];                            // string za gradjenje iteratora 01,02,03 ...
    mid[2] = '\0';                          // da bi bio string zavrsicemo ga sa '\0'


    freopen("output.txt", "w", stdout);     //u fajl upisujemo vremena izvrsavanja kompresije - dekompresije


    for(z = 1; z <= 20; z++){

        begin = clock();
        k = 0;

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

            no_of_semples = meta->subchunk2_size / meta->block_align;

            buff16_temp = (short *) malloc(meta->num_channels * sizeof(short));
            buff16_pre  = (short *) malloc(meta->num_channels * sizeof(short));

            buff_for_comp = (unsigned char *) malloc((sizeof(short)/ sizeof(char))*meta->num_channels * no_of_semples * sizeof(unsigned char));
            buff_comp = (unsigned char *) malloc((sizeof(short)/ sizeof(char))*meta->num_channels * no_of_semples * sizeof(unsigned char));

            /*read first semple for all chanels*/
            nb = fread(buff16_temp, sizeof(short), meta->num_channels, infile);
            fwrite(buff16_temp, 2, nb, outfile);

            for(i = 0; i < meta->num_channels; i++)
                buff16_pre[i] = buff16_temp[i];

            while (!feof(infile)){

                nb = fread(buff16_temp, 2, meta->num_channels, infile);

                for(i = 0; i < meta->num_channels; i++){
                    dif = buff16_temp[i] -  buff16_pre[i];

                    buff_for_comp[k+i*2] = (dif & 0xFF00) >> 8;
                    buff_for_comp[k+i*2+1] = (dif & 0xFF);

                    buff16_pre[i] = buff16_temp[i];
                }
                k+=4;
            }
        }
        outsize = Huffman_Compress( buff_for_comp, buff_comp, k );

        fwrite(&outsize, sizeof(outsize),sizeof(outsize)/sizeof(int), outfile);
        fwrite(buff_comp, 1, outsize, outfile);

        free(buff16_temp);
        free(buff16_pre);
        free(buff_for_comp);
        free(buff_comp);

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

            no_of_semples = meta->subchunk2_size / meta->block_align;

            outsize = no_of_semples * sizeof(int);
            buff16_temp = (short *) malloc(meta->num_channels * sizeof(short));

            buff_uncomp = (unsigned char *) malloc((sizeof(short)/ sizeof(char))*meta->num_channels * no_of_semples * sizeof(unsigned char));

            /*read first semple for all chanels*/
            nb = fread(buff16_temp, sizeof(short), meta->num_channels, infile);
            fwrite(buff16_temp, 2, nb, outfile);


            nb = fread(&insize, sizeof(int), 1, infile);;

            buff_for_uncomp = (unsigned char *) malloc(insize * sizeof(unsigned char));

            while (!feof(infile)){
                nb = fread(buff_for_uncomp, 1, insize, infile);
            }
        }
        Huffman_Uncompress( buff_for_uncomp, buff_uncomp, insize, outsize );

        for(i = 0 ; i < outsize; i+=2){
               dif = 0;
               dif = (buff_uncomp[i] << 8) | buff_uncomp[i+1];

               buff16_temp[0] = buff16_temp[0] + dif;

               i+=2;

               dif = 0;
               dif = (buff_uncomp[i] << 8) | buff_uncomp[i+1];

               buff16_temp[1] = buff16_temp[1] + dif;

               fwrite(buff16_temp, 2, 2, outfile);
        }

        free(buff_for_uncomp);
        free(buff_uncomp);
        free(buff16_temp);

        end = clock();
        printf("%f\n", (double)(end - begin) / CLOCKS_PER_SEC);
    }

return 0;
}
