#ifndef QTDIARY_UTIL_H
#define QTDIARY_UTIL_H
#define QTDIARY_PROMPT "QtDiary"
#define qdwrn(str,...) printf("\033[;33m["QTDIARY_PROMPT" Warning] "str"\033[0m\n",__VA_ARGS__)
#define qdinfo(str,...) printf("\033[;34m["QTDIARY_PROMPT" Info] "str"\033[0m\n",__VA_ARGS__)
#define qderr(str,...) printf("\033[;31m["QTDIARY_PROMPT" Info] "str"\033[0m\n",__VA_ARGS__)

#define qdwrns(str) printf("\033[;33m["QTDIARY_PROMPT" Warning] "str"\033[0m\n")
#define qdinfos(str) printf("\033[;34m["QTDIARY_PROMPT" Info] "str"\033[0m\n")
#define qderrs(str) printf("\033[;31m["QTDIARY_PROMPT" Info] "str"\033[0m\n")

#endif
