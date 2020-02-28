VLAD ANDREI-ALEXANDRU
321CB

================================= README =================================
================================ TEMA1 PC ================================

	Pentru implementarea acestei teme am folosit scheletul din laboratorul
cu fereastra glisanta impreuna cu modificari majore.

	SEND
=============================

	In acest fisier preiau parametrii programului : nume fisier, SPEED si
DELAY.
	TIMEOUT- ul am ales sa fie 2 deoarece programul este facut sa lucreze
foarte rapid, iar fereastra nu va fi goala pe parcursul transmiterii.
Valoarea 2 este valoarea care scoate cea mai mare viteza.
	Dupa ce deschid fisierul pentru citire calculez BDP-ul, numarul de 
octeti ai fisierului (ma deplasez cu lseek pana la final si spune cati 
octeti a parcurs, apoi mut cursorul virtual din fisier la 0), numarul
de pachete (dimensiune fisier / dimensiunea payload-ului meu (+1 daca 
nu se imparte fix)) si dimensiunea ferestrei (folosind formula din
laborator).
	Dupa toate calculele aloc un vector de pointeri catre structuri de
mesaje ("messages"), un vector "sent" in care retin mesajele confirmate
de recv.
	Pe pozitia 0 din vector retin mesajul cu numele fisierului si COUNT-ul.
	Imediat dupa ce scriu structura mesajului in vector o trimit pentru a 
nu irosi timp (pentru primele w mesaje). De la mesajul 2 incolo doar le
retin in vector.
	Dupa acestea am un while(1) in care astept ACK. Daca primesc mesaj
marchez mesajul cu ACK in vectorul sent.
	Daca primesc un mesaj de tipul T_FINISH sau T_RECEIVED atunci opresc
while-ul si astept ultimul mesaj de confirmare ca s-a terminat de scris
fisierul in RECV.
	Indiferent ca am primit ACK sau nu (a expirat timeout-ul sau alta 
eroare) trimit mesaje in continuare. Decizia urmatorului se face pe baza
unor limite inferioare si superioare:
	lastSent retine ultimul mesaj neconfirmat din partea stanga (0->)
	fromEnd retine ultimul mesaj neconfirmat din partea dreapta (<-COUNT);
	Intre cele doua variabile se afla next care se incrementeaza la
fiecare mesaj trimis (nu confirmat). Cand next iese din cele doua limite
il aduc inapoi in interval. La fiecare tura de while lastSent si fromEnd
se deplaseaza pe mesajele neconfirmate cele mai apropiate (se observa din
cod metoda). Aceasta metoda reduce complexitatea cautarii si accelereaza
trimiterea mesajelor in ordine.
	Cand am ajuns cu lastSent == fromEnd trimit si ultimul mesaj si astept
ca recv sa termine de scris mesajele in fisier.
	Intre timp eliberez memoria cu mesajele, vectorul de pointeri si
vectorul sent (cel care tine cont de ce mesaje au fost trimise) si inchid
fisierul. Cand mesajul de GATA de la recv a ajuns SEND se termina.



	RECV
=============================
	In acest fisier pregatesc variabilele pentru numele fisierului,
COUNT-ul ce trebuie sa soseasca din retea, numele fisierului nou si
file descriptorul pentru fisier.
	Aici aloc un vector de mesaje (nu pointeri catre mesaje) de
maxim 10k (in caz ca se vor trimite si fisiere de 10MB >)
si astept intr-o bucla while(1) primirea mesajelor din retea.
	Cand un mesaj a sosit verific daca e corupt. Daca da, atunci il
arunc pe jos (nu ii fac nimic, iar send il va retrimite la TIMEOUT
expirat). Pentru mesajul primit trimit ACK cu acelasi incex cu care
a venit. Apoi verific daca acest mesaj este prima data cand ajunge.
Daca a mai ajuns atunci il arunc pe jos (nu ii fac nimic). Daca
e unic atunci il copiez in vector, il marchez ca fiind primit
(setez campul len al mesajului salvat in vector = 1 (a.k.a RECEIVED)).
	Verific daca a ajuns mesajul 0, adica cel cu numele fisierului
si COUNT-ul. Daca da atunci salvez informatia si deschid fisierul si
marchez flag-ul fileIsOpened pentru a stii sa ma apuc de scris in fisier
odata ce se poate.
	Imediat ce prelucrez mesajul verific daca un counter este egal cu COUNT,
adica daca am primit toate mesajele. (COUNT la inceput este o valoare mare,
iar counter = 0, acesta fiind incrementat la fiecare mesaj unic primit)
	Daca flag-ul fileIsOpened scriu in fisier pachetele la rand de la
indexul 0, iar o variabila lastWritten se incrementeaza pentru fiecare
pachet scris.
	Dupa ce ies din aceasta bucla de primire a mesajelor continui sa scriu
mesajele de unde a ramas lastWritten pana la final.
	Apoi eliberez memoria (vectorul de mesaje), inchid fisierul si anunt
SENDer-ul ca am terminat de scris. 
	

	PACK
=============================
	Acest fisier "header" contine toate functiile ajutatoare pentru
initializarea mesajelor, citirea din fisier in structuri msj, 
calculul numarului de pachete necesar, functie pentru ACK pentru
indexul x si cele doua functii pentru corupere: crc (face xor pe 
buffer-ul dat de lungime len) si crcVerify care verifica toate 
campurile din structura mea daca s-au corupt sau nu.
	Structura mea este de forma :
	len - cat am citit din fisier
	index - ce bucata din fisier este
	type - ce tip de mesaj / ACK este
	buffer - aici vor fi octetii din fisier
	+ checksum pentru fiecare dintre campurile de mai sus


Pe checker-ul local obtin 85 de puncte.
