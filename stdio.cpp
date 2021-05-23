#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

using namespace std;

char decimal[100];

int recursive_itoa(int arg)
{
    int div = arg / 10;
    int mod = arg % 10;
    int index = 0;
    if (div > 0)
    {
        index = recursive_itoa(div);
    }
    decimal[index] = mod + '0';
    return ++index;
}

char *itoa(const int arg)
{
    bzero(decimal, 100);
    int order = recursive_itoa(arg);
    char *new_decimal = new char[order + 1];
    bcopy(decimal, new_decimal, order + 1);
    return new_decimal;
}

int printf(const void *format, ...)
{
    va_list list;
    va_start(list, format);

    char *msg = (char *)format;
    char buf[1024];
    int nWritten = 0;

    int i = 0, j = 0, k = 0;
    while (msg[i] != '\0')
    {
        if (msg[i] == '%' && msg[i + 1] == 'd')
        {
            buf[j] = '\0';
            nWritten += write(1, buf, j);
            j = 0;
            i += 2;

            int int_val = va_arg(list, int);
            char *dec = itoa(abs(int_val));
            if (int_val < 0)
            {
                nWritten += write(1, "-", 1);
            }
            nWritten += write(1, dec, strlen(dec));
            delete dec;
        }
        else
        {
            buf[j++] = msg[i++];
        }
    }
    if (j > 0)
    {
        nWritten += write(1, buf, j);
    }
    va_end(list);
    return nWritten;
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
    if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF)
    {
        return -1;
    }
    stream->mode = mode;
    stream->pos = 0;
    if (stream->buffer != (char *)0 && stream->bufown == true)
    {
        delete stream->buffer;
    }

    switch (mode)
    {
    case _IONBF:
        stream->buffer = (char *)0;
        stream->size = 0;
        stream->bufown = false;
        break;
    case _IOLBF:
    case _IOFBF:
        if (buf != (char *)0)
        {
            stream->buffer = buf;
            stream->size = size;
            stream->bufown = false;
        }
        else
        {
            stream->buffer = new char[BUFSIZ];
            stream->size = BUFSIZ;
            stream->bufown = true;
        }
        break;
    }
    return 0;
}

void setbuf(FILE *stream, char *buf)
{
    setvbuf(stream, buf, (buf != (char *)0) ? _IOFBF : _IONBF, BUFSIZ);
}

FILE *fopen(const char *path, const char *mode)
{
    FILE *stream = new FILE();
    setvbuf(stream, (char *)0, _IOFBF, BUFSIZ);

    // fopen( ) mode
    // r or rb = O_RDONLY
    // w or wb = O_WRONLY | O_CREAT | O_TRUNC
    // a or ab = O_WRONLY | O_CREAT | O_APPEND
    // r+ or rb+ or r+b = O_RDWR
    // w+ or wb+ or w+b = O_RDWR | O_CREAT | O_TRUNC
    // a+ or ab+ or a+b = O_RDWR | O_CREAT | O_APPEND

    switch (mode[0])
    {
    case 'r':
        if (mode[1] == '\0') // r
        {
            stream->flag = O_RDONLY;
        }
        else if (mode[1] == 'b')
        {
            if (mode[2] == '\0') // rb
            {
                stream->flag = O_RDONLY;
            }
            else if (mode[2] == '+') // rb+
            {
                stream->flag = O_RDWR;
            }
        }
        else if (mode[1] == '+') // r+  r+b
        {
            stream->flag = O_RDWR;
        }
        break;
    case 'w':
        if (mode[1] == '\0') // w
        {
            stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
        }
        else if (mode[1] == 'b')
        {
            if (mode[2] == '\0') // wb
            {
                stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
            }
            else if (mode[2] == '+') // wb+
            {
                stream->flag = O_RDWR | O_CREAT | O_TRUNC;
            }
        }
        else if (mode[1] == '+') // w+  w+b
        {
            stream->flag = O_RDWR | O_CREAT | O_TRUNC;
        }
        break;
    case 'a':
        if (mode[1] == '\0') // a
        {
            stream->flag = O_WRONLY | O_CREAT | O_APPEND;
        }
        else if (mode[1] == 'b')
        {
            if (mode[2] == '\0') // ab
            {
                stream->flag = O_WRONLY | O_CREAT | O_APPEND;
            }
            else if (mode[2] == '+') // ab+
            {
                stream->flag = O_RDWR | O_CREAT | O_APPEND;
            }
        }
        else if (mode[1] == '+') // a+  a+b
        {
            stream->flag = O_RDWR | O_CREAT | O_APPEND;
        }
        break;
    }

    mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    if ((stream->fd = open(path, stream->flag, open_mode)) == -1)
    {
        delete stream;
        printf("fopen failed\n");
        stream = NULL;
    }

    return stream;
}

int fpurge(FILE *stream)
{
    memset(stream->buffer, '\0', stream->size);
    stream->pos = 0;
    return 0;
}

int fflush(FILE *stream)
{
    if (stream->flag == (O_RDONLY))
    {
        //printf("FILE CAN NOT WRITE\n");
        return -1;
    }

    if (stream->eof)
    {
        return EOF;
    }

    write(stream->fd, stream->buffer, stream->pos);
    fpurge(stream);

    return 0;
}

void clear(void *ptr, int size)
{

    memset(ptr, '\0', size);
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (stream->flag == (O_WRONLY | O_CREAT | O_TRUNC) || stream->flag == (O_WRONLY | O_CREAT | O_APPEND))
    {
        printf("FILE CAN NOT READ\n");
        return -1;
    }

    //clear(ptr, nmemb);
    if (stream->eof)
    {
        return -1;
    }

    // number of byetes request by the user
    int bytesRequest = size * nmemb;

    // if buffer is empty of we have reached the end of the buffer
    if (*stream->buffer == '\0')
    {
        // reset the pos
        stream->pos = 0;

        // read to the buffer and reaturn the actual amount of bytes read //8k
        fpurge(stream);
        stream->actual_size = read(stream->fd, stream->buffer, stream->size);
        //printf("STREAM ACTUAL SIZE: %d", stream->actual_size);

        // copy the bytes over to the driver file
        char *buf = (char *)ptr;
        memset(buf, '\0', bytesRequest);

        // if buffer is really small compared to request
        if (bytesRequest >= stream->actual_size)
        {
            memcpy(buf, &stream->buffer[stream->pos], stream->actual_size);
            stream->pos = stream->pos + stream->actual_size;
        }
        else
        {
            memcpy(buf, &stream->buffer[stream->pos], bytesRequest);
            stream->pos = stream->pos + bytesRequest;
        }
    }
    else if ((stream->pos != 0) && (stream->pos <= (stream->actual_size - bytesRequest)))
    {

        // copy the bytes over to the driver file
        char *buf = (char *)ptr;
        memset(buf, '\0', bytesRequest);
        memcpy(buf, &stream->buffer[stream->pos], bytesRequest);
        stream->pos = stream->pos + bytesRequest;
    }
    else if (stream->pos > (stream->actual_size - bytesRequest) && stream->pos < stream->actual_size)
    {

        char *buf = (char *)ptr;
        memset(buf, '\0', bytesRequest);

        memcpy(buf, &stream->buffer[stream->pos], (stream->actual_size - stream->pos));
        //move the postion on of the file
        stream->pos = stream->pos + (stream->actual_size - stream->pos);

        //flag = true;
    }
    else if (stream->pos >= stream->actual_size)
    {

        // reset the pos
        stream->pos = 0;

        // read to the buffer and reaturn the actual amount of bytes read
        //fpurge(stream);
        stream->actual_size = read(stream->fd, stream->buffer, stream->size);

        // copy the bytes over to the driver file
        char *buf = (char *)ptr;
        memset(buf, '\0', bytesRequest);

        // if buffer is really small compared to request
        if (bytesRequest >= stream->actual_size)
        {
            memcpy(buf, &stream->buffer[stream->pos], stream->actual_size);
            stream->pos = stream->pos + stream->actual_size;
        }
        else
        {
            memcpy(buf, &stream->buffer[stream->pos], bytesRequest);
            stream->pos = stream->pos + bytesRequest;
        }
    }

    // end of file if everything is not read
    if (stream->actual_size == 0)
    {
        stream->eof = true;
        return -1;
    }

    if (stream->actual_size == stream->pos)
    {

        return stream->actual_size % bytesRequest;
    }

    return bytesRequest / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (stream->flag == (O_RDONLY))
    {
        printf("FILE CAN NOT WRITE\n");
        return -1;
    }

    // number of byetes request by the user
    int bytesToWrite = size * nmemb;

    // if buffer is empty copy over the the buffer passed in from position 0
    if (*stream->buffer == '\0')
    {

        char *buf = (char *)ptr;
        stream->pos = 0;
        memcpy(&stream->buffer[stream->pos], buf, bytesToWrite);
        stream->pos = stream->pos + bytesToWrite;
    }
    else if ((stream->pos != 0) && (stream->pos <= (stream->size - bytesToWrite)))
    {
        char *buf = (char *)ptr;
        memcpy(&stream->buffer[stream->pos], buf, bytesToWrite);
        stream->pos = stream->pos + bytesToWrite;

        if (stream->pos >= stream->size)
        {
            fflush(stream);
        }
    }
    else if (stream->pos > (stream->size - bytesToWrite) && stream->pos < stream->size)
    {
        fflush(stream);
        char *buf = (char *)ptr;
        write(stream->fd, buf, bytesToWrite);
        stream->pos = stream->pos + bytesToWrite;
    }
    else if (stream->pos >= stream->size)
    {
        fflush(stream);
        stream->pos = 0;
        char *buf = (char *)ptr;
        memcpy(&stream->buffer[stream->pos], buf, bytesToWrite);
        stream->pos = stream->pos + bytesToWrite;
    }

    return 0;
}

int fgetc(FILE *stream)
{
    if (stream->flag == (O_WRONLY | O_CREAT | O_TRUNC) || stream->flag == (O_WRONLY | O_CREAT | O_APPEND))
    {
        printf("FILE CAN NOT READ\n");
        return -1;
    }

    if (stream->eof)
    {
        return EOF;
    }

    char buf[1];

    int bytesRead = fread(buf, 1, 1, stream);

    if (bytesRead >= 0)
    {
        char c = buf[0];
        return c;
    }

    return EOF;
}

int fputc(int c, FILE *stream)
{
    if (stream->flag == (O_RDONLY))
    {
        printf("FILE CAN NOT WRITE\n");
        return -1;
    }

    char buf[1];
    buf[0] = c;
    //size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
    fwrite(buf, 1, 1, stream);

    return 0;
}

char *fgets(char *str, int size, FILE *stream)
{
    if (stream->flag == (O_WRONLY | O_CREAT | O_TRUNC) || stream->flag == (O_WRONLY | O_CREAT | O_APPEND))
    {
        printf("FILE CAN NOT READ\n");
        return NULL;
    }

    int bytesRead = fread(str, 1, size, stream);

    char x = 'A';
    char *ptr = &x;

    str[bytesRead] = '\0';

    if (bytesRead > 0)
    {
        return ptr;
    }

    return NULL;
}

int fputs(const char *str, FILE *stream)
{
    if (stream->flag == (O_RDONLY))
    {
        printf("FILE CAN NOT WRITE\n");
        return -1;
    }

    //size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)

    fwrite(str, 1, strlen(str), stream);
    //fflush(stream);

    return 0;
}

int feof(FILE *stream)
{
    return stream->eof == true;
}

int fseek(FILE *stream, long offset, int whence)
{
    // Sets postion to the begging of buffer
    if (whence == SEEK_SET)
    {
        stream->pos = 0;
        return 0;
    }

    // Sets postion to the given location
    if (whence == SEEK_CUR)
    {
        stream->pos = offset;
        return 0;
    }

    // Sets position to the end of the buffer
    if (whence == SEEK_END)
    {
        stream->pos = stream->actual_size;
        return 0;
    }

    return -1;
}

int fclose(FILE *stream)
{
    if (stream->flag != (O_RDONLY))
    {
        fflush(stream);
    }

    close(stream->fd);
    return 0;
}
