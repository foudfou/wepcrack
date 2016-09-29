const char xmpp_cli_first1[] = "n,,n=koma_test,r=hydra";
const char xmpp_srv_first1[] = "r=hydraFe3A1scL7C0jtKsm+kcg96MWg769FuRu,"
    "s=kM6lTjjnZW4F8WLboyagcA==,i=4096";
const char xmpp_cli_final1[] = "c=biws,r=hydraFe3A1scL7C0jtKsm+kcg96MWg769FuRu,"
    "p=mZU2Qekd8JR7ybCtb3hnJMGEfIg=";

// These are manually extracted for now...
const char xmpp_salt1[] = "kM6lTjjnZW4F8WLboyagcA==";
const int xmpp_iter1 = 4096;
// client_first_bare,server_first,client_final_without_proof
const char xmpp_auth_msg1[] = "n=koma_test,r=hydra,"
    "r=hydraFe3A1scL7C0jtKsm+kcg96MWg769FuRu,s=kM6lTjjnZW4F8WLboyagcA==,i=4096,"
    "c=biws,r=hydraFe3A1scL7C0jtKsm+kcg96MWg769FuRu";
const char xmpp_cli_final_proof1[] = "mZU2Qekd8JR7ybCtb3hnJMGEfIg=";


/* pwd2 = 'koma_test' */
/* cli_fst2 = 'n,,n=koma_test,r=hydra' */
/* srv_fst2 = 'r=hydra4OjoFBGFJyzTaBWKiGfuqNM+v9rDA0wn,s=qgiJIJQsQPhvAotJWVNHPQ==,i=4096' */
/* cli_fin2 = 'c=biws,r=hydra4OjoFBGFJyzTaBWKiGfuqNM+v9rDA0wn,p=anvxRRv7SVKIwwsJ3Y6/0hKC0YU=' */

const char xmpp_pwd2[] = "koma_test";
const char xmpp_cli_first2[] = "n,,n=koma_test,r=hydra";
const char xmpp_srv_first2[] = "r=hydra4OjoFBGFJyzTaBWKiGfuqNM+v9rDA0wn,"
    "s=qgiJIJQsQPhvAotJWVNHPQ==,i=4096";
const char xmpp_cli_final2[] = "c=biws,r=hydra4OjoFBGFJyzTaBWKiGfuqNM+v9rDA0wn,"
    "p=anvxRRv7SVKIwwsJ3Y6/0hKC0YU=";

const char xmpp_salt2[] = "qgiJIJQsQPhvAotJWVNHPQ==";
const int xmpp_iter2 = 4096;
const char xmpp_auth_msg2[] = "n=koma_test,r=hydra,"
    "r=hydra4OjoFBGFJyzTaBWKiGfuqNM+v9rDA0wn,s=qgiJIJQsQPhvAotJWVNHPQ==,i=4096,"
    "c=biws,r=hydra4OjoFBGFJyzTaBWKiGfuqNM+v9rDA0wn";
const char xmpp_cli_final_proof2[] = "anvxRRv7SVKIwwsJ3Y6/0hKC0YU=";
