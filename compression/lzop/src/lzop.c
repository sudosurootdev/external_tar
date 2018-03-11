/* lzop.c
   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include<unistd.h>
#include<string.h>
#include <sys/ioctl.h>


#include <lzop.h>
#include <lzo1x.h>

// flag
#define        COMMAND_OPTION_USE_STD_MODE            0x00000001
#define        COMMAND_OPTION_DECOMPRESS            0x00000002
#define        COMMAND_OPTION_LEVEL_LOW            0x00000004
#define        COMMAND_OPTION_LEVEL_MIDDLE            0x00000008
#define        COMMAND_OPTION_LEVEL_HIGH            0x00000010


#define IN_LEN      (256*1024ul)
#define OUT_LEN     (IN_LEN + IN_LEN / 16 + 64 + 3 + 105943)

static unsigned char __LZO_MMODEL in  [ IN_LEN ];
static unsigned char __LZO_MMODEL out [ OUT_LEN ];

#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]



//------------------------------------------------------------------------------------------------------------------------
// Local Variable
//------------------------------------------------------------------------------------------------------------------------

static HEAP_ALLOC(wrkmem, LZO1X_999_MEM_COMPRESS);
static const unsigned char lzop_magic[9] =
    { 0x89, 0x4c, 0x5a, 0x4f, 0x00, 0x0d, 0x0a, 0x1a, 0x0a };
int                g_bLog;


//------------------------------------------------------------------------------------------------------------------------
// Local Function
//------------------------------------------------------------------------------------------------------------------------
lzo_uint32         Lzop_Getbe32(const unsigned char *b);
void             Lzop_Read32(int ft, lzo_uint32 *v);
void             Lzop_Setbe32(unsigned char *b, lzo_uint32 v);
void             Lzop_Write32(int *fp, lzo_uint32 v);
void             Lzop_Setbe16(unsigned char *b, lzo_uint32 v);
void             Lzop_Write16(int *fp, lzo_uint32 v);
void             Lzop_Setbe8(unsigned char *b, lzo_uint32 v);
void             Lzop_Write8(int *fp, lzo_uint32 v);
void             Lzop_Writeheader(int fp, unsigned int uiVersion, unsigned int uiLowTime);
int                Lzop_ReadHeader(int fp);


unsigned int     Lzop_GetOption(int iArgc, unsigned int *uiVersion, unsigned int *uiLowTime,  char *szArgv[]);
char *             Lzop_GetFilename(int iArgc, char *szArgv[], int iTargetCount);

int             Lzop_DoCompress(int fin, int fout, unsigned int uiVersion, unsigned int uiLowTime);
int                Lzop_DoDeCompress(int fin, int fout);

unsigned int    Lzop_StringToHex(unsigned char *strHex);
void            Lzop_GetVaildeHexString(unsigned char *strHex);
unsigned int    Lzop_GetHexChar(unsigned char    cCh);


//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
int main(int iArgc, char *szArgv[])
{
    int                fin, fout;
    unsigned int    iOption;
    unsigned int     uiVersion;
    unsigned int     uiLowTime;


    if (iArgc < 0 && szArgv == NULL)   /* avoid warning about unused args */
        return 0;


    iOption = Lzop_GetOption(iArgc, &uiVersion, &uiLowTime, szArgv);


    if (iOption & COMMAND_OPTION_USE_STD_MODE)
    {
        g_bLog = 0;

        fin     = (fileno(stdin));
        fout     = (fileno(stdout));
    }
    else
    {
        g_bLog = 1;

        fin     = open(Lzop_GetFilename(iArgc, szArgv, 0), O_RDONLY);
        fout    = open(Lzop_GetFilename(iArgc, szArgv, 1), O_WRONLY | O_CREAT | O_TRUNC, 0777);
    }

    if (iOption & COMMAND_OPTION_DECOMPRESS)
    {
        Lzop_DoDeCompress(fin, fout);
    }
    else
    {
        Lzop_DoCompress(fin, fout, uiVersion, uiLowTime);
    }


    if (iOption & COMMAND_OPTION_USE_STD_MODE)
    {
        //
    }
    else
    {
        close(fin);
        close(fout);
    }

    return 0;
}




//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
lzo_uint32 Lzop_Getbe32(const unsigned char *b)
{
    lzo_uint32 v;

    v  = (lzo_uint32) b[3] <<  0;
    v |= (lzo_uint32) b[2] <<  8;
    v |= (lzo_uint32) b[1] << 16;
    v |= (lzo_uint32) b[0] << 24;
    return v;
}

//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
void Lzop_Read32(int ft, lzo_uint32 *v)
{
    unsigned char b[4];

    read(ft,b,4);

    *v = Lzop_Getbe32(b);
}



//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
void Lzop_Setbe32(unsigned char *b, lzo_uint32 v)
{
    b[3] = (unsigned char) (v >>  0);
    b[2] = (unsigned char) (v >>  8);
    b[1] = (unsigned char) (v >> 16);
    b[0] = (unsigned char) (v >> 24);
}


//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
void Lzop_Write32(int *fp, lzo_uint32 v)
{
    unsigned char b[4];
    Lzop_Setbe32(b,v);
    write(fp, b, 4);
}

//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
void Lzop_Setbe16(unsigned char *b, lzo_uint32 v)
{
    b[1] = (unsigned char) (v >> 0);
    b[0] = (unsigned char) (v >> 8);
}

//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
void Lzop_Write16(int *fp, lzo_uint32 v)
{
    unsigned char b[2];
    Lzop_Setbe16(b,v);
    write(fp, b, 2);
}

//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
void Lzop_Setbe8(unsigned char *b, lzo_uint32 v)
{
    b[0] = (unsigned char) (v >> 0);
}


//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
void Lzop_Write8(int *fp, lzo_uint32 v)
{
    unsigned char b[1];
    Lzop_Setbe8(b,v);
    write(fp, b, 1);
}


//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
void Lzop_WriteHeader(int fp, unsigned int uiVersion, unsigned int uiLowTime)
{
    unsigned int    uiCrc;
    unsigned char    szHeader[34];
    int                i;


    write(fp, lzop_magic, sizeof(lzop_magic));

    i = 0;

    // lzop version
    // szHeader[i++]    = lzop[0];
    // szHeader[i++]    = lzop[1];

    // lib version
    // szHeader[i++]    = lzo[0];
    // szHeader[i++]    = lzo[1];

    Lzop_Setbe32(&(szHeader[i]), uiVersion);
    i += 4;

    // opt_filter
    szHeader[i++]    = 0x09;
    szHeader[i++]    = 0x40;

    // M_LZO1X_999
    szHeader[i++]    = 0x03;

    // Level
    szHeader[i++]    = 0x09;

    // F_OS_UNIX F_ADLER32_D F_ADLER32_C F_STDIN  F_STDOUT
    szHeader[i++]    = 0x03;
    szHeader[i++]    = 0x00;
    szHeader[i++]    = 0x00;
    szHeader[i++]    = 0x0D;

    // Mode
    szHeader[i++]    = 0x00;
    szHeader[i++]    = 0x00;
    szHeader[i++]    = 0x00;
    szHeader[i++]    = 0x00;

    // low time
//    szHeader[i++]    = uiLowTime[0];
//    szHeader[i++]    = uiLowTime[1];
//    szHeader[i++]    = uiLowTime[2];
//    szHeader[i++]    = uiLowTime[3];
    Lzop_Setbe32(&(szHeader[i]), uiLowTime);
    i += 4;

    // high time
    szHeader[i++]    = 0x0;
    szHeader[i++]    = 0x0;
    szHeader[i++]    = 0x0;
    szHeader[i++]    = 0x0;

    // file name size
    szHeader[i++]    = 0x0;


    write(fp, szHeader, i);

    uiCrc = lzo_adler32(1, szHeader, i);

    Lzop_Write32(fp, uiCrc);            // CRC

}



//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
int Lzop_ReadHeader(int fp)
{
    unsigned int    uiCrc;
    unsigned char    szHeader[34];
    unsigned char    szMagic[9];
    int                i;


    read(fp, szMagic, sizeof(lzop_magic));

    if (strcmp(lzop_magic, szMagic))
    {
        return -1;
    }

    i = 0;

    // lzop version
    szHeader[i++]    = 0x10;
    szHeader[i++]    = 0x30;

    // lib version
    szHeader[i++]    = 0x20;
    szHeader[i++]    = 0x40;

    // opt_filter
    szHeader[i++]    = 0x09;
    szHeader[i++]    = 0x40;

    // M_LZO1X_999
    szHeader[i++]    = 0x03;

    // Level
    szHeader[i++]    = 0x09;

    // F_OS_UNIX F_ADLER32_D F_ADLER32_C F_STDIN  F_STDOUT
    szHeader[i++]    = 0x03;
    szHeader[i++]    = 0x00;
    szHeader[i++]    = 0x00;
    szHeader[i++]    = 0x0D;

    // Mode
    szHeader[i++]    = 0x00;
    szHeader[i++]    = 0x00;
    szHeader[i++]    = 0x00;
    szHeader[i++]    = 0x00;

    // low time
    szHeader[i++]    = 0;
    szHeader[i++]    = 0;
    szHeader[i++]    = 0;
    szHeader[i++]    = 0;

    // high time
    szHeader[i++]    = 0x0;
    szHeader[i++]    = 0x0;
    szHeader[i++]    = 0x0;
    szHeader[i++]    = 0x0;

    // file name size
    szHeader[i++]    = 0x0;


    read(fp, szHeader, i);

    read(fp, &uiCrc, 4);

    if (uiCrc, lzo_adler32(1, szHeader, i))
    {
        return -1;
    }

    return 0;
}


//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
unsigned int Lzop_GetOption(int iArgc, unsigned int *uiVersion, unsigned int *uiLowTime, char *szArgv[])
{
    int                i;
    unsigned int    uiOption;

    uiOption = COMMAND_OPTION_USE_STD_MODE;
    uiOption |= COMMAND_OPTION_LEVEL_MIDDLE;

    for (i = 1; i < iArgc; i++)
    {
        if (!strcmp("-d", szArgv[i]))
        {
            uiOption |= COMMAND_OPTION_DECOMPRESS;
        }
        else if (!strcmp("-0", szArgv[i]))
        {
            uiOption &= ~COMMAND_OPTION_LEVEL_MIDDLE;
            uiOption |= COMMAND_OPTION_LEVEL_LOW;
        }
        else if (!strcmp("-5", szArgv[i]))
        {
            uiOption |= COMMAND_OPTION_LEVEL_MIDDLE;
        }
        else if (!strcmp("-9", szArgv[i]))
        {
            uiOption &= ~COMMAND_OPTION_LEVEL_MIDDLE;
            uiOption |= COMMAND_OPTION_LEVEL_HIGH;
        }
        else if (!strcmp("-v", szArgv[i]))
        {
            *uiVersion = Lzop_StringToHex(szArgv[++i]);
        }
        else if (!strcmp("-t", szArgv[i]))
        {
            *uiLowTime = Lzop_StringToHex(szArgv[++i]);
        }
        else if (szArgv[i][0] == '-')
        {
//            printf("%s dose not support option\n", szArgv[i]);
        }
        else
        {
//            printf("filename : %s\n", szArgv[i]);
            uiOption &= ~COMMAND_OPTION_USE_STD_MODE;
        }
    }

    return uiOption;
}



//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
char * Lzop_GetFilename(int iArgc, char *szArgv[], int iTargetCount)
{
    int                i;
    unsigned int    iCount;

    iCount = 0;

    for (i = 1; i < iArgc; i++)
    {
        if (szArgv[i][0] == '-')
        {
            // option
        }
        else
        {
            if (iCount++ == iTargetCount)
            {
                return szArgv[i];
            }
        }
    }

    return NULL;
}


//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
int Lzop_DoCompress(int fin, int fout, unsigned int uiVersion, unsigned int uiLowTime)
{
    int                 r;
    lzo_uint         in_len;
    lzo_uint         out_len;
    int                iLen;
    int                iReadLen;
    int                iCrc;


/*
 * Step 1: initialize the LZO library
 */
    if (lzo_init() != LZO_E_OK)
    {
        return 3;
    }

    iCrc = 0;

    in_len = IN_LEN;

    Lzop_WriteHeader(fout, uiVersion, uiLowTime);

    for (;;)
    {
        iReadLen = 0;
        while (iReadLen < IN_LEN)
        {
            iLen = read(fin, (in + iReadLen), (in_len - iReadLen));

            if (iLen <= 0)
            {
                break;
            }

            iReadLen += iLen;
        }

        Lzop_Write32(fout, iReadLen);

        if (iReadLen <= 0)
        {
            break;
        }

        r = lzo1x_999_compress_level(in,iReadLen,out,&out_len,wrkmem, NULL, 0, 0, 9);
        if (r == LZO_E_OK)
        {
            lzo1x_optimize(out, out_len, in, &iReadLen, NULL);
        }
        else
        {
            return 2;
        }
        iCrc = lzo_adler32(1,in,iReadLen);

        if (out_len >= in_len)
        {
            Lzop_Write32(fout, iReadLen);
            Lzop_Write32(fout, iCrc);
            write(fout, in, iReadLen);
        }
        else
        {
            Lzop_Write32(fout, out_len);
            Lzop_Write32(fout, iCrc);
            write(fout, out, out_len);
        }
    }

    return 0;
}




//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
int Lzop_DoDeCompress(int fin, int fout)
{
    int                 r;
    lzo_uint         in_len;
    lzo_uint         out_len;
    int                iReadLen;
    int                iCrc;


/*
 * Step 1: initialize the LZO library
 */
    if (lzo_init() != LZO_E_OK)
    {
        return 3;
    }

    iCrc = 0;

    in_len = IN_LEN;

    Lzop_ReadHeader(fin);

    for (;;)
    {
        Lzop_Read32(fin, &out_len);
        Lzop_Read32(fin, &iReadLen);
        Lzop_Read32(fin, &iCrc);

        if (out_len <= 0)
        {
            break;
        }

        iCrc = lzo_adler32(1,in,iReadLen);

        //printf("iReadLen %x\n", iReadLen);
        //printf("iReadLen %x\n", out_len);

        read(fin, in, iReadLen);

        if (out_len == iReadLen)
        {
            //printf("1231412312314 %x\n", out_len);
            //read(fout, in, iReadLen);
        }
        else
        {
            //printf("iReadLen %x\n", out_len);

            r = lzo1x_decompress_safe(in,iReadLen,out,&out_len,wrkmem);
            if (r == LZO_E_OK)
            {
                //
            }
            else
            {
                return 2;
            }
        }

        write(fout, out, out_len);
    }

    return 0;
}




//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
unsigned int        Lzop_StringToHex(unsigned char *strHex)
{
    unsigned int            iHex;
    unsigned int            iLen;
    unsigned int            i;

    Lzop_GetVaildeHexString(strHex);

    iHex    = 0;
    iLen    = strlen(strHex);

    // 0x0112
    for (i = 2; i < iLen; i++)
    {

        iHex = iHex + ((Lzop_GetHexChar(strHex[i])) << (4 * (iLen - i - 1)));
    }

    return iHex;
}


//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
void    Lzop_GetVaildeHexString(unsigned char *strHex)
{
    unsigned int            iLen;
    unsigned int            i;

    iLen    = strlen(strHex);

    for (i = 2; i < iLen; i++)
    {
        if (Lzop_GetHexChar(strHex[i]) < 0)
        {
            strHex[i] = '\0';
            break;
        }
    }
}



//------------------------------------------------------------------------------------------------------------------------
// @Function        :
// @Description        :
// @Parameters        :
// @Return Value    :
//------------------------------------------------------------------------------------------------------------------------
// @ID             @Date            @Name            @Description
//------------     ----------        -----------        -----------------------------------------
//
//------------------------------------------------------------------------------------------------------------------------
unsigned int        Lzop_GetHexChar(unsigned char    cCh)
{
    unsigned int        iNumber;

    if ('0' <= cCh && cCh <= '9')
    {
        iNumber = cCh - '0';
    }
    else if ('a' <= cCh && cCh <= 'f')
    {
        iNumber = cCh - 'a' + 10;
    }
    else if ('A' <= cCh && cCh <= 'F')
    {
        iNumber = cCh - 'A' + 10;
    }
    else
    {
        iNumber = -1;
    }

    return iNumber;
}
