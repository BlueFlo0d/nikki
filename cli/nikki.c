#include "nikki/nikki.h"
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <readline/readline.h>
#define FILE_CHUNK 8096
FILE *tmp;
qtdiary_local_chain lc;
int nfd;
void close_nikki(){
        close(nfd);
}
void write_tmp_to_lc(){
        size_t tsize = ftell(tmp);
        if(isatty(fileno(stdout))&&isatty(fileno(stdin))){
                puts("\n******************");
                puts("* Saving changes *");
                puts("******************");
        }
        char *buf = (char *)malloc(tsize);
        fseek(tmp, 0, SEEK_SET);
        fread(buf, tsize, 1, tmp);
        if(tsize){
                qtdiary_nikki_push(&lc, buf, tsize, ONE_DAY);
        }
        qtdiary_local_chain_close(&lc);
        fclose(tmp);
}
void normal_exit(){
        exit(0);
}
int main(int argc,char **argv){
        struct passwd *pwd = getpwuid(getuid());
        const char *homepath;
        if(pwd){
                homepath = pwd->pw_dir;
        }
        else {
                perror("Get home dir");
                return -1;
        }
        int hfd = open(homepath, O_DIRECTORY);
        if(hfd==-1){
                perror("Open home dir");
                return -1;
        }
        nfd = openat(hfd, ".nikki", O_DIRECTORY);
        if(nfd == -1){
                mkdirat(hfd, ".nikki", S_IRUSR|S_IWUSR|S_IEXEC);
                nfd = openat(hfd, ".nikki", O_DIRECTORY);
        }
        if(nfd == -1){
                perror("Create new nikki");
                return -1;
        }
        atexit(close_nikki);
        close(hfd);
        switch(argc){
        case 1:{
                qtdiary_local_chain_lopen(&lc, nfd);
                fseek(lc.nodebase, 0, SEEK_END);
                if(ftell(lc.nodebase)){

                }
                else {
                        fputs("Creating new nikki.\n",stdout);
                        char *title = readline("Title:");
                        if(!title)
                                return 0;
                        char *summary = readline("Summary:");
                        if(!summary)
                                return 0;
                        qtdiary_nikki_create(&lc, title, summary);
                        puts("Nikki created.");
                }
                tmp = tmpfile();
                atexit(write_tmp_to_lc);
                signal(SIGINT,normal_exit);
                fputc('p', tmp);
                if(isatty(fileno(stdout))&&isatty(fileno(stdin))){
                        puts("QtDiary Nikki ☆ Ver 0.1");
                        puts("The nikki command is designed to be used as a shell filter.");
                        puts("You may edit your note in some file and then cat it to nikki.");
                        puts("For using terminal to write to nikki, press [Ctrl+D] (EOF) to finish your input.");
#ifdef __APPLE__
                        puts("Because of Apple's terminal implementation, you may need to press [Ctrl+D] twice.");
#endif
                        while(1){
                                char *line = readline("");
                                if(line){
                                        fputc('\n', tmp);
                                        fwrite(line, sizeof(char), strlen(line), tmp);
                                        free(line);
                                }
                                else{
                                        break;
                                }
                        }
                }
                else{
                        while(1){
                                char content[FILE_CHUNK];
                                int len;
                                len = fread(content, 1, FILE_CHUNK, stdin);
                                fwrite(content, len, 1, tmp);
                                if(len<FILE_CHUNK){
                                        exit(0);
                                }
                        }
                }
                return 0;
        }
        case 2:{
                if(!strcmp(argv[1], "log")){
                        qtdiary_local_chain_lopen(&lc, nfd);
                        fseek(lc.nodebase, 0, SEEK_END);
                        if(ftell(lc.nodebase)){
                                qtdiary_nikki_print(&lc);
                        }
                        else {
                                fputs("No nikki found.\n",stdout);
                        }
                        return 0;
                }
                else if(!strcmp(argv[1],"dump")){
                        qtdiary_local_chain_lopen(&lc, nfd);
                        fseek(lc.nodebase, 0, SEEK_END);
                        if(ftell(lc.nodebase)){
                                qtdiary_local_chain_node_do_with_args(&lc,qtdiary_node_logop,stdout);
                        }
                        else {
                                fputs("No nikki found.\n",stdout);
                        }
                        return 0;
                }
        }
        default:
                fputs("Invalid paraments.\n",stdout);
        }
        return 0;
}
