#include "node.h"
#include "local_chain.h"
#include "nikki.h"
char msg1[]="hello";
char msg2[]="world";
char msg3[]="レミリア最高！";
char fpath[]="test/nodebase";
void test1(){
        FILE* test = fopen("test.chain", "rb");
        qtdiary_node *root = (qtdiary_node *)malloc(sizeof(qtdiary_node));
        root->state = 0;
        root->flag = QTDIARY_FRF_ROOT;
        qtdiary_node *head = (qtdiary_node *)malloc(sizeof(qtdiary_node));
        head->state = 0;
        head->flag = 0;
        qtdiary_node *head2 = (qtdiary_node *)malloc(sizeof(qtdiary_node));
        head2->state = 0;
        head2->flag = 0;
        qtdiary_node_read(root, test);
        qtdiary_node_read(head, test);
        qtdiary_node_read(head2, test);
        printf("%d\n", qtdiary_node_verify(head, root));
        printf("%d\n", qtdiary_node_verify(head2, head));
        qtdiary_node_log(root, stdout);
        qtdiary_node_log(head, stdout);
        qtdiary_node_log(head2, stdout);
}
void test2(){
        qtdiary_local_chain lc;
        qtdiary_local_chain_open(&lc,"test");
        qtdiary_local_chain_push(&lc, msg1, strlen(msg1));
        qtdiary_local_chain_archive(&lc);
        qtdiary_local_chain_push(&lc, msg2, strlen(msg2));
        qtdiary_local_chain_archive(&lc);
        qtdiary_local_chain_push(&lc, msg3, strlen(msg3));
        qtdiary_local_chain_archive(&lc);
        qtdiary_local_chain_close(&lc);
}
void test4(){
        qtdiary_local_chain lc;
        qtdiary_local_chain_open(&lc, "test");
        int err = qtdiary_local_chain_verify(&lc);
        printf("%d\n", err);
}
void test5(){
        qtdiary_local_chain lc;
        qtdiary_local_chain_open(&lc, "test");
        int err = qtdiary_local_chain_print(&lc);
}
void nop(qtdiary_node *pfr,size_t height){
        qtdiary_node_print(pfr);
        printf("%llu\n",pfr->content_len);
}
void test6(){
        qtdiary_local_chain lc;
        qtdiary_local_chain_open(&lc, "test");
        int err = qtdiary_local_chain_node_do(&lc, nop);
        qtdiary_local_chain_close(&lc);
}
void test7(){
        qtdiary_local_chain lc;
        qtdiary_local_chain_open(&lc, "test");
        qtdiary_nikki_create(&lc, "NIKKI!", "test it");
        qtdiary_nikki_push(&lc, msg1, sizeof(msg1), 0);
        qtdiary_nikki_push(&lc, msg2, sizeof(msg2), 0);
        qtdiary_nikki_push(&lc, msg3, sizeof(msg3), 0);
        qtdiary_local_chain_close(&lc);
}
void test8(){
        qtdiary_local_chain lc;
        qtdiary_local_chain_open(&lc, "test");
        qtdiary_nikki_print(&lc);
        qtdiary_local_chain_close(&lc);
}

void test3(){
        FILE* test = fopen(fpath, "wb");
		qtdiary_node *root= qtdiary_node_from_buf(msg1, strlen(msg1), QTDIARY_FRF_ROOT, "this is a chain");
        qtdiary_node *head= qtdiary_node_from_buf(msg2, strlen(msg2), 0,root);
        qtdiary_node *head2= qtdiary_node_from_buf(msg3, strlen(msg3), 0,head);
        qtdiary_node_write(root, test);
        qtdiary_node_write(head, test);
        qtdiary_node_write(head2, test);
}
int main(int argc,char **argv){
        //test7();
        //test6();
        test8();
        return 0;
}
