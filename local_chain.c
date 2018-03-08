#include "local_chain.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "node_init.h"
#include "util.h"
#define FILEOP_CHUNK 8192

void qtdiary_local_chain_rebuild_index(qtdiary_local_chain *plc){
        qdwrns("Trying to rebuild index.");
        ftruncate(fileno(plc->index), sizeof(size_t));
        fseek(plc->index, 0, SEEK_END);
        fseek(plc->nodebase, 0, SEEK_END);
        off_t offset = 0;
        size_t len;
        off_t ftail = ftell(plc->nodebase);
        fseek(plc->nodebase, 0, SEEK_SET);
        while(ftell(plc->nodebase)<ftail){
                fread(&len, sizeof(off_t), 1, plc->nodebase);
                offset+=len;
                plc->height++;
                fwrite(&offset, sizeof(off_t), 1, plc->index);
        }
        fseek(plc->index, 0, SEEK_SET);
        fwrite(&(plc->height), sizeof(size_t), 1, plc->index);
}

void qtdiary_local_chain_open(qtdiary_local_chain *plc,const char *path){
        int dir = open(path, O_DIRECTORY);
        if(dir==-1){
                mkdir(path, S_IRUSR|S_IWUSR|S_IEXEC);
                dir = open(path, O_DIRECTORY);
        }
        int inb = openat(dir, "nodebase", O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
        int ifd = openat(dir, "index", O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
        int ihd = openat(dir, "head", O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
        int iac = openat(dir, "active", O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
        plc->index = fdopen(ifd, "wb+");
        plc->nodebase = fdopen(inb, "wb+");
        plc->head = fdopen(ihd, "wb+");
        plc->active = fdopen(iac, "wb+");
        fseek(plc->index, 0, SEEK_END);
        if(ftell(plc->index)){
                fseek(plc->index, 0, SEEK_SET);
                fread(&(plc->height), sizeof(size_t), 1, plc->index);
        }
        else{
                fseek(plc->nodebase, 0, SEEK_END);
                if(ftell(plc->nodebase)){
                        qtdiary_local_chain_rebuild_index(plc);
                }
                else{
                        plc->height = 0;
                        fwrite(&(plc->height), sizeof(size_t), 1, plc->index);
                }
        }
        if(plc->height){
                fseek(plc->index, -sizeof(off_t), SEEK_END);
                fread(&(plc->chain_tail), sizeof(off_t), 1, plc->index);
        }
        else{
                plc->chain_tail = 0;
        }
}
int qtdiary_local_chain_push(qtdiary_local_chain *plc, void *buf,size_t len){
        int ret = 0;
        fseek(plc->active, 0, SEEK_END);
        if(ftell(plc->active)){
                ret = 1;
        }
        fwrite(buf, len, 1, plc->active);
        return ret;
}
void qtdiary_local_chain_archive(qtdiary_local_chain *plc){
        qtdiary_node *pre;
        if(plc->height){
                off_t pre_pos;
                pre = (qtdiary_node *)malloc(sizeof(qtdiary_node));
                pre->state = 0;
                if(plc->height==1){
                        pre_pos = 0;
                        pre->flag = QTDIARY_FRF_ROOT;
                }
                else {
                        fseek(plc->index, -sizeof(off_t)*2, SEEK_END);
                        fread(&pre_pos, sizeof(off_t), 1, plc->index);
                        pre->flag = 0;
                }
                fseek(plc->nodebase, pre_pos, SEEK_SET);
                qtdiary_node_read(pre, plc->nodebase);
                fseek(plc->head, 0, SEEK_SET);
                mpz_t z;
                mpz_init(z);
                mpz_inp_raw(z,plc->head);
                init_pri(pre);
                nettle_ecc_scalar_set(&(pre->pri_key), z);
                mpz_clear(z);
        }
        fseek(plc->active, 0, SEEK_END);
        size_t len = ftell(plc->active);
        uint8_t *buf = (uint8_t *)malloc(len);
        fseek(plc->active, 0, SEEK_SET);
        fread(buf, len, 1, plc->active);
        ftruncate(fileno(plc->active), 0);
        qtdiary_node *new_node = qtdiary_node_from_buf(buf, len, (plc->height==0)?QTDIARY_FRF_ROOT:0, pre);
        fseek(plc->nodebase, plc->chain_tail, SEEK_SET);
        plc->chain_tail = qtdiary_node_write(new_node, plc->nodebase);
        fseek(plc->index, 0, SEEK_SET);
        fwrite(&(plc->height), sizeof(size_t), 1, plc->index);
        fseek(plc->index, 0, SEEK_END);
        fwrite(&(plc->chain_tail), sizeof(off_t), 1, plc->index);
        mpz_t z;
        mpz_init(z);
        nettle_ecc_scalar_get(&(new_node->pri_key), z);
        fseek(plc->head, 0, SEEK_SET);
        mpz_out_raw(plc->head,z);
        mpz_clear(z);
        qtdiary_node_free(new_node);
        free(buf);
        if(plc->height){
                qtdiary_node_free(pre);
        }
        plc->height++;
        fseek(plc->index, 0, SEEK_SET);
        fwrite(&(plc->height), sizeof(size_t), 1, plc->index);
}
#define VERIFY_DO(FUNC) if(!plc->height){      \
        return 0;\
        }\
size_t i=0;                                     \
        fseek(plc->nodebase,0,SEEK_SET);        \
qtdiary_node *pre = qtdiary_node_new(QTDIARY_FRF_ROOT);\
qtdiary_node *cur = qtdiary_node_new(0);;\
qtdiary_node_read(pre, plc->nodebase);\
if(plc->height==1){                               \
FUNC(pre,0);\
        return 0;                               \
}                                               \
qtdiary_node_read(cur, plc->nodebase);\
cur->state|=QTDIARY_FRS_KEYROOT_VERIFIED;\
if(qtdiary_node_verify(cur, pre)){\
		cur->state^=QTDIARY_FRS_KEYROOT_VERIFIED; \
        cur->state|=QTDIARY_FRS_KEYROOT_CORRUPTED; \
        goto bad_chain;\
}\
FUNC(pre,0);   \
FUNC(cur,1);                                     \
pre->flag = 0;                                   \
for(i=2;i<plc->height;i++){\
        qtdiary_node *tmp = pre;\
        pre = cur;\
        cur = tmp;\
        qtdiary_node_read(cur, plc->nodebase);\
        cur->state|=QTDIARY_FRS_KEYROOT_VERIFIED;  \
        if(qtdiary_node_verify(cur, pre)){\
        		cur->state^=QTDIARY_FRS_KEYROOT_VERIFIED;  \
                cur->state|=QTDIARY_FRS_KEYROOT_CORRUPTED; \
                goto bad_chain;\
        }\
FUNC(cur,i);                        \
}\
qtdiary_node_free(pre);\
qtdiary_node_free(cur);\
return 0;\
bad_chain:\
qdwrn("Node verification failed at height %lu",i);\
qtdiary_node_free(pre);\
qtdiary_node_free(cur);\
return -1

#define N(node,i)
int qtdiary_local_chain_verify(qtdiary_local_chain *plc){
        VERIFY_DO(N);
}
#define LOG(cur,i) fprintf(fp,"Node %lu\n",(i));fwrite(cur->content,cur->content_len,1,fp);fputc('\n',fp)
int qtdiary_local_chain_log(qtdiary_local_chain *plc,FILE *fp){
        VERIFY_DO(LOG);
}
#define CONS_LOG(cur,i) qdinfo("Node %lu",(i));fwrite(cur->content,cur->content_len,1,stdout);fputc('\n',stdout)
int qtdiary_local_chain_print(qtdiary_local_chain *plc){
        VERIFY_DO(CONS_LOG);
}
#define DO_FUNC(cur,i) op(cur,i)
int qtdiary_local_chain_node_do(qtdiary_local_chain *plc,node_operator op){
        VERIFY_DO(DO_FUNC);
}
uint8_t *qtdiary_local_chain_get_content(qtdiary_local_chain *plc,size_t height,uint8_t *buf,size_t *len){
        fseek(plc->index, sizeof(size_t)+height*sizeof(off_t), SEEK_SET);
        off_t fcur;
        fread(&fcur, sizeof(off_t), 1, plc->index);
        fseek(plc->nodebase, fcur, SEEK_SET);
        qtdiary_node *cur = qtdiary_node_new((height==0)?QTDIARY_FRF_ROOT:0);
        qtdiary_node_read(cur, plc->nodebase);
        if(len){
                *len = cur->content_len;
        }
        if(buf){
                memcpy(buf, cur->content, cur->content_len);
                qtdiary_node_free(cur);
                return buf;
        }
        else{
                cur->state|=QTDIARY_FRS_CONTENT_INITIALIZED;
                uint8_t *ret = cur->content;
                qtdiary_node_free(cur);
                return ret;
        }
}
void qtdiary_local_chain_close(qtdiary_local_chain *plc){
        fclose(plc->index);
        fclose(plc->nodebase);
        fclose(plc->head);
}
