#include "format.h"
#include <stdlib.h>
#include <assert.h>
const char hmac_key[] = "Kyouko!";
#define AE assert(err==0)
static inline void calc_hmac(char **ppbuf,size_t *plen,const void *key,size_t keylen){
        gcry_mac_hd_t mac_hd;
        gcry_error_t err;
        err = gcry_mac_open(&mac_hd, GCRY_MAC_HMAC_SHA3_512, 0, NULL);
        AE;
        err = gcry_mac_setkey(mac_hd, key, keylen);
        AE;
        err = gcry_mac_write(mac_hd, *ppbuf, *plen);
        AE;
        err = gcry_mac_read(mac_hd, *ppbuf, plen);
        AE;
        *ppbuf = realloc(*ppbuf, *plen);
        gcry_mac_close(mac_hd);
}
qtdiary_file_record *qtdiary_file_record_from_buf(void *buf,size_t len,int flag,void *pre,int fd){
        gcry_mac_hd_t mac_hd;
        gcry_error_t err;
        qtdiary_file_record *pfr = (qtdiary_file_record *)malloc(sizeof(qtdiary_file_record));
        pfr->hmac_len = 64;
        pfr->hmac = malloc(64);
        calc_hmac(&(pfr->hmac), &(pfr->hmac_len), hmac_key, 8);
        gcry_sexp_t keyparms;
        gcry_sexp_t keypair;
        err = gcry_sexp_build(&keyparms, NULL, "(genkey (rsa (nbits 4:2048)))");
        AE;
        gcry_pk_genkey(&keypair, keyparms);
        pfr->pub_key = gcry_sexp_find_token(keypair, "public-key", 0);
        pfr->pri_key = gcry_sexp_find_token(keypair, "private-key", 0);
        gcry_sexp_release(keyparms);
        gcry_sexp_release(keypair);
        if(flag&QTDIARY_FRF_ROOT){
                char *title = (char *)pre;
                gcry_mpi_t title_mpi;
                gcry_sexp_t title_sexp;
                gcry_sexp_t stitle_sexp;
                gcry_mpi_scan(&title_mpi, GCRYMPI_FMT_USG, title, strlen(title), NULL);
                gcry_sexp_build(&title_sexp, NULL, "(data (flags raw) (value %m))", title_mpi);
                gcry_pk_sign(&stitle_sexp,title_sexp,pfr->pri_key);
                pfr->signed_pub_len = gcry_sexp_sprint(stitle_sexp, GCRYSEXP_FMT_CANON, NULL, 0);
                pfr->signed_pub = malloc(pfr->signed_pub_len);
                gcry_sexp_sprint(stitle_sexp, GCRYSEXP_FMT_CANON, pfr->signed_pub, pfr->signed_pub_len);
                gcry_mpi_release(title_mpi);
                gcry_sexp_release(title_sexp);
                gcry_sexp_release(stitle_sexp);
        }
        else{
                qtdiary_file_record *ppre = (qtdiary_file_record *)pre;
                gcry_mpi_t pub_mpi;
                gcry_sexp_t pub_sexp;
                gcry_sexp_t spub_sexp;
                size_t publen = gcry_sexp_sprint(pfr->pub_key, GCRYSEXP_FMT_CANON, NULL, 0);
                char *pubbuf = (char *)malloc(publen);
                size_t plen = gcry_sexp_sprint(pfr->pub_key, GCRYSEXP_FMT_CANON, pubbuf, publen);
                calc_hmac(&pubbuf, &publen, hmac_key, 8);
                err = gcry_mpi_scan(&pub_mpi, GCRYMPI_FMT_USG, pubbuf, publen, NULL);
                err = gcry_sexp_build(&pub_sexp, NULL, "(data (flags raw) (value %m))", pub_mpi);
                err = gcry_pk_sign(&spub_sexp, pub_sexp, ppre->pri_key);
                assert(err==0);
                pfr->signed_pub_len = gcry_sexp_sprint(spub_sexp, GCRYSEXP_FMT_CANON, NULL, 0);
                pfr->signed_pub = malloc(pfr->signed_pub_len);
                gcry_sexp_sprint(spub_sexp, GCRYSEXP_FMT_CANON, pfr->signed_pub, pfr->signed_pub_len);
                gcry_mpi_release(pub_mpi);
                gcry_sexp_release(pub_sexp);
                gcry_sexp_release(spub_sexp);
                free(pubbuf);

                gcry_mpi_t hmac_mpi;
                gcry_sexp_t hmac_sexp;
                err = gcry_mpi_scan(&hmac_mpi, GCRYMPI_FMT_USG, pfr->hmac, pfr->hmac_len, NULL);
                AE;
                err = gcry_sexp_build(&hmac_sexp, NULL, "(data (flags raw) (value %m))", hmac_mpi);
                AE;
                gcry_sexp_t signed_hmac_sexp;
                err = gcry_pk_sign(&signed_hmac_sexp, hmac_sexp, ppre->pri_key);
                AE;
                pfr->signed_hmac_len = gcry_sexp_sprint(signed_hmac_sexp, GCRYSEXP_FMT_CANON, NULL, 0);
                pfr->signed_hmac = malloc(pfr->signed_hmac_len);
                gcry_sexp_sprint(signed_hmac_sexp, GCRYSEXP_FMT_CANON, pfr->signed_hmac, pfr->signed_hmac_len);
                gcry_mpi_release(hmac_mpi);
                gcry_sexp_release(hmac_sexp);
                gcry_sexp_release(signed_hmac_sexp);
        }
        return pfr;
}
int qtdiary_file_record_verify_keychain(qtdiary_file_record *pfr,qtdiary_file_record *ppre){
        gcry_error_t err;
        gcry_sexp_t spub_sexp;
        gcry_sexp_t pub_sexp;
        gcry_mpi_t pub_mpi;
        size_t publen = gcry_sexp_sprint(pfr->pub_key, GCRYSEXP_FMT_CANON, NULL, 0);
        char *pubbuf = (char *)malloc(publen);
        err = gcry_sexp_sprint(pfr->pub_key, GCRYSEXP_FMT_CANON, pubbuf, publen);
        calc_hmac(&pubbuf, &publen, hmac_key, 8);
        err = gcry_mpi_scan(&pub_mpi, GCRYMPI_FMT_USG, pubbuf, publen, NULL);
        err = gcry_sexp_build(&pub_sexp, NULL, "(data (flags raw) (value %m))", pub_mpi);
        err = gcry_sexp_sscan(&spub_sexp, NULL, pfr->signed_pub, pfr->signed_pub_len);
        err = gcry_pk_verify(spub_sexp, pub_sexp, ppre->pub_key);
        gcry_sexp_release(pub_sexp);
        gcry_mpi_release(pub_mpi);
        gcry_sexp_release(spub_sexp);
        free(pubbuf);
        if(gcry_err_code(err) == GPG_ERR_BAD_SIGNATURE){
                goto bad_sig;
        }
        else if(err){
                goto error;
        }
        gcry_sexp_t shmac_sexp;
        gcry_sexp_t hmac_sexp;
        gcry_mpi_t hmac_mpi;
        gcry_sexp_sscan(&shmac_sexp, NULL, pfr->signed_hmac, pfr->signed_hmac_len);
        gcry_mpi_scan(&hmac_mpi, GCRYMPI_FMT_USG, pfr->hmac, pfr->hmac_len, NULL);
        gcry_sexp_build(&hmac_sexp, NULL, "(data (flags raw) (value %m))", hmac_mpi);
        err = gcry_pk_verify(shmac_sexp, hmac_sexp, ppre->pub_key);
        gcry_sexp_release(shmac_sexp);
        gcry_sexp_release(hmac_sexp);
        gcry_mpi_release(hmac_mpi);
        if(gcry_err_code(err) == GPG_ERR_BAD_SIGNATURE){
                goto bad_sig;
        }
        else if(err){
                goto error;
        }
good_sig:
        pfr->flag |= QTDIARY_FRS_KEYCHAIN_VERIFIED;
        pfr->flag &= (-1)^QTDIARY_FRS_KEYCHAIN_CORRUPTED;
        return 0;
bad_sig:
        pfr->flag |= QTDIARY_FRS_KEYCHAIN_CORRUPTED;
        pfr->flag &= (-1)^QTDIARY_FRS_KEYCHAIN_VERIFIED;
        return QTDIARY_FRS_KEYCHAIN_CORRUPTED;
error:
        return -1;
}
