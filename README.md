# Client Web. Comunicatie cu REST API

CUPRINS
-----------------------------
- Introducere
- Realizarea temei

INTRODUCERE
------------------------------
Din punctul meu de vedere, tema a avut un nivel de dificultate usor - mediu. Durata de
rezolvare a temei a fost de aproximativ 12 ore. Consider ca mi-a fost de folos
aceasta tema deoarece am putut intelege mai bine informatiile si exercitiile de
la laborator legate de protocolul HTTP si aplicatia client-server web.

REALIZAREA TEMEI
--------------------------------
Precizez ca o mare parte din scheletul temei este preluat din rezolvarea laboratorului
10, publicata pe OCW. Pentru utilizarea JSON, am folosit Parson, asa cum a fost 
recomandat in enuntul temei, pentru limbajul C (https://github.com/kgabis/parson).
Multe dintre erorile intoarse de server au fost tratate individual, pentru a afisa mesaje
mult mai sugestive pentru utilizator. Totusi, in cazul in care nu am descoperit eu toate
erorile posibile intoarse de server, afisez acea eroare.
Voi prezenta pasii pentru fiecare comanda in parte:

1) exit (inchide clientul)

2) register (inregistrarea unui cont)
Verific daca acel cont este deja autentificat printr-o variabila loged de tip bool, care
poate fi false daca acel cont nu este autentificat si true daca este.
Daca acel cont nu este autentificat, nu pot inregistra un alt cont. Afisez un mesaj
corespunzator si sar la urmatoarea comanda.
Altfel, creez un prompt si citesc username-ul si parola de la tastatura si verific daca acestea
nu contin spatii. Daca datele sunt valide, creez un obiect de tip JSON care contine fieldurile
username si password si il trimit ca un string serverului.
Verific mesajul de la server si caut erori, pe care le afisez. Daca nu exista, afisez un mesaj
pentru a instiinta userul ca acel cont s-a creat cu succes.

3) login (autentificarea unui cont)
Verific daca utilizatorul este deja logat, la fel ca la pasul precedent si afisez un mesaj,
apoi trec la comanda urmatoare. Altfel, citesc usernameul si parola si creez un obiect
de tip JSON pe care il trimit serverului ca un string. Primesc mesajul de la server si caut in el
posibile erori. Daca nu se gasesc, extrag cookie-ul de sesiune din mesaj si il adaug contului.

4) enter_library (accesarea bibliotecii)
Trimit un mesaj  catre server, cu adresa URL specificata in enunt. Primesc
mesajul de la server si caut posibile erori. Daca nu exista, imi extrag token-ul primit ca un 
obiect JSON si il adaug contului.

5) get_books (vizualizarea bibliotecii)
Trimit un mesaj catre server, cu adresa URL specificata in enunt. Primesc
mesajul de la server si verific daca acesta mi-a trimis o lista goala de carti, caz in care
afisez un mesaj si trec la comanda urmatoare. Altfel, caut posibile erori, si daca nu
exista, afisez lista de carti primita de la server.

6) get_book (vizualizarea detaliilor despre o carte)
Citesc id-ul si verific daca acesta contine numai cifre. Trimit mesajul catre server numai
daca acesta contine cifre, si in urma raspunsului primit de la server, verific daca am erori
sau daca am primit cartea ceruta. Precizez ca enuntul temei nu a fost tocmai corect la acest pas,
pentru ca serverul trimite un array de un element (obiect JSON), nu un obiect JSON. De asemenea,
in acest obiect nu exista field-ul id, asa ca l-am afisat pe cel citit.

7) add_book (adaugarea unei carti in biblioteca)
Citesc detaliile cartii, iar in cazul in care vreunul incepe cu spatiu, afisez eroare. Altfel, 
se creeaza un obiect JSON si este trimis ca string serverului. Primesc mesajul de la server si
verific daca am erori, altfel afisez un mesaj pentru a instiinta utilizator ca acea carte a fost
adaugata.

8) delete_book (stergerea unei carti din biblioteca)
Citesc id-ul si verific daca acesta contine numai cifre. Trimit mesajul catre server numai
daca acesta contine cifre, si in urma raspunsului primit de la server, verific daca am erori.
In caz negativ, afisez un mesaj pentru a instiinta utilizatorul ca acea carte a fost stearsa.

9) logout (deconectare) 
Trimit un mesaj catre server, cu adresa URL specificata in enunt. Primesc
mesajul de la server si verific daca in acesta exista erori. In caz negativ,
afisez un mesaj pentru a instiinta utilizatorul ca s-a deconectat si ii setez statusul la
loged = false si eliberez memoria pentru cookies si token.

Toata memoria alocata este eliberata atat la finalul programului, cat si la fiecare logout.

Idei importante (care se inteleg mai bine din mesajele trimise la stderr si stdout):
- cookie-ul este primit in urma login-ului
- token-ul este primit in urma accesarii bibliotecii
- toate comenzile tratate dupa login functioneaza numai daca cookie-ul exista (adica
daca utilizatorul este logat) si toate comenzile tratate dupa enter_library functioneaza
numai daca token-ul exista (adica utilizatorul a intrat, accessat biblioteca).
Desi nu erau specificate in enunt, am tratat mai multe erori la fiecare comanda.
Spre exemplu, daca un user este deja logat, nu se mai putea loga inca o data (loginuri
imbricate) si nu mai putea inregistra alte conturi. 
In enuntul temei, nu era specificat in ce fel trebuie sa demonstram ca avem acces la biblioteca
sau ca suntem autentificati, asa ca am tratat aceasta problema in 2 cazuri.
1. In urma mesajelor primite de la server, am afisat erori.
2. Inainte de "conceperea" mesajelor si de a trimite ceva la server, am verificat daca userul este
logat (prin bool loged) sau daca user-ul are un token. (acest caz este comentat).
Am citit forumul si au existat mai multe opinii, asa ca am preferat sa tratez ambele metode.
