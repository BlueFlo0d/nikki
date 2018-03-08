#include "node.h"
#include <nettle/ecc-curve.h>
static inline void init_pri(qtdiary_node *pfr){
        if(!(pfr->state&QTDIARY_FRS_PRI_KEY_INITIALIZED)){
                nettle_ecc_scalar_init(&(pfr->pri_key), &nettle_secp_256r1);
                pfr->state^=QTDIARY_FRS_PRI_KEY_INITIALIZED;
        }
}
static inline void qtdiary_node_purge(qtdiary_node *pfr){
        if(pfr->state&QTDIARY_FRS_CONTENT_INITIALIZED){
                free(pfr->content);
                pfr->state^=QTDIARY_FRS_CONTENT_INITIALIZED;
        }
        if(pfr->state&QTDIARY_FRS_NODE_HASH_INITIALIZED){
                free(pfr->node_hash);
                pfr->state^=QTDIARY_FRS_NODE_HASH_INITIALIZED;
        }
}

static inline void init_sn(qtdiary_node *pfr){
        if(!(pfr->state&QTDIARY_FRS_SNODE_HASH_INITIALIZED)){
                nettle_dsa_signature_init(&(pfr->signed_node_hash));
                pfr->state^=QTDIARY_FRS_SNODE_HASH_INITIALIZED;
        }
}
static inline void init_spn(qtdiary_node *pfr){
        if(!(pfr->state&QTDIARY_FRS_SPNODE_HASH_INITIALIZED)){
                nettle_dsa_signature_init(&(pfr->signed_prenode_hash));
                pfr->state^=QTDIARY_FRS_SPNODE_HASH_INITIALIZED;
        }
}
static inline void init_pub(qtdiary_node *pfr){
        if(!(pfr->state&QTDIARY_FRS_PUB_KEY_INITIALIZED)){
                nettle_ecc_point_init(&(pfr->pub_key), &nettle_secp_256r1);
                pfr->state^=QTDIARY_FRS_PUB_KEY_INITIALIZED;
        }
}
