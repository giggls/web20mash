/* read TLS key/certificate into buffer */
#include <stdio.h>
#include <stdlib.h>
#include "mashctld.h"

static long get_file_size(const char *filename) {
  FILE *fp;

  fp = fopen(filename, "rb");
  if (fp)
    {
      long size;

      if ((0 != fseek(fp, 0, SEEK_END)) || (-1 == (size = ftell (fp))))
        size = 0;

      fclose(fp);

      return size;
    }
  else
    return 0;
}

char * load_pem_into_buf(const char *filename) {
  FILE *fp;
  char *buffer;
  size_t size;

  size = get_file_size(filename);
  if (size == 0)
    die("Invalid pem-file: %s\n",filename);

  fp = fopen(filename, "rb");
  if (!fp)
    die("unable to open pem-file: %s\n",filename);

  buffer = malloc(size);
  if (!buffer)
    {
      fclose(fp);
      die("malloc error\n");
    }

  if (size != fread(buffer, 1, size, fp))
    {
      free(buffer);
      die("unable to read pem-file: %s\n",filename);
    }

  fclose(fp);
  return buffer;
}

