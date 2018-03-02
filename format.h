#ifndef QTDIARY_FORMAT_H
#define QTDIARY_FORMAT_H
#include <gcrypt.h>
#include <sys/types.h>
#include <unistd.h>
#define QTDIARY_FRS_DATAREADED 0x1
#define QTDIARY_FRS_HMAC_VERIFIED 0x2
#define QTDIARY_FRS_HMAC_CORRUPTED 0x4
#define QTDIARY_FRS_DATAWRITTEN 0x8
#define QTDIARY_FRS_DATABUFFERED 0xF
#define QTDIARY_FRS_KEYCHAIN_VERIFIED 0x10
#define QTDIARY_FRS_KEYCHAIN_CORRUPTED 0x20
#define QTDIARY_FRS_KEYROOT_VERIFIED 0x40
#define QTDIARY_FRS_KEYROOT_CORRUPTED 0x80

#define QTDIARY_FRF_ROOT 0x1
#define QTDIARY_FRT_HEAD 0x2
typedef struct _qtdiary_file_record{
        char *signed_hmac;//sexp
        size_t signed_hmac_len;
        char *signed_pub;//sexp
        size_t signed_pub_len;
        gcry_sexp_t pub_key;
        gcry_sexp_t pri_key;
        char *hmac;//raw
        size_t hmac_len;
        int flag;
        int fd;
        off_t off_start;
        size_t data_len;
        int state;
        void *buf;
} qtdiary_file_record;
#ifdef __cplusplus
extern "C"{
        #endif
        qtdiary_file_record *qtdiary_file_record_from_buf(void *buf,size_t len,int flag,void *pre,int fd);
        int qtdiary_file_record_verify_keychain(qtdiary_file_record *pfr,qtdiary_file_record *pre);
        int qtdiary_file_record_write(qtdiary_file_record *rec);
        #ifdef __cplusplus
}
#endif
#endif
