#include "util.h"
#include "nikki.h"
#include <string.h>
#undef QTDIARY_PROMPT
#define QTDIARY_PROMPT "Qt Nikki"
void print_time(time_t *pt,const char *fmt){
        struct tm *lt;
        lt = localtime(pt);
        char str_time[100];
        strftime(str_time, sizeof(str_time), "%Y-%m-%d,%H:%M:%S", lt);
        printf(fmt, str_time);
}
int qtdiary_nikki_create(qtdiary_local_chain *plc,char *title,char *summary){
        if(plc->height||plc->chain_tail){
                qderrs("Cannot create nikki at a non-empty node chain");
                return -1;
        }
        uint16_t typeinfo = (QTDIARY_TYPE_NIKKI<<8)|QTDIARY_NIKKI_VERSION;
        qtdiary_local_chain_push(plc, &typeinfo, sizeof(uint16_t));
        time_t cur;
        time(&cur);
        qtdiary_local_chain_push(plc, &cur, sizeof(time_t));
        size_t len;
        len = strlen(title);
        qtdiary_local_chain_push(plc, &len, sizeof(size_t));
        qtdiary_local_chain_push(plc, title, len);
        len = strlen(summary);
        qtdiary_local_chain_push(plc, &len, sizeof(size_t));
        qtdiary_local_chain_push(plc, summary, len);
        qtdiary_local_chain_archive(plc);
        return 0;
}
void qtdiary_nikki_push(qtdiary_local_chain *plc,char *buf,size_t len,time_t interval){
        time_t cur,last_update;
        time(&cur);
        int pret = qtdiary_local_chain_push(plc, &cur, sizeof(time_t));
        qtdiary_local_chain_push(plc, &len, sizeof(size_t));
        qtdiary_local_chain_push(plc, buf, len);
        if(pret){
                fseek(plc->nodebase, plc->chain_tail+sizeof(size_t), SEEK_SET);
                fread(&last_update, sizeof(time_t), 1, plc->nodebase);
                if(difftime(cur, last_update)>interval){
                        qtdiary_local_chain_archive(plc);
                }
        }
}
void node_nikki_print(qtdiary_node *pfr,size_t i){
        char *buf = pfr->content;
        if(pfr->flag&QTDIARY_FRF_ROOT){
                buf+=sizeof(uint16_t);
                print_time((time_t *)buf, "Date: %s\n");
                buf+=sizeof(time_t);
                fputs("Title: ", stdout);
                size_t clen = *(size_t *)buf;
                buf+=sizeof(size_t);
                fwrite(buf, clen, 1, stdout);
                buf+=clen;
                fputc('\n', stdout);
                fputs("Summary: ", stdout);
                clen = *(size_t *)buf;
                buf+=sizeof(size_t);
                fwrite(buf, clen, 1, stdout);
                fputc('\n', stdout);
        }
        else {
                while(buf<pfr->content+pfr->content_len){
                        print_time((time_t *)buf,"[%s] ");
                        buf+=sizeof(time_t);
                        size_t clen = *(size_t *)buf;
                        buf+=sizeof(clen);
                        fwrite(buf, clen, 1, stdout);
                        fputc('\n',stdout);
                        buf+=clen;
                }
        }
}
void qtdiary_nikki_print(qtdiary_local_chain *plc){
        if(!qtdiary_local_chain_node_do(plc, node_nikki_print)){
                fseek(plc->active, 0, SEEK_END);
                size_t ftail = ftell(plc->active);
                fseek(plc->active, 0, SEEK_SET);
                while(ftell(plc->active)<ftail){
                        time_t t;
                        fread(&t, sizeof(time_t), 1, plc->active);
                        print_time(&t,"[%s] ");
                        size_t clen;
                        fread(&clen, sizeof(size_t), 1, plc->active);
                        char *buf=(char *)malloc(clen);
                        fread(buf, clen, 1, plc->active);
                        fwrite(buf, clen, 1, stdout);
                        free(buf);
                        fputc('\n', stdout);
                }
        }
}
