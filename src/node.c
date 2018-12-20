#include "node.h"
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <nettle/yarrow.h>
#include <nettle/ecc-curve.h>
#include <string.h>
#include <stdlib.h>
#include "node_init.h"
#include <time.h>
#define INIT_RAN 0x1

int initializing_flags = 0;
struct yarrow256_ctx _ran_ctx;
int _ran_fd;
uint8_t _ran_buf[YARROW256_SEED_FILE_SIZE];
struct yarrow_source _ran_src[2];
static inline struct yarrow256_ctx *ran_ctx(){
        if(initializing_flags&INIT_RAN) {
                clock_t tmp = clock();
                nettle_yarrow256_update(&_ran_ctx, 1, 0, sizeof(clock_t),&tmp);
                nettle_yarrow256_update(&_ran_ctx, 0, YARROW256_SEED_FILE_SIZE, YARROW256_SEED_FILE_SIZE, _ran_buf);
                return &_ran_ctx;
        }
        else {
                _ran_fd = open("/dev/random", O_RDONLY);
                read(_ran_fd, _ran_buf, YARROW256_SEED_FILE_SIZE);
                nettle_yarrow256_init(&_ran_ctx,2,_ran_src);
                nettle_yarrow256_seed(&_ran_ctx, YARROW256_SEED_FILE_SIZE, _ran_buf);
                initializing_flags^=INIT_RAN;
                return &_ran_ctx;
        }
}
static inline void calc_node_hash(qtdiary_node *pfr){
        if(pfr->state&QTDIARY_FRS_NODE_HASH_UPDATED){
                return;
        }
        pfr->state^=QTDIARY_FRS_NODE_HASH_UPDATED;
        if(!(pfr->state&QTDIARY_FRS_NODE_HASH_INITIALIZED)){
                pfr->node_hash_len = SHA3_256_DIGEST_SIZE;
                pfr->node_hash = (uint8_t *)malloc(pfr->node_hash_len);
                pfr->state^=QTDIARY_FRS_NODE_HASH_INITIALIZED;
        }
        struct sha3_256_ctx node_sha_ctx;
        nettle_sha3_256_init(&node_sha_ctx);
        nettle_sha3_256_update(&node_sha_ctx,pfr->content_len,pfr->content);
        uint8_t *node_hash_buf;
        size_t count;
        mpz_t x,y,z;
        mpz_init(x);
        mpz_init(y);
        nettle_ecc_point_get(&(pfr->pub_key),x,y);
        node_hash_buf = mpz_export(NULL,&count,1,sizeof(uint8_t),0,0,x);
        nettle_sha3_256_update(&node_sha_ctx,count,node_hash_buf);
        free(node_hash_buf);
        node_hash_buf = mpz_export(NULL,&count,1,sizeof(uint8_t),0,0,y);
        nettle_sha3_256_update(&node_sha_ctx,count,node_hash_buf);
        free(node_hash_buf);
        if(!(pfr->flag&QTDIARY_FRF_ROOT)){
                node_hash_buf = mpz_export(NULL,&count,1,sizeof(uint8_t),0,0,pfr->signed_prenode_hash.r);
                nettle_sha3_256_update(&node_sha_ctx,count,node_hash_buf);
                free(node_hash_buf);
                node_hash_buf = mpz_export(NULL,&count,1,sizeof(uint8_t),0,0,pfr->signed_prenode_hash.s);
                nettle_sha3_256_update(&node_sha_ctx,count,node_hash_buf);
                free(node_hash_buf);
        }
        nettle_sha3_256_digest(&node_sha_ctx,pfr->node_hash_len,pfr->node_hash);
        mpz_clear(x);
        mpz_clear(y);
}

qtdiary_node *qtdiary_node_from_buf(void *buf,size_t len,int flag,void *pre){
        qtdiary_node *pfr = (qtdiary_node *)malloc(sizeof(qtdiary_node));
        pfr->content_len = len;
        pfr->content = (uint8_t *)malloc(len);
        memcpy(pfr->content, buf, len);
        pfr->flag = flag;
        pfr->state = QTDIARY_FRS_CONTENT_INITIALIZED|QTDIARY_FRS_PRI_KEY_INITIALIZED|QTDIARY_FRS_PUB_KEY_INITIALIZED;
        if(!(pfr->flag&QTDIARY_FRF_ROOT)){
                pfr->state|=QTDIARY_FRS_SNODE_HASH_INITIALIZED|QTDIARY_FRS_SPNODE_HASH_INITIALIZED;
        }
        nettle_ecc_scalar_init(&(pfr->pri_key),&nettle_secp_256r1);
        nettle_ecc_point_init(&(pfr->pub_key),&nettle_secp_256r1);
        nettle_ecdsa_generate_keypair(&(pfr->pub_key),&(pfr->pri_key),ran_ctx(),nettle_yarrow256_random);
        if(!(pfr->flag&QTDIARY_FRF_ROOT)){
                qtdiary_node *ppre = (qtdiary_node *)pre;
                calc_node_hash(ppre);
                nettle_dsa_signature_init(&(pfr->signed_prenode_hash));
                nettle_ecdsa_sign(&(pfr->pri_key),ran_ctx(),nettle_yarrow256_random,ppre->node_hash_len,ppre->node_hash,&(pfr->signed_prenode_hash));
        }
        calc_node_hash(pfr);
        if(!(pfr->flag&QTDIARY_FRF_ROOT)){
                qtdiary_node *ppre = (qtdiary_node *)pre;
                nettle_dsa_signature_init(&(pfr->signed_node_hash));
                nettle_ecdsa_sign(&(ppre->pri_key),ran_ctx(),nettle_yarrow256_random,pfr->node_hash_len,pfr->node_hash,&(pfr->signed_node_hash));
        }
        return pfr;
}
int qtdiary_node_verify(qtdiary_node *pfr,qtdiary_node *ppre){
        calc_node_hash(ppre);
        calc_node_hash(pfr);
        if(!nettle_ecdsa_verify(&(ppre->pub_key),pfr->node_hash_len,pfr->node_hash,&(pfr->signed_node_hash))){
                goto bad_sig;
        }
        if(!nettle_ecdsa_verify(&(pfr->pub_key),ppre->node_hash_len,ppre->node_hash,&(pfr->signed_prenode_hash))){
                goto bad_sig;
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
off_t qtdiary_node_write(qtdiary_node *pfr,FILE* fp){
        fwrite(&(pfr->content_len), sizeof(size_t), 1, fp);
        fwrite(pfr->content, pfr->content_len, 1, fp);
        mpz_t x,y;
        mpz_init(x);
        mpz_init(y);
        nettle_ecc_point_get(&(pfr->pub_key), x, y);
        mpz_out_raw(fp,x);
        mpz_out_raw(fp,y);
        mpz_clear(x);
        mpz_clear(y);
        if(!(pfr->flag&QTDIARY_FRF_ROOT)){
                mpz_out_raw(fp,pfr->signed_node_hash.r);
                mpz_out_raw(fp,pfr->signed_node_hash.s);
                mpz_out_raw(fp,pfr->signed_prenode_hash.r);
                mpz_out_raw(fp,pfr->signed_prenode_hash.s);
        }
        return ftell(fp);
}
off_t qtdiary_node_read(qtdiary_node *pfr,FILE* fp){
        fread(&(pfr->content_len), sizeof(size_t), 1, fp);
        pfr->state&=(-1)^QTDIARY_FRS_NODE_HASH_UPDATED;
        if(pfr->state&QTDIARY_FRS_CONTENT_INITIALIZED){
                pfr->content = (uint8_t *)realloc(pfr->content, pfr->content_len);
        }
        else{
                pfr->content = (uint8_t *)malloc(pfr->content_len);
                pfr->state^=QTDIARY_FRS_CONTENT_INITIALIZED;
        }
        fread(pfr->content, pfr->content_len, 1, fp);
        mpz_t x,y;
        mpz_init(x);
        mpz_init(y);
        mpz_inp_raw(x,fp);
        mpz_inp_raw(y,fp);
        init_pub(pfr);
        nettle_ecc_point_set(&(pfr->pub_key), x, y);
        mpz_clear(x);
        mpz_clear(y);
        if(!(pfr->flag&QTDIARY_FRF_ROOT)){
                init_sn(pfr);
                init_spn(pfr);
                mpz_inp_raw(pfr->signed_node_hash.r,fp);
                mpz_inp_raw(pfr->signed_node_hash.s,fp);
                mpz_inp_raw(pfr->signed_prenode_hash.r,fp);
                mpz_inp_raw(pfr->signed_prenode_hash.s,fp);
        }
        return ftell(fp);
}
void qtdiary_node_logop(qtdiary_node *pfr,size_t i,void *arg){
        FILE *fp = (FILE *)arg;
        qtdiary_node_log(pfr,fp);
}
void qtdiary_node_log(qtdiary_node *pfr,FILE* fp){
        mpz_t x,y;
        mpz_init(x);
        mpz_init(y);
        nettle_ecc_point_get(&(pfr->pub_key), x, y);
        fputs("\033[;34m",fp);
        gmp_fprintf(fp,"Public Key X: %Zd\n",x);
        gmp_fprintf(fp,"Public Key Y: %Zd\n",y);
        mpz_clear(x);
        mpz_clear(y);
        if(!(pfr->flag&QTDIARY_FRF_ROOT)){
                gmp_fprintf(fp,"Node hash signature R: %Zd\n",pfr->signed_node_hash.r);
                gmp_fprintf(fp,"Node hash signature S: %Zd\n",pfr->signed_node_hash.s);
                gmp_fprintf(fp,"Pre-node hash signature R: %Zd\n",pfr->signed_prenode_hash.r);
                gmp_fprintf(fp,"Pre-node hash signature S: %Zd\n",pfr->signed_prenode_hash.s);
        }
        fputs("[Content]\n", fp);
        fputs("\033[0m",fp);
        fwrite(pfr->content, pfr->content_len, 1, fp);
        fputs("\n\033[;34m[End content]\033[0m\n", fp);
}
