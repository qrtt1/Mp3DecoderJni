#include "qty_player_Mp3Decoder.h"
#include <mpg123.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void JNU_ThrowByName(JNIEnv *env, const char *name, const char *msg)
{
    jclass cls = (*env)->FindClass(env, name);
    /* if cls is NULL, an exception has already been thrown */
    if (cls != NULL) {
        (*env)->ThrowNew(env, cls, msg);
    }
    /* free the local ref */
    (*env)->DeleteLocalRef(env, cls);
}
#if 0
jstring JNU_NewStringNative(JNIEnv *env, const char *str)
{
    jstring result;
    jbyteArray bytes = 0;
    int len;
    if ((*env)->EnsureLocalCapacity(env, 2) < 0) {
        return NULL; /* out of memory error */
    }
    len = strlen(str);
    bytes = (*env)->NewByteArray(env, len);
    if (bytes != NULL) {
        (*env)->SetByteArrayRegion(env, bytes, 0, len,
                (jbyte *)str);

        jclass strCls = (*env)->FindClass(env, "java/lang/String");

        /* 取得 String(byte[]) 建構子的 jmethodID */
        /* 建構子的方法名稱為 <init> 這是之前講解方法時未提及的 */
        jmethodID strInitMid = (*env)->GetMethodID(env, strCls, "<init>", "([B)V");

        result = (*env)->NewObject(env, strCls, strInitMid, bytes);
        (*env)->DeleteLocalRef(env, bytes);
        return result;
    } /* else fall through */
    return NULL;
}
#endif

char *JNU_GetStringNativeChars(JNIEnv *env, jstring jstr)
{
    jbyteArray bytes = 0;
    jthrowable exc;
    char *result = 0;
    if ((*env)->EnsureLocalCapacity(env, 2) < 0) {
        return 0; /* out of memory error */
    }

    jclass strCls = (*env)->GetObjectClass(env, jstr);
    jmethodID getBytesMid = (*env)->GetMethodID(env, strCls, "getBytes", "()[B");
    bytes = (*env)->CallObjectMethod(env, jstr, getBytesMid);
    exc = (*env)->ExceptionOccurred(env);
    if (!exc) {
        jint len = (*env)->GetArrayLength(env, bytes);
        result = (char *)malloc(len + 1);
        if (result == 0) {
            JNU_ThrowByName(env, "java/lang/OutOfMemoryError",
                    0);
            (*env)->DeleteLocalRef(env, bytes);
            return 0;
        }
        (*env)->GetByteArrayRegion(env, bytes, 0, len,
                (jbyte *)result);
        result[len] = 0; /* NULL-terminate */
    } else {
        (*env)->DeleteLocalRef(env, exc);
    }
    (*env)->DeleteLocalRef(env, bytes);
    return result;
}

/*
 * Class:     qty_player_Mp3Decoder
 * Method:    open
 * Signature: (Ljava/io/File;)V
 */
JNIEXPORT void JNICALL Java_qty_player_Mp3Decoder_open
  (JNIEnv *env, jobject self, jobject file)
{
    
    jclass fileClass = (*env)->GetObjectClass(env, file);
    jmethodID toAbsPath = (*env)->GetMethodID(env, fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jstring jpath = (*env)->CallObjectMethod(env, file, toAbsPath);
    int bufSize = 0;
    char *cpath = JNU_GetStringNativeChars(env, jpath);

    /**
     * prepare mpg123 structure.
     **/
    int ret = 0;
    mpg123_handle* handle;
    int channels;
    int encoding;
    long rate;   

    int err = mpg123_init();
    if( err != MPG123_OK )
    {
        fprintf(stderr, "initial mpg123 failure.\n");
        ret = 0;
    }
    else if((handle = mpg123_new(NULL, &err)) == NULL)
    {
        fprintf(stderr, "alloc mpg123 failure.\n");
        ret = 0;
    }
    else if(mpg123_open(handle, cpath) != MPG123_OK)
    {
        fprintf(stderr, "open file[%s] failure.\n", cpath);
        ret = 0;
    }
    else if(mpg123_getformat(handle, &rate, &channels, &encoding) != MPG123_OK)
    {
        fprintf(stderr, "cannot get format.\n");
        ret = 0;
    }
    else
    {
        ret = 1;
        fprintf(stderr, "rate: %ld, channels: %d, encoding: %d\n", rate, channels, encoding);
    }
    free(cpath);
    (*env)->DeleteLocalRef(env, fileClass);

    if(ret == 0)
    {
        mpg123_close(handle);
        mpg123_delete(handle);
        mpg123_exit();
        return ;
    }

    mpg123_format_none(handle);
    mpg123_format(handle, rate, channels, encoding);
    int buffer_size = mpg123_outblock( handle );
    fprintf(stderr, "required buffer size: %d\n", buffer_size);

    /**
     * set information to decoder instance
     **/

    jclass cls = (*env)->GetObjectClass(env, self);
    jfieldID fid;

    /* handle */
    fid = (*env)->GetFieldID(env, cls, "handle", "I");
    (*env)->SetIntField(env, self, fid, (jint) handle);

    /* channels */
    fid = (*env)->GetFieldID(env, cls, "channels", "I");
    (*env)->SetIntField(env, self, fid, (jint) channels);

    /* encoding */
    fid = (*env)->GetFieldID(env, cls, "encoding", "I");
    (*env)->SetIntField(env, self, fid, (jint) encoding);

    /* rate  */
    fid = (*env)->GetFieldID(env, cls, "rate", "J");
    (*env)->SetIntField(env, self, fid, (jlong) rate);

    /* requiredBuffersize */
    fid = (*env)->GetFieldID(env, cls, "requiredBuffersize", "I");
    (*env)->SetIntField(env, self, fid, (jint) buffer_size);

    (*env)->DeleteLocalRef(env, cls);

    return ;
}

/*
 * Class:     qty_player_Mp3Decoder
 * Method:    decode
 * Signature: ([B)I
 */
JNIEXPORT jint JNICALL Java_qty_player_Mp3Decoder_decode
  (JNIEnv *env, jobject self, jbyteArray buf)
{
    jint len = (*env)->GetArrayLength(env, buf);
    jbyte* cbuf = (*env)->GetByteArrayElements(env, buf, NULL);

    jclass cls = (*env)->GetObjectClass(env, self);
    jfieldID fid = (*env)->GetFieldID(env, cls, "handle", "I");
    jint handleId =(*env)->GetIntField(env, self, fid);
    (*env)->DeleteLocalRef(env, cls);

    int decodedBytes = 0;
    int err = mpg123_read((mpg123_handle*) handleId, cbuf, len, &decodedBytes);
    (*env)->ReleaseByteArrayElements(env, buf, cbuf, 0);

    if(err == MPG123_OK)
        return decodedBytes;

    return -1;
}

/*
 * Class:     qty_player_Mp3Decoder
 * Method:    close
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_qty_player_Mp3Decoder_close
  (JNIEnv *env, jobject self)
{
    jclass cls = (*env)->GetObjectClass(env, self);
    jfieldID fid = (*env)->GetFieldID(env, cls, "handle", "I");
    jint handleId =(*env)->GetIntField(env, self, fid);
    (*env)->SetIntField(env, self, fid, 0);
    (*env)->DeleteLocalRef(env, cls);

    if(handleId <= 0)
    {
        fprintf(stderr, "no mpg123 handle found.\n");
        return ;
    }

    mpg123_handle* handle = (mpg123_handle*) handleId;
    mpg123_close(handle);
    mpg123_delete(handle);
    mpg123_exit();
    fprintf(stderr, "destroy mpg123 %p\n", handle);
}


