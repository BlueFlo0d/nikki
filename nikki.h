#include "local_chain.h"
#include <time.h>
#define ONE_DAY (24*3600)
#define QTDIARY_TYPE_NIKKI 0x01
#define QTDIARY_NIKKI_VERSION 0x00
int qtdiary_nikki_create(qtdiary_local_chain *plc,char *title,char *summary);
void qtdiary_nikki_push(qtdiary_local_chain *plc,char *buf,size_t len,time_t interval);
void qtdiary_nikki_print(qtdiary_local_chain *plc);
