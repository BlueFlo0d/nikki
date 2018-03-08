#ifndef QTDIARY_NODE_H
#define QTDIARY_NODE_H
#include <nettle/ecdsa.h>
#include <nettle/sha3.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define QTDIARY_FRS_DATAREADED 0x1
#define QTDIARY_FRS_KEYCHAIN_VERIFIED 0x2
#define QTDIARY_FRS_KEYCHAIN_CORRUPTED 0x4
#define QTDIARY_FRS_DATAWRITTEN 0x8
#define QTDIARY_FRS_DATABUFFERED 0x2000
#define QTDIARY_FRS_KEYROOT_VERIFIED 0x40
#define QTDIARY_FRS_KEYROOT_CORRUPTED 0x80
#define QTDIARY_FRS_NODE_HASH_INITIALIZED 0x4000
#define QTDIARY_FRS_CONTENT_INITIALIZED 0x100
#define QTDIARY_FRS_PRI_KEY_INITIALIZED 0x200
#define QTDIARY_FRS_SNODE_HASH_INITIALIZED 0x400
#define QTDIARY_FRS_SPNODE_HASH_INITIALIZED 0x800
#define QTDIARY_FRS_PUB_KEY_INITIALIZED 0x8000
#define QTDIARY_FRS_NODE_HASH_UPDATED 0x1000

#define QTDIARY_FRF_ROOT 0x1
#define QTDIARY_FRT_HEAD 0x2
typedef struct dsa_signature dsign;
typedef struct ecc_point ecc_pub;
typedef struct ecc_scalar ecc_pri;
typedef struct _qtdiary_node{
        dsign signed_node_hash;
        dsign signed_prenode_hash;
        ecc_pub pub_key;
        ecc_pri pri_key;
        uint8_t *content;
        size_t content_len;
        uint8_t *node_hash; //temp
        size_t node_hash_len; //temp
        int flag;
        int state;//temp
} qtdiary_node;
static inline qtdiary_node *qtdiary_node_new(int flag){
        qtdiary_node *ret  = (qtdiary_node *)malloc(sizeof(qtdiary_node));
        ret->state = 0;
        ret->flag = flag;
        return ret;
}
static inline void qtdiary_node_free(qtdiary_node *pfr){
        if(pfr->state&QTDIARY_FRS_CONTENT_INITIALIZED){
                free(pfr->content);
        }
        if(pfr->state&QTDIARY_FRS_NODE_HASH_INITIALIZED){
                free(pfr->node_hash);
        }
        if(pfr->state&QTDIARY_FRS_PRI_KEY_INITIALIZED){
                nettle_ecc_scalar_clear(&(pfr->pri_key));
        }
        if(pfr->state&QTDIARY_FRS_PUB_KEY_INITIALIZED){
                nettle_ecc_point_clear(&(pfr->pub_key));
        }
        if(pfr->state&QTDIARY_FRS_SNODE_HASH_INITIALIZED){
                nettle_dsa_signature_clear(&(pfr->signed_node_hash));
        }
        if(pfr->state&QTDIARY_FRS_SPNODE_HASH_INITIALIZED){
                nettle_dsa_signature_clear(&(pfr->signed_prenode_hash));
        }
        free(pfr);
}
#ifdef __cplusplus
extern "C"{
        #endif
        qtdiary_node *qtdiary_node_from_buf(void *buf,size_t len,int flag,void *pre);

        off_t qtdiary_node_write(qtdiary_node *pfr,FILE* fp);
        off_t qtdiary_node_read(qtdiary_node *pfr,FILE* fp);
        void qtdiary_node_log(qtdiary_node *pfr,FILE* fp);
        int qtdiary_node_verify(qtdiary_node *pfr,qtdiary_node *pre);
        #ifdef __cplusplus
}
#endif
static inline void qtdiary_node_print(qtdiary_node *pfr){
        qtdiary_node_log(pfr, stdout);
}
#endif
