// These are manually extracted for now...
const unsigned char xmpp_salt1[] = "kM6lTjjnZW4F8WLboyagcA==";
const int xmpp_salt1_len = 24;
const int xmpp_iter1 = 4096;
// client_first_bare,server_first,client_final_without_proof
const char xmpp_auth_msg1[] = "n=koma_test,r=hydra,"
    "r=hydraFe3A1scL7C0jtKsm+kcg96MWg769FuRu,s=kM6lTjjnZW4F8WLboyagcA==,i=4096,"
    "c=biws,r=hydraFe3A1scL7C0jtKsm+kcg96MWg769FuRu";
const char xmpp_cli_final_proof1[] = "mZU2Qekd8JR7ybCtb3hnJMGEfIg=";

const char xmpp_pwd2[] = "koma_test";
const char xmpp_salt2[] = "qgiJIJQsQPhvAotJWVNHPQ==";
const int xmpp_salt2_len = 24;
const int xmpp_iter2 = 4096;
const char xmpp_auth_msg2[] = "n=koma_test,r=hydra,"
    "r=hydra4OjoFBGFJyzTaBWKiGfuqNM+v9rDA0wn,s=qgiJIJQsQPhvAotJWVNHPQ==,i=4096,"
    "c=biws,r=hydra4OjoFBGFJyzTaBWKiGfuqNM+v9rDA0wn";
const char xmpp_cli_final_proof2[] = "anvxRRv7SVKIwwsJ3Y6/0hKC0YU=";
