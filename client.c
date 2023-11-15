#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <stdbool.h>
#include "helpers.h"
#include "requests.h"
#include "parson/parson.h"
#include "parson/parson.c"

#define HOST "34.254.242.81"
#define PORT 8080
#define PAYLOAD "application/json"
#define ROUTE "/api/v1/tema/"


/*
 * Functie pentru a verifica daca inputul este valid
 * si nu contine spatii (username si password)
 */
int without_spaces (char *buf) {
    char *p = strtok(buf, " ");
    p = strtok(NULL, " ");
    if (p != NULL) {
        printf("Do not use spaces. Retry.\n");
        return 1;
    }
    else return 0;
}

/*
 * Functie care realizeaza citirea si verificarea
 * corectitudinii scrise a datelor de inregistrare/logare
 */
int data_completion(char *username, char *password) {
    printf("username=");
    fgets (username, LINELEN, stdin);

    int user_length = strlen(username);
    username[user_length - 1] = '\0';

    if (without_spaces(username) == 1)
        return 1;

    printf("password=");
    fgets (password, LINELEN, stdin);

    int pass_length = strlen(password);
    password[pass_length - 1] = '\0';

    if (without_spaces(password) == 1)
        return 1;
    
    return 0;
}

/*
 * Functie care verifica daca utilizatorul
 * are acces la biblioteca sau nu
 * (este logat si se afla in biblioteca)
 */
bool permission(bool log_in, bool in_lib) {
    if (!log_in) {
        printf("You must log in first\n");
        return false;
    }
    if (!in_lib) {
        printf("You must enter in the library first\n");
        return false;
    }
    return true;
}

/*
 * Functie care verifica daca un string
 * este sub forma de numar
 */
int number(char *string) {
    for (int i = 0;  i < strlen(string); i++) {
        if (string[i] < '0' || string[i] > '9') {
            return 0;
        }
    }
    return 1;
}

/*
 * Functie care realizeaza serializarea datelor 
 * utilizatorului pentru autentificare
 */
char *serialize_user(char *username, char *password) {
    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);
    json_object_set_string(object, "username", username);
    json_object_set_string(object, "password", password);

    return json_serialize_to_string(value);
}

/*
 * Functie care creeaza json-ul continand 
 * date despre carte si il returneaza serializat
 */
char *serialize_book(char *title, char *author, char *publisher, char *genre, char*page_count) {
    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);
    json_object_set_string(object, "title", title);
    json_object_set_string(object, "author", author);
    json_object_set_string(object, "publisher", publisher);
    json_object_set_string(object, "genre", genre);
    json_object_set_string(object, "page_count", page_count);

    return json_serialize_to_string(value);
}

/*
 * Functie pentru inregistrarea unui nou utilizator
 */
void registration(int sockfd, char *username, char *password) {
    char *data = serialize_user(username, password);

    char *message = compute_post_request(HOST, "/api/v1/tema/auth/register", PAYLOAD, NULL, data, NULL, 0);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Se trateaza cazul de eroare in care utilizatorul exista deja
    if (strstr(response, "400 Bad Request"))
        printf("The username is already taken! Choose a different username!\n");
    else 
        printf("200 - OK - Registration complete\n");
}

/*
 * Functie pentru logare
 */
int logging(int sockfd, char *username, char *password, char **cookies) {
    char *data = serialize_user(username, password);
    char *message = compute_post_request(HOST, "/api/v1/tema/auth/login", PAYLOAD, NULL, data, NULL, 0);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Se trateaza cele 2 cazuri de eroare
    if (strstr(response, "error")) {
        // Cand nu introducem bine parola
        if (strstr(response, "Credentials are not good!")) {
            printf("Incorrect password. Retry.\n");
            return 1;
        }
        // Cand utilizatorul nu exista
        else {
            printf("No account found. Register.\n");
            return 1;
        }
    }

    /*
     * Se extrage din raspunsul primit de la server cookie-ul de conectare
     */
    char *cookie = strstr(response, "Set-Cookie:");
    cookie = strstr(cookie, "connect");
    cookie = strtok(cookie, ";");

    if (cookie != NULL) {
        cookies[0] = cookie;
        printf("Connected!\n");
        return 0;
    }
    printf("Couldn't connect\n");
    return 1;

}

/*
 * Functie pentru obtinerea token-ului care ofera acces la biblioteca
 */
char *entry_library(int sockfd, char **cookies) {
    char *request = compute_get_request(HOST, "/api/v1/tema/library/access", PAYLOAD, NULL, cookies, 1);
    send_to_server(sockfd, request);
    char *response = receive_from_server(sockfd);

    /*
     * Se extrage token-ul specific din raspunsul primit de la server
     */
    char *token = strstr(response, "token");
    char *tok = strstr(token, ":\"");
    tok = strtok(tok, "\"");
    tok = strtok(NULL, "\"");

    if (tok)
        printf("You have entered the library!\n");
    else printf("You can not enter the library\n");
    
    return tok;
}

/*
 * Functie pentru obtinerea tuturor cartilor din biblioteca
 */
void all_books(int sockfd, char *token_jwt) {
    char *request = compute_get_request(HOST, "/api/v1/tema/library/books", PAYLOAD, token_jwt, NULL, 0);
    send_to_server(sockfd, request);
    char *response = receive_from_server(sockfd);
    char *books = strstr(response, "[");
    printf("%s\n", books);
}

/*
 * Functie pentru obtinerea informatiilor despre 
 * o anumita carte (cea cu id-ul "id")
 */
void one_book(int sockfd, char *token_jwt, char *id) {
    // Se compune adresa la care se cauta cartea
    char address[150] = "/api/v1/tema/library/books/";
    strcat(address, id);

    char *request = compute_get_request(HOST, address, PAYLOAD, token_jwt, NULL, 0);
    send_to_server(sockfd, request);
    char *response = receive_from_server(sockfd);

    /*
     * Se trateaza cazul de eroare cand id-ul introdus este gresit
     * si se afiseaza informatiile daca este corect
     */
    if (strstr(response, "error")) {
        printf("Wrong ID. No book was found. Retry.\n");
    } else {
        printf("Look what I found!\n");
        char *book = strstr(response, "{");
        printf("%s\n", book);
    }
}

/*
 * Functie pentru adaugarea unei noi carti in biblioteca
 */
void adding_book(int sockfd, char *token_jwt) {
    char title[LINELEN], author[LINELEN], publisher[LINELEN];
    char genre[LINELEN], page_count[LINELEN];

    /*
     * Se primesc datele de la utilizator
     */
    printf("title=");
    fgets(title, LINELEN, stdin);
    if (strcmp(title, "\n") == 0) {
        printf("Your book must have a title.\n");
        printf("If unknown, please complete with \"-\".\n");
        return;
    }

    printf("author=");
    fgets(author, LINELEN, stdin);
    if (strcmp(author, "\n") == 0) {
        printf("Your book must have an author.\n");
        printf("If unknown, please complete with \"-\".\n");
        return;
    }

    printf("publisher=");
    fgets(publisher, LINELEN, stdin);
    if (strcmp(publisher, "\n") == 0) {
        printf("Your book just have a publisher.\n");
        printf("If unknown, please complete with \"-\".\n");
        return;
    }

    printf("genre=");
    fgets(genre, LINELEN, stdin);
    if (strcmp(genre, "\n") == 0) {
        printf("Your book just have a genre.\n");
        printf("If unknown, please complete with \"-\".\n");
        return;
    }

    printf("page_count=");
    fgets(page_count, LINELEN, stdin);
    if (strcmp(page_count, "\n") == 0) {
        printf("Your book just have a page count.\n");
        printf("If unknown, please complete with \"-\".\n");
        return;
    }

    title[strlen(title) - 1] = '\0';
    author[strlen(author) - 1] = '\0';
    publisher[strlen(publisher) - 1] = '\0';
    genre[strlen(genre) - 1] = '\0';
    page_count[strlen(page_count) - 1] = '\0';

    // Se verifica ca numarul de pagini sa fie intradevar un numar
    if (!number(page_count)) {
        printf("The page count must be a number! Retry.\n");
        return;
    }

    char *book = serialize_book(title, author, publisher, genre, page_count);
    char *request = compute_post_request(HOST, "/api/v1/tema/library/books", 
                    PAYLOAD, token_jwt, book, NULL, 0);
    send_to_server(sockfd, request);
    char *response = receive_from_server(sockfd);
    
    if (strstr(response, "error")) 
        printf("Something gone wrong. Retry.\n");
    else 
        printf("Book successfully added to your library!\n");

}

/*
 * Functie pentru stergerea unei carti din biblioteca
 */
void deleting_book(int sockfd, char *token_jwt, char *id) {
    char address[LINELEN + 30] = "/api/v1/tema/library/books/";
    strcat(address, id);

    char *request = compute_delete_request(HOST, address, PAYLOAD, token_jwt, NULL, 0);
    send_to_server(sockfd, request);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) {
        printf("Wrong ID. No book was found. Retry.\n");
    } else {
        printf("Successfully removed!\n");
    }
}

/*
 * Functie pentru deconectare
 */
void loggingout(int sockfd, char **cookies) {
    char *request = compute_get_request(HOST, "/api/v1/tema/auth/logout", PAYLOAD, NULL, cookies, 1);
    send_to_server(sockfd, request);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) 
        printf("You couldn't logout. Retry.\n");
    else
        printf("Successfully logout. You are free now!\n");
}

int main(int argc, char *argv[])
{
    // log_in = true daca exista un utilizator conectat
    // in_lib = true daca utilizatorul conectat are acces la biblioteca
    bool log_in = false, in_lib = false;  

    // Socketul pe care se trimit si se primesc date
    int sockfd;

    // action = instructiunea primita de la intrarea standard
    char *action = malloc(LINELEN);

    // Token-ul si Cookie-urile de access
    char *token_jwt;
    char **cookies = malloc(2 * sizeof(char));

    while(1) {
        // Se deschide o conexiune
        sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        fgets(action, LINELEN, stdin);

        /*
         * In functie de inputul primit, se realizeaza actiunile corespunzatoare
         */
        if (strcmp (action, "register\n") == 0) {
            char *username = malloc(sizeof(char));
            char *password = malloc(sizeof(char));
            // Daca username-ul sau parola contin spatii, nu se face inregistrarea
            if (data_completion(username, password) == 1)
                continue;
            registration(sockfd, username, password);
        }
        else if (strcmp(action, "login\n") == 0) {
            // Daca suntem deja conectati, trebuie sa ne deconectam inainte de alta conectare
            if (log_in) {
                printf("You are already connected. Deconnect before accesing another account\n");
                continue;
            } 
            char *username = malloc(sizeof(char));
            char *password = malloc(sizeof(char));
            if (data_completion(username, password) == 1)
                continue;
            int log = logging(sockfd, username, password, cookies);
            if (log == 0)
                log_in = true;
        
        }
        else if (strcmp(action, "enter_library\n") == 0) {
            // Putem intra in biblioteca doar daca suntem conectati
            if (!log_in) {
                printf("You must log in first\n");
                continue;
            }
            token_jwt = entry_library(sockfd, cookies);
            in_lib = true;
        
        }
        else if (strcmp(action, "get_books\n") == 0) {
            // Putem afisa cartile doar daca suntem conectati si in biblioteca
            if (permission(log_in, in_lib))
                all_books(sockfd, token_jwt);
        }
        else if (strcmp(action, "get_book\n") == 0) {
            // Putem afisa informatii doar daca suntem conectati si in biblioteca
            if (!permission(log_in, in_lib)) {
                continue;
            }
            // Informatiile vor fi afisate doar pentru cartea cu id-ul "id"
            char *id = malloc(sizeof(char));
            printf("id=");
            fgets(id, LINELEN, stdin);
            id[strlen(id) - 1] = '\0';
            
            if (!number(id)) {
                printf("Give me an id number. Retry.\n");
                continue;
            }
            one_book(sockfd, token_jwt, id);
        }
        else if (strcmp(action, "add_book\n") == 0) {
            // Putem adauga carti doar daca suntem conectati si in biblioteca
            if (!permission(log_in, in_lib))
                continue;
            
            adding_book(sockfd, token_jwt);
        }
        else if (strcmp(action, "delete_book\n") == 0) {
            // Putem sterge cartile doar daca suntem conectati si in biblioteca
            if (!permission(log_in, in_lib)) {
                continue;
            }
            // Se va sterge doar cartea cu id-ul "id"
            char *id = malloc(sizeof(char));
            printf("id=");
            fgets(id, LINELEN, stdin);
            id[strlen(id) - 1] = '\0';
            
            if (!number(id)) {
                printf("Give me an id number. Retry.\n");
                continue;
            }
            deleting_book(sockfd, token_jwt, id);
        }
        else if(strcmp(action, "logout\n") == 0) {
            // Ne putem deconecta doar daca sutem conectati
            if (log_in) {
                log_in = 0;
                in_lib = 0;
                loggingout(sockfd, cookies);
            } else {
                printf("You can not logout. You are not even connected.\n");
            }
        }
        else if(strcmp(action, "exit\n") == 0)
            break;
        else {
            /*
             * Daca instructiunea primita nu este niciuna dintre cele de mai sus
             * inseamna ca instructiunea primita nu este valida
             */
            printf("I don't know what you want!\n");
            printf("Give valid command!\n");
            printf("Ex: register, login, enter_library, get_books, get_book, add_book, delete_book, logout, exit.\n");
        }
    }
    return 0;
}
