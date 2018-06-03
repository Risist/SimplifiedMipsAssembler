Dane do programu powinny by� wprowadzane poprzez standardowe wej�cie.

program przyjmuje instrukcje w oddzielnych liniach
mo�liwe jest u�ywanie programu iteratywnie t.j. wprowadzaj�c ka�d� instrukcj� oddzielnie
i otrzymuj�c w wyniku po�rednie wyniki.

w zadaniu drugim pojawi�y si� lekkie kontrowersje dotycz�ce dyrektywy .org i jej relacji z .text i .data
W napisanym programie za�o�y�em, �e .org mo�e pojawi� si� tylko przed sekcj� i smienia lokalizacj� pocz�tku nast�pnej sekcji
je�li .org b�dzie wywo�ane kilkukrotnie na tej samej sekcji zachowanie jest niezdefiniowane

Adresy sekcji s� ustawione jeden za drugim, z ofsetem zale�nym od ko�ca poprzedniej sekcji.
je�li kolejno�� sekcji jest zmieniana (t.j .data ma mniejszy adress ni� .text ) 
.org powinno by� u�yte na obydwu sekcjach

w przypadku dyrektyw kt�re nie zapisuj� �adnych danych wy�wietlanajest sama instrukcja bez adresu

W zadaniu trzecim w przypadku wy�wietlania symtab i .rela zamiast numer�w s� wy�wietlane nazwy sekcji,
Powody s� dwa, po pierwsze �atwiejsza implementacja, po drugie bardziej czytelny rezultat. 
(zak�adam, �e owe informacje b�d� wykorzystywane g��wnie do debugowania )

je�li tag nie zosta� jeszcze ustalony (co mo�e si� zdarzy� podczas korzystania z programu jak interpreter - patrz wy�ej)
do programu zostaje wstawiony tag z domy�lnymi danymi. 
Po wpisaniu instrukcji z tagiem odpowiadaj�cym tagowi u�ywanego wcze�niej
program wykonuje relokacj� w wcze�niej zdefiniowanych miejscach.

w przypadku typu relokacji "MIPS_32: argument dyrektywy �.word�" pojawi�y si� pewne kontrowersje, 
kt�rych nie uda�o mi si� rozwia� czytaj�c dokumentacj�. Zak�adam, i� w tym typie relokacji chodzi o to,
aby by�o mo�liwym w dyrektywie ".word" jako argument poda� tag

