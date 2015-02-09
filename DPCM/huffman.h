#ifndef HUFFMAN_H_INCLUDED
#define HUFFMAN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
* Function prototypes
*************************************************************************/

int Huffman_Compress( unsigned char *in, unsigned char *out,
                      unsigned int insize );
void Huffman_Uncompress( unsigned char *in, unsigned char *out,
                         unsigned int insize, unsigned int outsize );


#ifdef __cplusplus
}
#endif

#endif // HUFFMAN_H_INCLUDED
