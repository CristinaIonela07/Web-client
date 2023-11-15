# Web-client
In client.c exista doua variabile booleene care retin daca exista un utilizator conectat
(log_in) sau daca utilizatorul are acces la biblioteca(in_lib). Exista de 
asemenea un string token_jwt si un vector de stringuri cookies pentru a
retine cookiul de logare si token ul de acces la biblioteca.

Daca se doreste inregistraea unui nou cont, se verifica daca datele introduse
contin spatii, caz in care se genereaza eroare. ALtfel se realizeaza
serializarea si trimiterea cererii catre server. Raspunsul este apoi analizat
pentru a verifica daca s-a realizat intradevar inregistrarea noului utilizator.

Similar pentru autentificare, cu precizarea ca se verifica a priori daca
exista deja un utilizator conectat, caz care este de asemenea tratat ca eroare
(am ales sa consider ca nu se poate conecta al doilea utilizator pana ce primul
nu se deconecteaza). Raspunsul este analizat si se retine cookie-ul folosit
la conectare.

Pentru enter_library trebuie ca utilizatorul sa fie logat. Se extrage din
raspunsul primit de la server token ul specific, care va fi folosit pentru
urmatoarele actiuni din biblioteca.

Actiunile get_books, get_book, add_book si delete_book pot avea loc doar daca
utilizatorul este conectat si se afla in biblioteca. In caz contrar se 
genereaza mesaj de eroare.

Pentru add_book se citesc informatiile necesare de la utilizator si se 
verifica corectitudinea lor: se verifica ca nu cumva utilizatorul sa fi lasat
un camp necompletat, caz care genereaza eroare. Se verifica de asemenea ca 
numarul de pagini al cartii sa fie un numar (similar, pentru get_book si 
delete_book se verifica caa id-ul introdus de utilizator sa fie numar).

Functiile au fost construite conform indicatiilor din cerinta, tratand
toate exceptiile si mesajele de eroare.
