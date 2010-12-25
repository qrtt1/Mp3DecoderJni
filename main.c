#include <mpg123.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


struct Mp3DecodeContext {
    mpg123_handle* handle;
    int channels;
    int encoding;
    long rate;

    unsigned char* buffer;
    size_t buffer_size;
    size_t decodedBytes;
};

void closeHandle(struct Mp3DecodeContext *ctx)
{
    if(ctx != NULL && ctx->handle != NULL)
    {
        ctx->handle = NULL;
        mpg123_close(ctx->handle);
        mpg123_delete(ctx->handle);
        mpg123_exit();
    }
}

void close(struct Mp3DecodeContext* ctx)
{
    if(ctx != NULL)
    {
        closeHandle(ctx);
        free(ctx);
    }
}

int nextFrame(struct Mp3DecodeContext* ctx)
{
    int err = mpg123_read(ctx->handle, ctx->buffer, ctx->buffer_size, &ctx->decodedBytes);
    if(err != MPG123_OK)
    {
        closeHandle(ctx);
        ctx->decodedBytes = -1;
        return 0;
    }
    return 1;
}

struct Mp3DecodeContext* open(const char* file)
{
	int  err  = MPG123_OK;
    struct Mp3DecodeContext* ctx = malloc(sizeof(struct Mp3DecodeContext));
    memset(ctx, 0, sizeof(struct Mp3DecodeContext));

    err = mpg123_init();
    if( err != MPG123_OK || (ctx->handle = mpg123_new(NULL, &err)) == NULL
            /* Let mpg123 work with the file, that excludes MPG123_NEED_MORE messages. */
            || mpg123_open(ctx->handle, file) != MPG123_OK
            /* Peek into track and get first output format. */
            || mpg123_getformat(ctx->handle, &ctx->rate, &ctx->channels, &ctx->encoding) != MPG123_OK )
    {
        /*
        fprintf( stderr, "Trouble with mpg123: %s\n",
                ctx->handle==NULL ? mpg123_plain_strerror(err) : mpg123_strerror(ctx->handle) );
                */
        goto FAILURE;
    }

    if(ctx->encoding != MPG123_ENC_SIGNED_16)
    { /* Signed 16 is the default output format anyways; it would actually by only different if we forced it.
         So this check is here just for this explanation. */
        /*
        cleanup(mh);
        fprintf(stderr, "Bad encoding: 0x%x!\n", encoding);
        return -2;
        */
        goto FAILURE;
    }

    mpg123_format_none(ctx->handle);
    mpg123_format(ctx->handle, ctx->rate, ctx->channels, ctx->encoding);

    ctx->buffer_size = mpg123_outblock( ctx->handle );
    ctx->buffer = malloc( ctx->buffer_size );

    return ctx;

FAILURE:
    closeHandle(ctx);
    free(ctx);
    return NULL;
}



int main(){

    struct Mp3DecodeContext* ctx;
    ctx = open("sample.mp3");

    FILE *fp = fopen("out.raw", "wb");

    if(ctx)
    {
        int cnt = 0;
        while(nextFrame(ctx))
        {
            if(fp)
                fwrite(ctx->buffer, sizeof(char), ctx->decodedBytes, fp);
        }
    }

    if(fp)
        fclose(fp);
    close(ctx);

}
