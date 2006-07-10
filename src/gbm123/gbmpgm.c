/*

gbmpgm.c - Poskanzers PGM format


Supported formats and options:
------------------------------
Greymap : Portable Greyscale-map : .PGM

Reads  8 bpp grey images (ASCII format P2, binary format P5).
Writes 8 bpp grey equivalent of passed in 8 bit colour data (no palette written)
       (ASCII format P1, binary format P4)

Input:
------

Can specify image within PGM file with multiple images (only for P5 type)
  Input option: index=# (default: 0)

Output:
-------

Can specify the colour channel the output grey values are based on
  Output option: r,g,b,k (default: k, combine color channels and write grey equivalent)

Write ASCII format P2 (default is binary P5)
  Output option: ascii

Write additonal comment
  Output option: comment=text


History:
--------
(Heiko Nitzsche)

22-Feb-2006: Move format description strings to gbmdesc.h
11-Jun-2006: Add function to query number of images (type P5 only)
             Add read support for multipage images  (type P5 only)
16-Jun-2006: Add support for reading & writing ASCII format P2
*/

#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"

/* ---------------------------------------- */

#define  GBM_ERR_PGM_BAD_M    ((GBM_ERR) 100)

/* ---------------------------------------- */

static GBMFT pgm_gbmft =
{
   GBM_FMT_DESC_SHORT_PGM,
   GBM_FMT_DESC_LONG_PGM,
   GBM_FMT_DESC_EXT_PGM,
   GBM_FT_R8 |
   GBM_FT_W8
};

typedef struct
{
   unsigned int max_intensity;

   /* This entry will store the options provided during first header read.
    * It will keep the options for the case the header has to be reread.
    */
   char read_options[PRIV_SIZE - sizeof(int)
                               - 8 /* space for structure element padding */ ];

} PGM_PRIV_READ;

/* ---------------------------------------- */

#define  low_byte(w)    ((byte)  ((w)&0x00ff)    )
#define  high_byte(w)   ((byte) (((w)&0xff00)>>8))
#define  make_word(a,b) (((word)a) + (((word)b) << 8))
#define  SW4(a,b,c,d)   ((a)*8+(b)*4+(c)*2+(d))

/* ---------------------------------------- */

static BOOLEAN make_output_palette(const GBMRGB gbmrgb[], byte grey[], const char *opt)
{
  BOOLEAN  k = ( gbm_find_word(opt, "k") != NULL );
  BOOLEAN  r = ( gbm_find_word(opt, "r") != NULL );
  BOOLEAN  g = ( gbm_find_word(opt, "g") != NULL );
  BOOLEAN  b = ( gbm_find_word(opt, "b") != NULL );
  int i;

  switch ( SW4(k,r,g,b) )
  {
    case SW4(0,0,0,0):
      /* Default is the same as "k" */
    case SW4(1,0,0,0):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = (byte) ( ((word) gbmrgb[i].r *  77U +
                            (word) gbmrgb[i].g * 150U +
                            (word) gbmrgb[i].b *  29U) >> 8 );
      }
      return TRUE;

    case SW4(0,1,0,0):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = gbmrgb[i].r;
      }
      return TRUE;

    case SW4(0,0,1,0):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = gbmrgb[i].g;
      }
      return TRUE;

    case SW4(0,0,0,1):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = gbmrgb[i].b;
      }
      return TRUE;
  }
  return FALSE;
}

/* ---------------------------------------- */

static byte read_byte(int fd)
{
  byte b = 0;
  gbm_file_read(fd, (char *) &b, 1);
  return b;
}

/* ---------------------------------------- */

static char read_char(int fd)
{
  char c;
  while ( (c = read_byte(fd)) == '#' )
  {
    /* Discard to end of line */
    while ( (c = read_byte(fd)) != '\n' )
      ;
  }
  return c;
}

static int read_num(int fd)
{
  char c;
  int num;

  while ( isspace(c = read_char(fd)) )
    ;
  num = c - '0';
  while ( isdigit(c = read_char(fd)) )
  {
    num = num * 10 + (c - '0');
  }
  return num;
}

static int read_num_data(int fd)
{
  char c;
  int num;

  do
  {
    c = read_char(fd);
  }
  while (isspace(c) || (c == '\n') || (c == '\r'));

  num = c - '0';
  while ( isdigit(c = read_char(fd)) )
  {
    num = num * 10 + (c - '0');
  }
  return num;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pgm_qft(GBMFT *gbmft)
{
   *gbmft = pgm_gbmft;
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static GBM_ERR read_pgm_header(int fd, int *h1, int *h2, int *w, int *h, int *m, int *data_bytes)
{
   *h1 = read_byte(fd);
   *h2 = read_byte(fd);
   if ( (*h1 != 'P') || ((*h2 != '2') && (*h2 != '5')) )
   {
      return GBM_ERR_BAD_MAGIC;
   }

   *w  = read_num(fd);
   *h  = read_num(fd);
   if ((*w <= 0) || (*h <= 0))
   {
      return GBM_ERR_BAD_SIZE;
   }

   *m  = read_num(fd);
   if (*m <= 1)
   {
      return GBM_ERR_PGM_BAD_M;
   }

   /* we only support 1 byte format (in case there are others) */
   if (*m < 0x100)
   {
      *data_bytes = (*w) * (*h);
   }
   else
   {
      return GBM_ERR_PGM_BAD_M;
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

/* Read number of images in the PGM file. */
GBM_ERR pgm_rimgcnt(const char *fn, int fd, int *pimgcnt)
{
   GBM_ERR rc;
   GBM     gbm;
   int     h1, h2, m, data_bytes;

   fn=fn; /* suppress compiler warning */

   *pimgcnt = 1;

   /* read header info of first bitmap */
   rc = read_pgm_header(fd, &h1, &h2, &gbm.w, &gbm.h, &m, &data_bytes);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }

   /* find last available image index */

   /* we only support multipage images for binary type P5 */
   if (h2 == '5')
   {
      long image_start;

      image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;

      /* index 0 has already been read */

      /* move file pointer to beginning of the next bitmap */
      while (gbm_file_lseek(fd, image_start, GBM_SEEK_SET) >= 0)
      {
         /* read header info of next bitmap */
         rc = read_pgm_header(fd, &h1, &h2, &gbm.w, &gbm.h, &m, &data_bytes);
         if (rc != GBM_ERR_OK)
         {
            break;
         }
         (*pimgcnt)++;

         image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;
      }
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

static GBM_ERR internal_pgm_rhdr(int fd, GBM * gbm, GBM * gbm_src, int * type)
{
   GBM_ERR rc;
   int     h1, h2, m, data_bytes;
   const char *s = NULL;

   PGM_PRIV_READ *pgm_priv = (PGM_PRIV_READ *) gbm->priv;

   /* start at the beginning of the file */
   gbm_file_lseek(fd, 0, GBM_SEEK_SET);

   /* read header info of first bitmap */
   rc = read_pgm_header(fd, &h1, &h2, &gbm->w, &gbm->h, &m, &data_bytes);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }
   *type = h2;

   /* goto requested image index */
   if ((s = gbm_find_word_prefix(pgm_priv->read_options, "index=")) != NULL)
   {
      int  image_index_curr;
      long image_start;
      int  image_index = 0;
      if (sscanf(s + 6, "%d", &image_index) != 1)
      {
         return GBM_ERR_BAD_OPTION;
      }

      /* we only support multipage images for binary type P5 */
      if ((h2 != '5') && (image_index != 0))
      {
         return GBM_ERR_BAD_OPTION;
      }

      image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;

      /* index 0 has already been read */
      image_index_curr = 0;
      while (image_index_curr < image_index)
      {
         /* move file pointer to beginning of the next bitmap */
         if (gbm_file_lseek(fd, image_start, GBM_SEEK_SET) < 0)
         {
            return GBM_ERR_READ;
         }

         /* read header info of next bitmap */
         rc = read_pgm_header(fd, &h1, &h2, &gbm->w, &gbm->h, &m, &data_bytes);
         if (rc != GBM_ERR_OK)
         {
            return rc;
         }

         image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;
         image_index_curr++;
      }
   }

   pgm_priv->max_intensity = (unsigned int) m;

   /* we only support 1 byte format */
   if (pgm_priv->max_intensity < 0x100)
   {
      gbm    ->bpp = 8;
      gbm_src->bpp = 8;
   }
   else
   {
      return GBM_ERR_PGM_BAD_M;
   }

   gbm_src->w = gbm->w;
   gbm_src->h = gbm->h;

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pgm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
   PGM_PRIV_READ *pgm_priv = (PGM_PRIV_READ *) gbm->priv;
   GBM gbm_src;
   int type;

   fn=fn; /* Suppress 'unref arg' compiler warnings */

   /* copy possible options */
   if (strlen(opt) >= sizeof(pgm_priv->read_options))
   {
      return GBM_ERR_BAD_OPTION;
   }
   strcpy(pgm_priv->read_options, opt);

   /* read bitmap info */
   return internal_pgm_rhdr(fd, gbm, &gbm_src, &type);
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pgm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
  PGM_PRIV_READ *pgm_priv = (PGM_PRIV_READ *) gbm->priv;

  int i;
  int type;
  GBM gbm_src;

  GBM_ERR rc = internal_pgm_rhdr(fd, gbm, &gbm_src, &type);
  if (rc != GBM_ERR_OK)
  {
    return rc;
  }

  if (gbm->bpp == 8)
  {
    for ( i = 0; i <= pgm_priv->max_intensity; i++ )
    {
      gbmrgb[i].r =
      gbmrgb[i].g =
      gbmrgb[i].b = (byte) (i * 255U / pgm_priv->max_intensity);
    }
  }

  return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pgm_rdata(int fd, GBM *gbm, byte *data)
{
  int type = 0;
  GBM gbm_src;

  const int stride = ((gbm->w * gbm->bpp + 31)/32) * 4;

  GBM_ERR rc = internal_pgm_rhdr(fd, gbm, &gbm_src, &type);
  if (rc != GBM_ERR_OK)
  {
     return rc;
  }

  if (gbm->bpp != 8)
  {
     return GBM_ERR_READ;
  }

  /* binary type P5 */
  if (type == '5')
  {
    int    i;
    byte * p = data + ((gbm->h - 1) * stride);

    const int line_bytes = gbm_src.w * (gbm_src.bpp / 8);

    for (i = gbm->h - 1; i >= 0; i--)
    {
       if (gbm_file_read(fd, p, line_bytes) != line_bytes)
       {
          return GBM_ERR_READ;
       }
       p -= stride;
    }
  }
  /* ASCII type P2 */
  else if (type == '2')
  {
    int    i, x;
    int    num;
    byte * pNumFill;
    byte * p = data + ((gbm->h - 1) * stride);

    for (i = gbm->h - 1; i >= 0; i--)
    {
      pNumFill = p;
      for (x = 0; x < gbm_src.w; x++)
      {
        num = read_num_data(fd);
        if ((num < 0) || (num > 0xff))
        {
          return GBM_ERR_READ;
        }
        *pNumFill++ = (byte) num;
      }

      p -= stride;
    }
  }
  else
  {
    return GBM_ERR_NOT_SUPP;
  }

  return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static BOOLEAN internal_pgm_w_ascii(int fd, const byte * data, int bytes)
{
  char d[4] = { 0 };
  int  b, c;

  d[3] = ' ';

  /* write the bytes */
  c  = 0;
  for (b = 0; b < bytes; b++)
  {
    if (c > 66)
    {
      c = 0;
      if (gbm_file_write(fd, "\n", 1) != 1)
      {
        return FALSE;
      }
    }

    /* write always 3 digits, also if they are 0 */
    d[0] = '0' +   (*data / 100);
    d[1] = '0' + (((*data % 100) - (*data % 10)) / 10);
    d[2] = '0' +   (*data % 10);
    /* d[3] is already filled in before the loop */

    if (gbm_file_write(fd, d, 4) != 4)
    {
      return FALSE;
    }
    c += 4;
    data++;
  }

  if (gbm_file_write(fd, "\n", 1) != 1)
  {
    return FALSE;
  }

  return TRUE;
}

/* ---------------------------------------- */

static BOOLEAN internal_pgm_write_comment(int fd, const char *options)
{
   const char *s;

   if ((s = gbm_find_word_prefix(options, "comment=")) != NULL)
   {
     int   len = 0;
     char  buf[200+1] = { 0 };

     if (sscanf(s + 8, "%200[^\"]", buf) != 1)
     {
        if (sscanf(s + 8, "%200[^ ]", buf) != 1)
        {
           return FALSE;
        }
     }

     if (gbm_file_write(fd, "# ", 2) != 2)
     {
       return FALSE;
     }

     len = strlen(buf);
     if (gbm_file_write(fd, buf, len) != len)
     {
       return FALSE;
     }

     if (gbm_file_write(fd, "\n", 1) != 1)
     {
       return FALSE;
     }
   }

   return TRUE;
}

/* ---------------------------------------- */

GBM_ERR pgm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt)
{
  char s[100+1];
  int i, j, stride;
  byte grey[0x100];
  const byte *p;
        byte *linebuf;

  const BOOLEAN ascii = ( gbm_find_word(opt, "ascii" ) != NULL );

  fn=fn; opt=opt; /* Suppress 'unref arg' compiler warnings */

  if (gbm->bpp != 8)
  {
    return GBM_ERR_NOT_SUPP;
  }

  if (! make_output_palette(gbmrgb, grey, opt))
  {
    return GBM_ERR_BAD_OPTION;
  }

  if ( (linebuf = malloc((size_t) gbm->w)) == NULL )
  {
    return GBM_ERR_MEM;
  }

  sprintf(s, "P%c\n", (ascii ? '2' : '5'));
  if (gbm_file_write(fd, s, (int) strlen(s)) != (int) strlen(s))
  {
    free(linebuf);
    return GBM_ERR_WRITE;
  }

  /* write optional comment */
  if (! internal_pgm_write_comment(fd, opt))
  {
    free(linebuf);
    return GBM_ERR_WRITE;
  }

  sprintf(s, "%d %d\n255\n", gbm->w, gbm->h);
  if (gbm_file_write(fd, s, (int) strlen(s)) != (int) strlen(s))
  {
    free(linebuf);
    return GBM_ERR_WRITE;
  }

  stride = ((gbm->w + 3) & ~3);
  p = data + ((gbm->h - 1) * stride);
  for ( i = gbm->h - 1; i >= 0; i-- )
  {
    for ( j = 0; j < gbm->w; j++ )
    {
      linebuf[j] = grey[p[j]];
    }

    /* write as ASCII pattern or as binary data */
    if (ascii)
    {
      if (! internal_pgm_w_ascii(fd, linebuf, gbm->w))
      {
        free(linebuf);
        return GBM_ERR_WRITE;
      }
    }
    else
    {
      if (gbm_file_write(fd, linebuf, gbm->w) != gbm->w)
      {
        free(linebuf);
        return GBM_ERR_WRITE;
      }
    }
    p -= stride;
  }

  free(linebuf);

  return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

const char *pgm_err(GBM_ERR rc)
{
  switch ( (int) rc )
  {
    case GBM_ERR_PGM_BAD_M:
      return "bad maximum pixel intensity";
  }
  return NULL;
}

