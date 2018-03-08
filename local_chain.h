#ifndef QTDIARY_LOCAL_CHAIN_H
#define QTDIARY_LOCAL_CHAIN_H
#include "node.h"
typedef struct _qtdiary_local_chain{
        FILE *index;
        FILE *nodebase;
        FILE *head;
        FILE *active;
        size_t height;
        off_t chain_tail;
} qtdiary_local_chain;
typedef void (*node_operator)(qtdiary_node *pfr,size_t height);
void qtdiary_local_chain_open(qtdiary_local_chain *plc,const char *path);
void qtdiary_local_chain_close(qtdiary_local_chain *plc);
int qtdiary_local_chain_push(qtdiary_local_chain *plc, void *buf,size_t len);
void qtdiary_local_chain_archive(qtdiary_local_chain *plc);
int qtdiary_local_chain_verify(qtdiary_local_chain *plc);
int qtdiary_local_chain_log(qtdiary_local_chain *plc,FILE *fp);
int qtdiary_local_chain_node_do(qtdiary_local_chain *plc,node_operator op);
uint8_t *qtdiary_local_chain_get_content(qtdiary_local_chain *plc,size_t height,uint8_t *buf,size_t *len);
#endif
