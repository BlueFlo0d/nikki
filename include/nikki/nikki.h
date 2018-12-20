#include "local_chain.h"
#include <time.h>
#define ONE_DAY (24*3600)
#define QTDIARY_TYPE_NIKKI 0x01
#define QTDIARY_NIKKI_VERSION 0x00
typedef void (*record_operator)(char *content,size_t len,time_t time);
typedef void (*nikki_operator)(char *title,size_t tlen,char *summary,size_t slen,time_t time);
int qtdiary_nikki_create(qtdiary_local_chain *plc,char *title,char *summary);
void qtdiary_nikki_push(qtdiary_local_chain *plc,char *buf,size_t len,time_t interval);
void qtdiary_nikki_print(qtdiary_local_chain *plc);
void qtdiary_nikki_do(qtdiary_local_chain *plc,record_operator op,nikki_operator sop);
void qtdiary_nikki_do_for_path(const char *path,record_operator op,nikki_operator sop);
