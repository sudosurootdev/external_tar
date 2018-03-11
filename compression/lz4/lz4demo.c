/*
  LZ4Demo - Demo CLI program using LZ4 compression
  Copyright (C) Yann Collet 2011-2012
  GPL v2 License

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

  You can contact the author at :
  - LZ4 homepage : http://fastcompression.blogspot.com/p/lz4.html
  - LZ4 source repository : http://code.google.com/p/lz4/
*/
/*
  Note : this is *only* a demo program, an example to show how LZ4 can be used.
  It is not considered part of LZ4 compression library.
  The license of LZ4 is BSD.
  The license of the demo program is GPL.
*/

//**************************************
// Compiler Options
//**************************************
// Disable some Visual warning messages
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE     // VS2005


//****************************
// Includes
//****************************
#include <stdio.h>        // fprintf, fopen, fread, _fileno(?)
#include <stdlib.h>        // malloc
#include <string.h>        // strcmp
#include <time.h>        // clock


#ifdef _WIN32
#include <io.h>            // _setmode
#include <fcntl.h>        // _O_BINARY
#endif
#include "lz4.h"
#include "lz4hc.h"
#include "bench.h"


//**************************************
// Compiler-specific functions
//**************************************
#define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)

#if defined(_MSC_VER)    // Visual Studio
#define swap32 _byteswap_ulong
#elif GCC_VERSION >= 403
#define swap32 __builtin_bswap32
#else
static inline unsigned int swap32(unsigned int x) {
    return    ((x << 24) & 0xff000000 ) |
        ((x <<  8) & 0x00ff0000 ) |
        ((x >>  8) & 0x0000ff00 ) |
        ((x >> 24) & 0x000000ff );
}
#endif


//****************************
// Constants
//****************************
#define COMPRESSOR_NAME "Compression CLI using LZ4 algorithm"
#define COMPRESSOR_VERSION ""
#define COMPILED __DATE__
#define AUTHOR "Yann Collet"
#define EXTENSION ".lz4"
#define WELCOME_MESSAGE "*** %s %s, by %s (%s) ***\n", COMPRESSOR_NAME, COMPRESSOR_VERSION, AUTHOR, COMPILED

#define CHUNKSIZE (8<<20)    // 8 MB
#define CACHELINE 64
#define ARCHIVE_MAGICNUMBER 0x184C2102
#define ARCHIVE_MAGICNUMBER_SIZE 4


//**************************************
// Architecture Macros
//**************************************
static const int one = 1;
#define CPU_LITTLE_ENDIAN  (*(char*)(&one))
#define CPU_BIG_ENDIAN     (!CPU_LITTLE_ENDIAN)
#define LITTLE_ENDIAN32(i) if (CPU_BIG_ENDIAN) { i = swap32(i); }


//**************************************
// Macros
//**************************************
#define DISPLAY(...) //fprintf(stderr, __VA_ARGS__)


//**************************************
// Special input/output
//**************************************
#define NULL_INPUT "null"
char stdinmark[] = "stdin";
char stdoutmark[] = "stdout";
#ifdef _WIN32
char nulmark[] = "nul";
#else
char nulmark[] = "/dev/null";
#endif


//**************************************
// I/O Parameters
//**************************************
static int overwrite = 0;



//****************************
// Functions
//****************************
int usage(char* exename)
{
    DISPLAY( "Usage :\n");
    DISPLAY( "      %s [arg] input output\n", exename);
    DISPLAY( "Arguments :\n");
    DISPLAY( " -c0/-c : Fast compression (default) \n");
    DISPLAY( " -c1/-hc: High compression \n");
    DISPLAY( " -d     : decompression \n");
    DISPLAY( " -y     : overwrite output \n");
    DISPLAY( " -t     : check compressed file \n");
    DISPLAY( " -b#    : Benchmark files, using # compression level\n");
    DISPLAY( " -H     : help (this text)\n");
    DISPLAY( "input   : can be 'stdin' (pipe) or a filename\n");
    DISPLAY( "output  : can be 'stdout'(pipe) or a filename or 'null'\n");
    return 0;
}


int badusage(char* exename)
{
    DISPLAY("Wrong parameters\n");
    usage(exename);
    return 0;
}

int get_paddFileHandler(char* output_filename, FILE* pfinput, int paddSize, int startPadd)
{

    DISPLAY( "get_paddFileHandler in - %d, %d, name - %s\n", paddSize, startPadd, output_filename);

    FILE* pfoutputpadd;
    char output_padd_filename[128];
    char* padd_buf;

    padd_buf = (char*)malloc(paddSize);
    if(!padd_buf)
    {
        DISPLAY( "padd_buf is NULL \n");
        return 888;
    }
    sprintf(output_padd_filename, "%s%s", output_filename, "_padd");

    DISPLAY( "output_padd_filename - %s\n", output_padd_filename);


    pfoutputpadd= fopen(output_padd_filename, "wb");
    if ( pfoutputpadd==0) { DISPLAY( "Pb opening %s\n", output_padd_filename); return 15; }

    fseek(pfinput, startPadd, 0);
    while(fread(padd_buf, 1, 1, pfinput) > 0)
    {
        fwrite(padd_buf, 1, 1, pfoutputpadd);
    }

    free(padd_buf);
    fclose(pfoutputpadd);

    return 0;
}


int get_fileHandle(char* input_filename, char* output_filename, FILE** pfinput, FILE** pfoutput)
{

    if (!strcmp (input_filename, stdinmark))
    {
        DISPLAY( "Using stdin for input\n");
        *pfinput = stdin;
#ifdef _WIN32 // Need to set stdin/stdout to binary mode specifically for windows
        _setmode( _fileno( stdin ), _O_BINARY );
#endif
    }
    else
    {
        *pfinput = fopen(input_filename, "rb");
    }

    if (!strcmp (output_filename, stdoutmark))
    {
        DISPLAY( "Using stdout for output\n");
        *pfoutput = stdout;
#ifdef _WIN32 // Need to set stdin/stdout to binary mode specifically for windows
        _setmode( _fileno( stdout ), _O_BINARY );
#endif
    }
    else
    {
        // Check if destination file already exists
        *pfoutput=0;
        if (output_filename != nulmark) *pfoutput = fopen( output_filename, "rb" );
        if (*pfoutput!=0)
        {
            char ch;
            fclose(*pfoutput);
            DISPLAY( "Warning : %s already exists\n", output_filename);
            if (!overwrite)
            {
                DISPLAY( "Overwrite ? (Y/N) : ");
                ch = getchar();
                if (ch!='Y') { DISPLAY( "Operation aborted : %s already exists\n", output_filename);  return 12; }
            }
        }
        *pfoutput = fopen( output_filename, "wb" );
    }

    if ( *pfinput==0 ) { DISPLAY( "Pb opening %s\n", input_filename);  return 13; }
    if ( *pfoutput==0) { DISPLAY( "Pb opening %s\n", output_filename); return 14; }

    return 0;
}



int compress_file(char* input_filename, char* output_filename, int compressionlevel)
{
    int (*compressionFunction)(const char*, char*, int);
    unsigned long long filesize = 0;
    unsigned long long compressedfilesize = ARCHIVE_MAGICNUMBER_SIZE;
    unsigned int u32var;
    char* in_buff;
    char* out_buff;
    FILE* finput;
    FILE* foutput;
    int r;
    int displayLevel = (compressionlevel>0);
    clock_t start, end;
    size_t sizeCheck;


    // Init
    switch (compressionlevel)
    {
    case 0 : compressionFunction = LZ4_compress; break;
    case 1 : compressionFunction = LZ4_compressHC; break;
    default : compressionFunction = LZ4_compress;
    }
    start = clock();
    r = get_fileHandle(input_filename, output_filename, &finput, &foutput);
    if (r) return r;

    // Allocate Memory
    in_buff = (char*)malloc(CHUNKSIZE);
    out_buff = (char*)malloc(LZ4_compressBound(CHUNKSIZE));
    if (!in_buff || !out_buff) { DISPLAY("Allocation error : not enough memory\n"); return 21; }

    // Write Archive Header
    u32var = ARCHIVE_MAGICNUMBER;
    LITTLE_ENDIAN32(u32var);
    *(unsigned int*)out_buff = u32var;
    sizeCheck = fwrite(out_buff, 1, ARCHIVE_MAGICNUMBER_SIZE, foutput);
    if (sizeCheck!=ARCHIVE_MAGICNUMBER_SIZE) { DISPLAY("write error\n"); return 22; }

    // Main Loop
    while (1)
    {
        int outSize;
        // Read Block
        int inSize = (int) fread(in_buff, (size_t)1, (size_t)CHUNKSIZE, finput);
        if( inSize<=0 ) break;
        filesize += inSize;
        if (displayLevel) {
            DISPLAY("Read : %i MB  \r", (int)(filesize>>20));
        }

        // Compress Block
        outSize = compressionFunction(in_buff, out_buff+4, inSize);
        compressedfilesize += outSize+4;
        if (displayLevel) {
            DISPLAY("Read : %i MB  ==> %.2f%%\r", (int)(filesize>>20), (double)compressedfilesize/filesize*100);
        }

        // Write Block
        LITTLE_ENDIAN32(outSize);
        * (unsigned int*) out_buff = outSize;
        LITTLE_ENDIAN32(outSize);
        sizeCheck = fwrite(out_buff, 1, outSize+4, foutput);
        if (sizeCheck!=(size_t)(outSize+4)) { DISPLAY("write error\n"); return 11; }
    }

    // Status
    end = clock();
    DISPLAY( "Compressed %llu bytes into %llu bytes ==> %.2f%%\n",
        (unsigned long long) filesize, (unsigned long long) compressedfilesize, (double)compressedfilesize/filesize*100);
    {
        double seconds = (double)(end - start)/CLOCKS_PER_SEC;
        DISPLAY( "Done in %.2f s ==> %.2f MB/s\n", seconds, (double)filesize / seconds / 1024 / 1024);
    }

    // Close & Free
    free(in_buff);
    free(out_buff);
    fclose(finput);
    fclose(foutput);

    return 0;
}

int    getFileSize(FILE* pFile)
{
    int        iSize;
    int        iCur;

    //current size
    fseek(pFile, 0, 1);
    iCur = ftell(pFile);
    //file size
    fseek(pFile, 0, 2);
    iSize = ftell(pFile);

    fseek(pFile, iCur, 0);

    DISPLAY( "iCur-%d > iSize-%d pFile-%d\n", iCur, iSize, pFile);
    return (iSize);
}


int decode_file(char* input_filename, char* output_filename)
{
    unsigned long long filesize = 0;
    char* in_buff;
    char* out_buff;
    size_t uselessRet;
    int sinkint;
    unsigned int chunkSize;
    FILE* finput;
    FILE* foutput;
    clock_t start, end;
    int r;
    size_t sizeCheck;
    int TotalchunkSize = 0;

    // Init
    start = clock();
    r = get_fileHandle(input_filename, output_filename, &finput, &foutput);
    if (r) return r;

    // Allocate Memory
    in_buff = (char*)malloc(LZ4_compressBound(CHUNKSIZE));
    out_buff = (char*)malloc(CHUNKSIZE);
    if (!in_buff || !out_buff) { DISPLAY("Allocation error : not enough memory\n"); return 31; }

    // Check Archive Header
    chunkSize = 0;
    uselessRet = fread(&chunkSize, 1, ARCHIVE_MAGICNUMBER_SIZE, finput);
    LITTLE_ENDIAN32(chunkSize);
    if (chunkSize != ARCHIVE_MAGICNUMBER) { DISPLAY("Unrecognized header : file cannot be decoded\n"); return 32; }

    // Main Loop
    while (1)
    {
        // Block Size
        uselessRet = fread(&chunkSize, 1, 4, finput);

        if( uselessRet==0 ) break;   // Nothing to read : file read is completed
        LITTLE_ENDIAN32(chunkSize);
        if (chunkSize == ARCHIVE_MAGICNUMBER)
            continue;   // appended compressed stream

        DISPLAY( "TotalchunkSize-%d > chunkSize-%d \n", TotalchunkSize, chunkSize);

        //adding escape condition
        //if chunksize bigger than file size, do not decode anymore.
        TotalchunkSize += chunkSize+4;
        if(TotalchunkSize > getFileSize(finput))
        {
            DISPLAY( "TotalchunkSize-%d > LGUpdateUtil_GetFileSize-%d \n", TotalchunkSize, getFileSize(finput));
            //kernel temp file output.
            get_paddFileHandler(output_filename, finput, (getFileSize(finput)-(TotalchunkSize - chunkSize)), TotalchunkSize-chunkSize);
            break;
        }

        // Read Block
        uselessRet = fread(in_buff, 1, chunkSize, finput);

        // Decode Block
        sinkint = LZ4_uncompress_unknownOutputSize(in_buff, out_buff, chunkSize, CHUNKSIZE);
        if (sinkint < 0) { DISPLAY("Decoding Failed ! Corrupted input !\n"); return 33; }
        filesize += sinkint;

        // Write Block
        sizeCheck = fwrite(out_buff, 1, sinkint, foutput);
        if (sizeCheck != (size_t)sinkint) { DISPLAY("write error\n"); return 34; }
    }

    // Status
    end = clock();
    DISPLAY( "Successfully decoded %llu bytes \n", (unsigned long long)filesize);
    {
        double seconds = (double)(end - start)/CLOCKS_PER_SEC;
        DISPLAY( "Done in %.2f s ==> %.2f MB/s\n", seconds, (double)filesize / seconds / 1024 / 1024);
    }

    // Close & Free
    free(in_buff);
    free(out_buff);
    fclose(finput);
    fclose(foutput);

    return 0;
}


int main(int argc, char** argv)
{
    int i,
        cLevel=0,
        decode=0,
        bench=0,
        filenamesStart=2;
    char* exename=argv[0];
    char* input_filename=0;
    char* output_filename=0;
    char nullinput[] = NULL_INPUT;
    char extension[] = EXTENSION;

    // Welcome message
    DISPLAY( WELCOME_MESSAGE);

    if (argc<2) { badusage(exename); return 1; }

    for(i=1; i<argc; i++)
    {
        char* argument = argv[i];

        if(!argument) continue;   // Protection if argument empty

        // Decode command (note : aggregated commands are allowed)
        if (argument[0]=='-')
        {
            while (argument[1]!=0)
            {
                argument ++;

                switch(argument[0])
                {
                    // Display help on usage
                case 'H': usage(exename); return 0;

                    // Compression (default)
                case 'c': if ((argument[1] >='0') && (argument[1] <='1')) { cLevel=argument[1] - '0'; argument++; } break;
                case 'h': if (argument[1]=='c') { cLevel=1; argument++; } break;

                    // Decoding
                case 'd': decode=1; break;

                    // Bench
                case 'b': bench=1;
                    if ((argument[1] >='0') && (argument[1] <='1')) { cLevel=argument[1] - '0'; argument++; }
                    break;

                    // Modify Block Size (benchmark only)
                case 'B':
                    if ((argument[1] >='0') && (argument[1] <='9'))
                    {
                        int B = argument[1] - '0';
                        int S = 1 << (10 + 2*B); BMK_SetBlocksize(S);
                        argument++;
                    }
                    break;

                    // Modify Nb Iterations (benchmark only)
                case 'i':
                    if ((argument[1] >= '0') && (argument[1] <= '9'))
                    {
                        int iters = argument[1] - '0';
                        BMK_SetNbIterations(iters);
                        argument++;
                    }
                    break;

                    // Pause at the end (benchmark only)
                case 'p': BMK_SetPause(); break;

                    // Test
                case 't': decode=1; output_filename=nulmark; break;

                    // Overwrite
                case 'y': overwrite=1; break;

                    // Unrecognised command
                default : badusage(exename); return 1;
                }
            }
            continue;
        }

        // first provided filename is input
        if (!input_filename) { input_filename=argument; filenamesStart=i; continue; }

        // second provided filename is output
        if (!output_filename)
        {
            output_filename=argument;
            if (!strcmp (output_filename, nullinput)) output_filename = nulmark;
            continue;
        }
    }

    // No input filename ==> Error
    if(!input_filename) { badusage(exename); return 1; }

    if (bench) return BMK_benchFile(argv+filenamesStart, argc-filenamesStart, cLevel);

    // No output filename ==> build one automatically (for compression only)
    if (!output_filename)
    {
        if (!decode)
        {
            int i=0, l=0;
            while (input_filename[l]!=0) l++;
            output_filename = (char*)calloc(1,l+5);
            for (i=0;i<l;i++) output_filename[i] = input_filename[i];
            for (i=l;i<l+4;i++) output_filename[i] = extension[i-l];
        }
        else
        {
            badusage(exename);
            return 1;
        }
    }

    if (decode) return decode_file(input_filename, output_filename);

    return compress_file(input_filename, output_filename, cLevel);   // Compression is 'default' action

}
