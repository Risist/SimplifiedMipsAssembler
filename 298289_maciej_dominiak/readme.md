Dane do programu powinny byæ wprowadzane poprzez standardowe wejœcie.

program przyjmuje instrukcje w oddzielnych liniach
mo¿liwe jest u¿ywanie programu iteratywnie t.j. wprowadzaj¹c ka¿d¹ instrukcjê oddzielnie
i otrzymuj¹c w wyniku poœrednie wyniki.

w zadaniu drugim pojawi³y siê lekkie kontrowersje dotycz¹ce dyrektywy .org i jej relacji z .text i .data
W napisanym programie za³o¿y³em, ¿e .org mo¿e pojawiæ siê tylko przed sekcj¹ i smienia lokalizacjê pocz¹tku nastêpnej sekcji
jeœli .org bêdzie wywo³ane kilkukrotnie na tej samej sekcji zachowanie jest niezdefiniowane

Adresy sekcji s¹ ustawione jeden za drugim, z ofsetem zale¿nym od koñca poprzedniej sekcji.
jeœli kolejnoœæ sekcji jest zmieniana (t.j .data ma mniejszy adress ni¿ .text ) 
.org powinno byæ u¿yte na obydwu sekcjach

w przypadku dyrektyw które nie zapisuj¹ ¿adnych danych wyœwietlanajest sama instrukcja bez adresu

W zadaniu trzecim w przypadku wyœwietlania symtab i .rela zamiast numerów s¹ wyœwietlane nazwy sekcji,
Powody s¹ dwa, po pierwsze ³atwiejsza implementacja, po drugie bardziej czytelny rezultat. 
(zak³adam, ¿e owe informacje bêd¹ wykorzystywane g³ównie do debugowania )

jeœli tag nie zosta³ jeszcze ustalony (co mo¿e siê zdarzyæ podczas korzystania z programu jak interpreter - patrz wy¿ej)
do programu zostaje wstawiony tag z domyœlnymi danymi. 
Po wpisaniu instrukcji z tagiem odpowiadaj¹cym tagowi u¿ywanego wczeœniej
program wykonuje relokacjê w wczeœniej zdefiniowanych miejscach.

w przypadku typu relokacji "MIPS_32: argument dyrektywy «.word»" pojawi³y siê pewne kontrowersje, 
których nie uda³o mi siê rozwiaæ czytaj¹c dokumentacjê. Zak³adam, i¿ w tym typie relokacji chodzi o to,
aby by³o mo¿liwym w dyrektywie ".word" jako argument podaæ tag

