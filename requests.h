#ifndef _REQUESTS_
#define _REQUESTS_

/*
 * Compune si returneaza o cerere GET/DELETE
 * (query_params si cookie-urile pot fi setate NULL daca nu sunt necesare)
 */
char *compute_some_request(char *host, char *line, char *token_jwt, 
                            char **cookies, int cookies_count);
                            
void get_req(char *line, char *url, char *query_params);
void delete_req(char *line, char *url, char *query_params);

char *compute_get_request(char *host, char *url, char *query_params,
                            char *token_jwt, char **cookies, int cookies_count);

char *compute_delete_request(char *host, char *url, char *query_params,
                            char *token_jwt, char **cookies, int cookies_count);

// Compune si returneaza o cerere POST (cookie-urile pot fi NULL daca nu sunt necesare)
char *compute_post_request(char *host, char *url, char* content_type, char* token_jwt,
                            char *data, char **cookies, int cookies_count);

#endif
