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
#include "parson.h"
#include "parson.c"
#define HOST "34.241.4.235"
#define PORT 8080
#define REGISTER "/api/v1/tema/auth/register"
#define LOGIN "/api/v1/tema/auth/login"
#define LOGOUT "/api/v1/tema/auth/logout"
#define ACCESS "/api/v1/tema/library/access"
#define BOOKS "/api/v1/tema/library/books"
#define PAYLOADTYPE "application/json"

int main()
{
    int sockfd;
    //char buffer[BUFLEN]; // comanda citita de la tastatura
    char username[BUFLEN];
    char password[BUFLEN];
    char *message;
    char *response;
    char *JWToken;
    char **cookies = NULL;
    bool loged = false;

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0); // conectez clientul la server
    while (1)
    {
        char buffer[BUFLEN];
        memset(buffer, 0, BUFLEN);
        fflush(stdin);
        fgets(buffer, BUFLEN, stdin);
        // If "exit" is typed in from STDIN
        if (strncmp(buffer, "exit", 4) == 0)
        {
            if (loged)
            {
                free(JWToken);
                JWToken = NULL;
                free(cookies[0]);
                free(cookies);
                cookies = NULL;
            }
            break;
        }
        // If "register" is typed from STDIN
        else if (strncmp(buffer, "register", 8) == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if(loged == true){
                fprintf(stderr, "Nu te poti inregistra in timp ce esti autentificat!\n");
                continue;
            }
            printf("username=");
            fgets(username, BUFLEN, stdin);
            username[strlen(username) - 1] = '\0';
            printf("password=");
            fgets(password, BUFLEN, stdin);
            password[strlen(password) - 1] = '\0';

            if (strchr(username, ' ') != NULL || strchr(password, ' ') != NULL)
            {
                fprintf(stderr, "Username-ul si parola nu trebuie sa contina spatii!\n");
                continue;
            }

            //Creating JSON Object and converting it to JSON String
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            char *serialized_string = NULL;
            json_object_set_string(root_object, "username", username);
            json_object_set_string(root_object, "password", password);
            serialized_string = json_serialize_to_string_pretty(root_value);

            message = compute_post_request(HOST, REGISTER, PAYLOADTYPE, serialized_string, NULL, 0, NULL);
            //printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            //printf("%s\n", response);
            //puts(response);
            char *myError = basic_extract_json_response(response);
            //printf("%s\n", myError);

            if (myError == NULL)
                printf("200 - OK - Utilizator inregistrat cu succes!\n");
            else
            {
                JSON_Value *root_v = json_parse_string(myError);
                JSON_Object *root_o = json_value_get_object(root_v);
                //printf("%s", json_object_get_string(root_o, "error"));
                //error(json_object_get_string(root_o, "error"));
                char messageError[BUFLEN];
                memcpy(messageError, json_object_get_string(root_o, "error"), BUFLEN);
                if (strncmp(messageError, "The username", 12) == 0)
                {
                    fprintf(stderr, "Exista deja un cont cu username-ul %s!\n", username);
                }
                else
                {
                    fprintf(stderr, "%s\n", messageError);
                }
                json_value_free(root_v);
            }

            json_free_serialized_string(serialized_string);
            json_value_free(root_value);
            continue;
        }
        else if (strncmp(buffer, "login", 5) == 0)
        {
            //If the user is already logged in
            if (loged)
            {
                printf("Sunteti deja logat!\n");
                continue;
            }

            //Else, the user can proceed to login
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            printf("username=");
            fscanf(stdin, "%s", username);
            printf("password=");
            fscanf(stdin, "%s", password);

            //Creating JSON Object and converting it to JSON String
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            char *serialized_string = NULL;
            json_object_set_string(root_object, "username", username);
            json_object_set_string(root_object, "password", password);
            serialized_string = json_serialize_to_string_pretty(root_value);

            message = compute_post_request(HOST, LOGIN, PAYLOADTYPE, serialized_string, NULL, 0, NULL);
            //printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            //printf("%s\n", response);
            //puts(response);
            char *myError = basic_extract_json_response(response);
            //printf("%s\n", myError);

            if (myError == NULL)
            {
                printf("200 - OK - Bun venit!\n");
                loged = true;

                //Get cookie from server's answer
                cookies = calloc(1, sizeof(char *));
                cookies[0] = calloc(LINELEN, sizeof(char));
                char *from = strstr(response, "Set-Cookie:");
                char *cookie = strtok(from + 12, ";");
                memcpy(cookies[0], cookie, strlen(cookie));
                //Decomentati linia urmatoare pentru a vedea cookie-ul de sesiune
                //printf("%s\n", cookies[0]);
            }
            else
            {
                JSON_Value *root_v = json_parse_string(myError);
                JSON_Object *root_o = json_value_get_object(root_v);
                //printf("%s", json_object_get_string(root_o, "error"));
                //error(json_object_get_string(root_o, "error"));
                char messageError[30];
                memcpy(messageError, json_object_get_string(root_o, "error"), 30);
                if (strcmp(messageError, "Credentials are not good!") == 0)
                {
                    fprintf(stderr, "%s\n", "Parola este gresita!");
                }
                else if (strcmp(messageError, "No account with this username!") == 0)
                {
                    fprintf(stderr, "%s\n", "Nu exista un cont cu acest username!");
                }
                else
                {
                    fprintf(stderr, "%s\n", messageError);
                }
                json_value_free(root_v);
            }

            json_free_serialized_string(serialized_string);
            json_value_free(root_value);
            continue;
        }
        else if (strncmp(buffer, "enter_library", 13) == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            //If the user is not logedin
            /*if (!loged)
            {
                fprintf(stderr, "Nu sunteti autentificat, deci nu aveti access la biblioteca!\n");
                continue;
            }*/

            message = compute_get_request(HOST, ACCESS, NULL, cookies, 1, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            //Extract the JSON
            char *myError = basic_extract_json_response(response);
            //printf("%s\n", myError);
            //Looking for errors
            char *error = strstr(myError, "error");
            //Looking for a token
            char *token = strstr(myError, "token");

            // The server sent an error
            if (error != NULL)
            {
                JSON_Value *root_v = json_parse_string(myError);
                JSON_Object *root_o = json_value_get_object(root_v);
                char messageError[BUFLEN];
                memcpy(messageError, json_object_get_string(root_o, "error"), BUFLEN);
                if (strcmp(messageError, "You are not logged in!") == 0)
                {
                    fprintf(stderr, "Nu sunteti autentificat, deci nu aveti access la biblioteca!\n");
                }
                else
                {
                    fprintf(stderr, "%s\n", messageError);
                }
                json_value_free(root_v);
            }
            // The server sent a token for the user access
            else if (token != NULL)
            {
                printf("Ati intrat cu succes in biblioteca!\n");
                JSON_Value *root_v = json_parse_string(myError);
                JSON_Object *root_o = json_value_get_object(root_v);
                //Extract the token from the serves's message
                JWToken = (char *)malloc(BUFLEN * sizeof(char));
                memset(JWToken, 0, BUFLEN);
                memcpy(JWToken, json_object_get_string(root_o, "token"), BUFLEN);

                //Decomentati aceasta linie pentru a vedea tokenul primit din partea serverului
                //printf("%s\n", JWToken);
            }
        }
        else if (strncmp(buffer, "get_books", 9) == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            //If the user was not granted the access to the library
            /*if (JWToken == NULL)
            {
                fprintf(stderr, "Nu aveti access la biblioteca!\n");
                continue;
            }*/

            message = compute_get_request(HOST, BOOKS, NULL, cookies, 1, JWToken);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            //printf("%s\n", response);
            if (strstr(response, "[]") != NULL)
            {
                fprintf(stderr, "Nu aveti carti in biblioteca!\n");
                continue;
            }
            char *myError = basic_extract_json_response(response);
            //printf("%s\n", myError);
            char *error = NULL;

            //Looking for errors
            if (myError != NULL)
            {
                error = strstr(myError, "error");
            }

            // The server sent an error
            if (error != NULL)
            {
                JSON_Value *root_v = json_parse_string(myError);
                JSON_Object *root_o = json_value_get_object(root_v);
                char messageError[BUFLEN];
                memcpy(messageError, json_object_get_string(root_o, "error"), BUFLEN);
                if (strcmp(messageError, "Authorization header is missing!") == 0)
                {
                    fprintf(stderr, "Nu aveti access la biblioteca! Verificati daca sunteti autentificat sau daca ati intrat in biblioteca.\n");
                }
                else
                {
                    fprintf(stderr, "%s\n", messageError);
                }
                json_value_free(root_v);
            }
            //the server sent a list of books (JSON Objects)
            else if (myError != NULL && error == NULL)
            {

                JSON_Value *root_v = json_parse_string(myError - 1);
                JSON_Array *commits = json_value_get_array(root_v);
                JSON_Object *commit;
                int count = json_array_get_count(commits);

                for (int i = 0; i < count; i++)
                {
                    commit = json_array_get_object(commits, i);
                    printf("Book ID: %d, Book Title: %s\n", (int)json_object_get_number(commit, "id"), json_object_get_string(commit, "title"));
                }
            }
        }
        else if (strncmp(buffer, "get_book", 8) == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            //If the user was not granted the access to the library
            /*if (JWToken == NULL)
            {
                fprintf(stderr, "Nu aveti access la biblioteca!\n");
                continue;
            }*/

            //Building the URL, depending on the ID
            char url[BUFLEN];
            char idstring[BUFLEN];
            int id;
            printf("id=");
            scanf("%s", idstring);
            id = atoi(idstring);
            if (id == 0)
            {
                fprintf(stderr, "ID invalid! ID-ul tastat trebuie sa contina numai cifre, nu alte caractere.\n");
                continue;
            }
            sprintf(url, "/api/v1/tema/library/books/%d", id);
            //printf("%s\n", url);

            message = compute_get_request(HOST, url, NULL, cookies, 1, JWToken);
            //printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char *myError = basic_extract_json_response(response);
            char *error;

            //Looking for errors
            if (myError != NULL)
            {
                error = strstr(myError, "error");
            }

            if (error != NULL)
            {
                JSON_Value *root_v = json_parse_string(myError);
                JSON_Object *root_o = json_value_get_object(root_v);
                char messageError[BUFLEN];
                memcpy(messageError, json_object_get_string(root_o, "error"), BUFLEN);
                if (strcmp(messageError, "Error when decoding tokenn!") == 0)
                {
                    fprintf(stderr, "Nu aveti access la biblioteca! Eroare de la get_book.\n");
                }
                else if (strcmp(messageError, "Authorization header is missing!") == 0)
                {
                    fprintf(stderr, "Nu sunteti autentificat sau nu ati intrat in biblioteca!\n");
                }

                else if (strcmp(messageError, "No book was found!") == 0)
                {
                    fprintf(stderr, "ID invalid! Cartea cu acest ID nu exista!\n");
                }
                else if (strcmp(messageError, "id is not int!") == 0)
                {
                    fprintf(stderr, "ID invalid! ID-ul tastat trebuie sa contina numai cifre, nu alte caractere.\n");
                }
                else
                {
                    fprintf(stderr, "%s\n", messageError);
                }
                json_value_free(root_v);
            }
            else
            {
                // JSON Object
                JSON_Value *root_v = json_parse_string(myError - 1);
                JSON_Array *commits = json_value_get_array(root_v);
                JSON_Object *commit = json_array_get_object(commits, 0);
                printf("ID: %d, Title: %s, Author: %s, Publisher: %s, Genre: %s, Page_Count:%d\n", id, json_object_get_string(commit, "title"),
                       json_object_get_string(commit, "author"), json_object_get_string(commit, "publisher"), json_object_get_string(commit, "genre"), (int)json_object_get_number(commit, "page_count"));
            }
        }
        else if (strncmp(buffer, "add_book", 8) == 0)
        {

            char title[BUFLEN], author[BUFLEN], genre[BUFLEN], publisher[BUFLEN];
            int page_count;
            char pagestring[BUFLEN];
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            //If the user was not granted the access to the library
            /*if (JWToken == NULL)
            {
                fprintf(stderr, "Nu aveti access la biblioteca!\n");
                continue;
            }*/

            //Creating prompt and reading user input
            printf("title=");
            fgets(title, BUFLEN, stdin);
            title[strlen(title) - 1] = '\0';

            printf("author=");
            fgets(author, BUFLEN, stdin);
            author[strlen(author) - 1] = '\0';

            printf("genre=");
            scanf("%s", genre);

            printf("publisher=");
            scanf("%s", publisher);

            printf("page_count=");
            scanf("%s", pagestring);
            page_count = atoi(pagestring);

            if (title[0] == ' ' || author[0] == ' ' || strlen(title) == 0 || strlen(author) == 0)
            {
                fprintf(stderr, "Titlu sau autor invalide! Acestea trebuie sa aiba un nume si nu pot incepe cu spatiu.\n");
                continue;
            }
            if (genre[0] == ' ' || publisher[0] == ' ' || strlen(genre) == 0 || strlen(publisher) == 0)
            {
                fprintf(stderr, "Genre sau publisher invalide! Acestea trebuie sa aiba un nume si nu pot incepe cu spatiu.\n");
                continue;
            }
            if (page_count == 0)
            {
                fprintf(stderr, "Page_count invalid! Page_count-ul trebuie sa contina numai cifre, nu alte caractere si sa fie > 0.\n");
                continue;
            }

            //Creating JSON Object and converting it to JSON String
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            char *serialized_string = NULL;
            json_object_set_string(root_object, "title", title);
            json_object_set_string(root_object, "author", author);
            json_object_set_string(root_object, "genre", genre);
            json_object_set_number(root_object, "page_count", page_count);
            json_object_set_string(root_object, "publisher", publisher);
            serialized_string = json_serialize_to_string_pretty(root_value);

            message = compute_post_request(HOST, BOOKS, PAYLOADTYPE, serialized_string, cookies, 1, JWToken);
            //printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            //printf("%s\n", response);
            char *myError = basic_extract_json_response(response);
            char *error;
            //Looking for errors
            if (myError != NULL)
            {
                error = strstr(myError, "error");
            }

            if (error != NULL)
            {
                JSON_Value *root_v = json_parse_string(myError);
                JSON_Object *root_o = json_value_get_object(root_v);
                char messageError[BUFLEN];
                memcpy(messageError, json_object_get_string(root_o, "error"), BUFLEN);
                if (strcmp(messageError, "Error when decoding tokenn!") == 0)
                {
                    fprintf(stderr, "Nu aveti access la biblioteca! Eroare de la add_book.\n");
                }
                else if (strcmp(messageError, "Something Bad Happened") == 0)
                {
                    fprintf(stderr, "Page_count invalid! Page_count-ul trebuie sa contina numai cifre, nu alte caractere.\n");
                }
                else if (strcmp(messageError, "Authorization header is missing!") == 0)
                {
                    fprintf(stderr, "Verificati daca sunteti autentificat si daca ati intrat in biblioteca!\n");
                }
                else
                {
                    fprintf(stderr, "%s\n", messageError);
                }
            }
            else
            {
                printf("Cartea a fost adaugata!\n");
            }
        }
        else if (strncmp(buffer, "delete_book", 11) == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            //If the user was not granted the access to the library
            /*if (JWToken == NULL)
            {
                fprintf(stderr, "Nu aveti access la biblioteca!\n");
                continue;
            }*/

            //Building the URL, depending on the ID
            char url[BUFLEN];
            char idstring[BUFLEN];
            int id;
            printf("id=");
            scanf("%s", idstring);
            id = atoi(idstring);
            if (id == 0)
            {
                fprintf(stderr, "ID invalid! ID-ul tastat trebuie sa contina numai cifre, nu alte caractere.\n");
                continue;
            }
            sprintf(url, "/api/v1/tema/library/books/%d", id);
            message = compute_delete_request(HOST, url, NULL, cookies, 1, JWToken);
            //printf("%s\n", message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            //printf("%s\n", response);
            char *myError = basic_extract_json_response(response);
            char *error;

            //Looking for errors
            if (myError != NULL)
            {
                error = strstr(myError, "error");
            }

            if (error != NULL)
            {
                JSON_Value *root_v = json_parse_string(myError);
                JSON_Object *root_o = json_value_get_object(root_v);
                char messageError[BUFLEN];
                memcpy(messageError, json_object_get_string(root_o, "error"), BUFLEN);
                if (strcmp(messageError, "Error when decoding tokenn!") == 0)
                {
                    fprintf(stderr, "Nu aveti access la biblioteca! Eroare de la get_book.\n");
                }

                else if (strcmp(messageError, "No book was deleted!") == 0)
                {
                    fprintf(stderr, "ID invalid! Cartea cu acest ID nu exista, deci nu poate fi stearsa!\n");
                }
                else if (strcmp(messageError, "id is not int!") == 0)
                {
                    fprintf(stderr, "ID invalid! ID-ul tastat trebuie sa contina numai cifre, nu alte caractere.\n");
                }
                else if (strcmp(messageError, "Authorization header is missing!") == 0)
                {
                    fprintf(stderr, "Verificati daca sunteti autentificat si daca ati intrat in biblioteca!\n");
                }
                else
                {
                    fprintf(stderr, "%s\n", messageError);
                }
                json_value_free(root_v);
            }
            else
            {
                printf("Cartea a fost stearsa!\n");
            }
        }
        else if (strncmp(buffer, "logout", 6) == 0)
        {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            //If the user is not logedin
            /*if (!loged)
            {
                fprintf(stderr, "Nu sunteti autentificat, deci nu aveti access la biblioteca!\n");
                continue;
            }*/

            message = compute_get_request(HOST, LOGOUT, NULL, cookies, 1, JWToken);
            send_to_server(sockfd, message);
            //printf("%s\n", message);
            response = receive_from_server(sockfd);
            //printf("%s\n", response);

            //Extract the JSON
            char *myError = basic_extract_json_response(response);
            char *error;

            //Looking for errors
            if (myError != NULL)
            {
                error = strstr(myError, "error");
            }

            if (error != NULL)
            {
                JSON_Value *root_v = json_parse_string(myError);
                JSON_Object *root_o = json_value_get_object(root_v);
                char messageError[BUFLEN];
                memcpy(messageError, json_object_get_string(root_o, "error"), BUFLEN);
                if (strcmp(messageError, "You are not logged in!") == 0)
                {
                    fprintf(stderr, "Nu sunteti autentificat, deci nu puteti face logout!\n");
                }
                else
                {
                    fprintf(stderr, "%s\n", messageError);
                }
                json_value_free(root_v);
            }
            else
            {
                loged = false;
                free(JWToken);
                JWToken = NULL;
                free(cookies[0]);
                free(cookies);
                cookies = NULL;
                printf("V-ati deconectat cu success!\n");
                close_connection(sockfd);
            }
        }
    }

    close_connection(sockfd);
    return 0;
}
