#include "format.h"
int main(int argc,char **argv){
        gcry_check_version (GCRYPT_VERSION);
        char msg1[]="hello";
        char msg2[]="world";
        qtdiary_file_record *root= qtdiary_file_record_from_buf(msg1, strlen(msg1), QTDIARY_FRF_ROOT, "this is a chain", 0);
        qtdiary_file_record *head= qtdiary_file_record_from_buf(msg2, strlen(msg2), 0,root, 0);
        printf("%d\n", qtdiary_file_record_verify_keychain(head, root));
        return 0;
}
