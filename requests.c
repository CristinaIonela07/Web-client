#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

/*
 * Functie ajutatoare pentru formarea mesajului de GET
 * formeaza prima linie dintr-un request de tip GET
 */
void get_req(char *line, char *url, char *query_params) {
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
}

/*
 * Functie ajutatoare pentru formarea mesajului de DELETE
 * formeaza prima linie dintr-un request de tip DELETE
 */
void delete_req(char *line, char *url, char *query_params) {
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
}

/*
 * Functie comuna pentru realizarea request-urilor de GET si DELETE
 */
char *compute_some_request(char *host, char *line, char *token_jwt, 
                            char **cookies, int cookies_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    compute_message(message, line);

    // Se adauga HOST
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Se adauga token-ul in header-ul "Authorization"
    if (token_jwt != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token_jwt);
        compute_message(message, line);
    }

    // Se adauga cookie-urile, conform formatului protocolului
    memset(line, 0, LINELEN);
    if (cookies != NULL) {
        strcat(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            strcat(line, "; ");
        }
        compute_message(message, line);
    }

    // Se adauga o linie noua la final
    compute_message(message, "");
    free(line);
    return message;
}

/*
 * Functia GET_REQUEST
 */
char *compute_get_request(char *host, char *url, char *query_params,
                            char *token_jwt, char **cookies, int cookies_count) {
    char *line = calloc(LINELEN, sizeof(char)); 
    get_req(line, url, query_params);
    return compute_some_request(host, line, token_jwt, cookies, cookies_count);
}

/*
 * Functia DELETE_REQUEST
 */
char *compute_delete_request(char *host, char *url, char *query_params,
                            char *token_jwt, char **cookies, int cookies_count) {
    char *line = calloc(LINELEN, sizeof(char)); 
    delete_req(line, url, query_params);
    return compute_some_request(host, line, token_jwt, cookies, cookies_count);
}

/*
 * Functia POST_REQUEST
 */
char *compute_post_request(char *host, char *url, char* content_type, char* token_jwt,
                            char *data, char **cookies, int cookies_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Se scrie numele metodei, URL-ul si tipul protocolului.
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Se adauga HOST
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Se adauga token-ul in header-ul "Authorization"
    if (token_jwt != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token_jwt);
        compute_message(message, line);
    }

    /* 
     * Se adauga datele necesare (Content-Type si Content-Length neaparat)
     * pentru a scrie Content-Length trebuie sa se calculeze intai lungimea mesajului
     */
    memset(line, 0, LINELEN);
    sprintf(line, "Content-type: %s", content_type);
    compute_message(message, line);

    int length = strlen(data);

    // Se adauga lungimea continutului request-ului in campul content-length
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Length: %d", length);
    compute_message(message, line);

    // Se adauga cookie-urile
    memset(line, 0, LINELEN);
    if (cookies != NULL) {
        strcat(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            strcat(line, "; ");
        }
        compute_message(message, line);
    }

    // Se adauga o linie noua la final de header
    compute_message(message, "");

    // Se adauga datele efective
    memset(line, 0, LINELEN);
    strcat(message, data);

    free(line);
    return message;
}
