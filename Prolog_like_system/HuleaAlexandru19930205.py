
# coding: utf-8

# # Tema 2 - Motor de rațioament logic (MiniLog)
#   - Andrei Olaru
# 

# In[100]:

from operator import add


# In[101]:

from testing_a import *


# # Partea 0: Funcții utile din testing.py
# 
# În `testing.py` se găsesc funcțiile utile:
# 
# * `print_formula` (pentru orice element de formulă logică), `print_subst` (pentru o substituție), `print_answer` (pentru o listă de substituție / răspuns la o interogare) și `print_KB` (pentru o bază de cunoștințe).
#   * primele 3 dintre funcții au un al doilea argument opțional boolean care, dacă este `True`, provoacă întoarcerea rezultatului (și nu afișarea lui)
# 
# * `check_sentence` -- care verifică posibilele erori dintr-o formulă)
# * `make_statement` și `add_statement` -- pentru construcția unei afirmații (implicative) și adăugarea unei informații la baza de cunștințe (cu verificare prin `check_sentence`).
# * `gather_vars` -- strânge într-o listă toate numele de variabile dintr-o propoziție (poate conține duplicate).
# * `make_unique_var_names` -- redenumește variabilele dintr-o bază de cunoștințe (există și variantă pentur o singură propoziție) în așa fel încât un același nume de variabilă să nu apară în mai multe propoziții. Baza de cunoștințe este modificată în această funcție.
# 
# * `substitute(f, subst)` -- aplică substituția `subst` asupra formulei `f`. Dacă `replace_args` întoarce o copie a formulei, atunci și `substitute` întoarce tot o copie.

# # Partea I: Rezolvarea pentru Laboratorul 5
# 
# ## Cerința 0
# 
# Includeți elementele de rezolvare pentru laboratorul 5 de IA, și anume reprezentarea formulelor logice (în `reprezentare.py` și funcțiile `check_occur` și `unify` (aici mai jos).
# 
# * verificați că `make_and` și `make_or` funcționează și pe mai mult de 2 propoziții (cu funcționare corespunzătoare pentru `get_args` și `replace_args`.
# * în funcția `unify`, inițializați substituția pe care lucrați cu una primită ca parametru (vedeți semnătură mai jos). Acest lucru vă poate fi util în implementarea motorului de raționament.

# ## Reprezentare
# 
# În LPOI trebuie să putem reprezenta următoarele elemente:
# * _termen_ -- poate fi luat ca argument de către un predicat. Un termen poate fi:
#   * o constantă -- are o valoare
#   * o variabilă -- are un nume și poate fi legată la o valoare
#   * un apel de funcție -- are numele funcției și argumentele (e.g. add[1, 2, 3]). Se evaluează la o valoare. Argumentele funcției sunt tot termeni.
#     * Notă: În text vom scrie apelurile de funcții cu paranteze drepte, pentru a le deosebi de atomi.
# * _formulă (propoziție) logică_ -- se poate evalua la o valoare de adevăr, într-o interpretare (o anumită legare a numelor la o semantică). O formulă poate fi:
#   * un atom -- aplicarea unui predicat (cu un nume) peste o serie de termeni (argumentele sale)
#   * negarea unei formule
#   * un conector logic între două sau mai multe propoziții -- conjuncție sau disjuncție
#   
# 
# ### Cerință reprezentare
# 
# Creați o reprezentare internă pentru formule logice. Pentru această reprezentare, vom avea:
# * o serie de funcții care o construiesc -- `make_*` și `replace_args`
# * o serie de funcții care o verifică -- `is_*`
# * o serie de funcții care accesează elementele din reprezentare -- `get_*`
# 
# **Important:** Pentru a lucra mai ușor cu formulele, vom considera că pentru apelurile de funcții și pentru toate formulele (atât atomi cât și propoziții compuse), reprezentarea are un _head_ (după caz, numele funcției, numele predicatului, sau conectorul logic) și o listă de argumente (după caz, lista de argumente a funcției, lista de argumente a predicatului, o listă cu propoziția negată, sau lista de propoziții unite de conectorul logic (2 sau mai multe)).

# In[102]:

from HuleaAlexandru19930205_R import *


# ## Unificare
# 
# ### Cerință unificare
# 
# Trebuie să aveți implementate funcțiile `occur_check` și `unify`. Funcția `substitute` este deja implementată în `testing.py`.
# 
# **Nu uitați** că o substituție este considerată ca un dicționar având drept chei **nume** de variabile și drept valori termeni.
# 
# Hint: vă poate fi util să suportați ca `unify` să primească și o substituție deja existentă, de la care să pornească. Dacă folosiți argumente opționale, fiți atenți la utilizarea lor.
# 
# Hint 2: pentru problema din test2.txt, va fi necesar ca funcțiile care pot fi evaluate (toate variabilele sunt instanțiate) să fie evaluate înainte de a realiza unificarea cu o constantă.

# In[103]:

# Verifică dacă variabila v poate fi înlocuită cu termenul t, având în vedere substituția subst.
# Întoarce True dacă v poate fi înlocuită cu t, și False altfel.
# Verificarea eșuează dacă, având în vedere și substituția, variabila v apare în 
#  termenul t (deci înlocuirea lui v ar fi un proces infinit).
def occur_check(v, t, subst, top = True):
    if (is_constant(t)):
        return True
    elif (is_variable(t)):
        if get_name(t) in subst:
            return occur_check(v, substitute(t, subst), subst)
        if (get_name(v) == get_name(t)):
            return False
        else:
            return True
    elif (is_function_call(t)):
        args = get_args(t)
        for arg in args:
            if not (occur_check(v, arg, subst)):
                return False
        return True
    return False

# Unifică formulele f1 și f2, considerând substituția deja existentă subst.
# Rezultatul unificării este o substituție (dicționar nume-variabilă -> termen),
#  astfel încât dacă se aplică substituția celor două formule, rezultatul este identic.
def unify(f1, f2, sigma = None):
    if not sigma:
        sigma = {}
    stack = [(f1, f2)]
    while (len(stack) != 0):
        (s, t) = stack.pop()
        while (get_name(s) in sigma):
            s = substitute(s, sigma)
        while (get_name(t) in sigma):
            t = substitute(t, sigma) 
        if (s != t):
            if (is_variable(s)):
                if (occur_check(s, t, sigma)):
                    sigma[get_name(s)] = t
                else:
                    return False
            elif (is_variable(t)):
                if (occur_check(t, s, sigma)):
                    sigma[get_name(t)] = s
                else:
                    return False
            elif (has_args(s) and has_args(t) and get_head(s) == get_head(t)):
                args_s = get_args(s)
                args_t = get_args(t)
                if (len(args_s) == len(args_t)):
                    for i in range(len(args_s)):
                        stack.append((args_s[i], args_t[i]))
                else:
                    return False
            else:
                return False
    return sigma


# ## Testare pentru lucrurile deja implementate

# In[104]:

# Test representation of logical formulas
    
new_tests()
test(is_term(make_const('A')), True)
test(is_term(make_var('x')), True)
test(is_atom(make_var('x')), False)
test(is_term(make_function_call(add, make_const(2), make_var('x'))), True)
test(is_sentence(make_function_call(add, make_const(2), make_var('x'))), False)
#5
test(is_atom(make_atom('P', make_var('x'))), True)
test(is_term(make_atom('P', make_var('x'))), False)
test(is_sentence(make_neg(make_atom('P', make_var('x')))), True)
test(get_value(make_const(2)), 2)
test(get_name(make_var('X')), 'X')
#10
test(get_name(make_atom('P')), None)
test(get_head(make_atom('P')), 'P')
test(get_head(make_function_call(add, make_const(2), make_var('x'))), str(add))
test(get_args(make_atom('P')), [])
#14
if get_args(make_atom('P', make_var('X'))):
    test(get_name(get_args(make_atom('P', make_var('X')))[0]), 'X')
else:
    test([], 'X')
#15
replaced = replace_args(make_and(make_atom('P', make_var('x')), make_atom('Q', make_var('x'))), 
                       [make_atom('P', make_const(1)), make_atom('Q', make_const(1))])
if isinstance(get_args(replaced), list) and get_args(replaced):
    test(get_value(get_args(get_args(replaced)[0])[0]), 1)
else:
    test([], 1)


# In[105]:

# Testare conectori cu mai multe propoziții și replace_args()
f = make_or(*L1)
print_formula(f) # trebuie să dea ceva gen (V P() P() P() P())
new_tests()
test(get_args(f), L1)
f2 = replace_args(f, L2)
test(get_args(f), L1) # tests f did not change
test(get_args(f2), L2) # tests arguments changed ok


# In[106]:

# Test occur_check
test( occur_check(make_var('x'), make_const('y'), {}), True ) #0
test( occur_check(make_var('x'), make_var('y'), {}), True ) #1
test( occur_check(make_var('x'), make_function_call(add, make_var('x')), {}), False ) #2
test( occur_check(make_var('y'), make_function_call(add, make_var('x')), {'x': make_var('y')}), False ) #3
test( occur_check(make_var('z'), make_function_call(add, make_var('x')), {'x': make_var('y'), 'y': make_var('z')}), False ) #4
test( occur_check(make_var('z'), make_function_call(add, make_const(5), make_function_call(add, make_var('x'))), {'x': make_var('y'), 'y': make_var('z')}), False ) #5
test( occur_check(make_var('z'), make_function_call(add, make_var('w')), {'x': make_var('y'), 'y': make_var('z')}), True ) #6
test( occur_check(make_var('x'), make_const('x'), {}), True ) #7


# In[107]:

# Test unify

new_tests()
#0 # P(B) vs P(A) -> False
test2( unify( make_atom('P', make_const('B')), make_atom('P', make_const('A'))), False)
#1 # P(x) vs P(A) -> x: A
test2( unify( make_atom('P', make_var('x')), make_atom('P', make_const('A'))), {'x': make_const('A')})
#2 # P(x, x) vs P(A, A) -> x: A
test2( unify( make_atom('P', make_var('x'), make_var('x')), make_atom('P', make_const('A'), make_const('A'))),
              {'x': make_const('A')} )
#3 # P(x, A) vs P(A, x) -> x: A
test2( unify( make_atom('P', make_var('x'), make_const('A')), make_atom('P', make_const('A'), make_var('x'))),
              {'x': make_const('A')} )
#4 # P(x, A, x) vs P(A, x, A) -> x: A
test2( unify( make_atom('P', make_var('x'), make_const('A'), make_var('x')), make_atom('P', make_const('A'), make_var('x'), make_const('A'))), {'x': make_const('A')} )
#5 # P(x) vs P(add[Z, 5]) -> x: add[Z, 5]
test2( unify( make_atom('P', make_var('x')), make_atom('P', make_function_call(add, make_var('Z'), make_const(5)))), {'x': make_function_call(add, make_var('Z'), make_const(5))} )
#6 # P(x, y, z) vs P(A, B, C) -> x: A, y: B, z: C
test2( unify( make_atom('P', make_var('x'), make_var('y'), make_var('z')), make_atom('P', make_const('A'), make_const('B'), make_const('C'))), {'z': make_const('C'), 'y': make_const('B'), 'x': make_const('A')} )
#7 # Q(2, 3, add[x, y]) vs Q(x, y, add[2, 3]) -> x: 2, y: 3
test2( unify( make_atom('Q', make_const(2), make_const(3), make_function_call(add, make_var('x'), make_var('y'))), make_atom('Q', make_var('x'), make_var('y'), make_function_call(add, make_const(2), make_const(3)))), {'x': make_const(2), 'y': make_const(3)} )
#8 # P(x, y) vs P(x, add[y, 2]) -> false
test2( unify( make_atom('P', make_var('x'), make_var('y')), make_atom('P', make_const('x'), make_function_call(add, make_var('y'), make_const(2)))), False )
#9 # P(B) vs Q(B) -> False
test2( unify( make_atom('P', make_const('B')), make_atom('Q', make_const('B'))), False)
#10 # P(x, x) vs P(A, B) -> False
test2( unify( make_atom('P', make_var('x'), make_var('x')), make_atom('P', make_const('A'), make_const('B'))), False)


# In[108]:

# Verificare construcție și afișare
formula1 = make_and(
    make_or(make_neg(make_atom("P", make_var("x"))), make_atom("Q", make_var("x")), make_atom("R")),
    make_atom("T", make_var("y"), make_function_call(add, make_const(1), make_const(2))))
# Ar trebui ca ieșirea să fie similară cu: (A (V ~P(?x) Q(?x)) T(?y, <built-in function add>[1,2]))
print("checking formula: " + print_formula(formula1, True))
new_tests()
test(check_sentence(formula1), True)


# # Partea II -- motorul de raționament

# ## Parsare
# ### Cerința 1
# 
# Implementați funcția `parse_file` care primește un nume de fișier și întoarce un tuplu cu cel puțin două elemente -- `KB` și `Queries`. Fișierul conține linii pe care se găsesc fie afirmații, fie interogări. Funcția trebuie să parseze afirmațiile și să le pună într-o bază de cunoștințe (inițial vidă), iar interogările să le pună în lista `Queries`.
# 
# Dacă doriți, pentru testare puteți parsa și liniile care conțin răspunsurile la interogări, dar nu este obligatoriu.
# 
# Pentru parsare vă pot fi utile funcțiile `strip()` și `split()` ale obiectelor de tip șir de caractere.
# 
# Este necesar ca constantele numerice să fie convertite la numere (e.g. folosind funcțiile `isnumeric()` și `int()`)
# 
# Mai multe detalii în enunțul temei.

# In[109]:

import re

# Testeaza daca linia este comentariu
def esteComentariu(line):
    lineCopy = line.strip()
    if (len(lineCopy) > 0 and lineCopy[0] == ':'):
        return True
    return False

# Testeaza daca linia este rezultat
def esteRezultat(line):
    lineCopy = line.strip()
    if (len(lineCopy) > 0 and lineCopy[0] == ':'):
        return True
    return False
    
# Testeaza daca termenul este o constanta si in caz afirmativ intoarce o constanta creata cu make_const
# - daca respecta expresia regulata din definitia constantei
def esteConstanta(termen):
    termenCopy = termen.strip()
    if (len(termenCopy) == 1 and termenCopy[0] == '0'):
        return make_const(0)
    if (termenCopy.isnumeric()):
        m = re.match("([1-9][0-9]*)", termenCopy)
        if (m):
            groups = m.groups()
            if (len(groups) == 1 and len(groups[0]) == len(termenCopy)):
                return make_const(int(termenCopy))
    m = re.match("([A-Za-z][A-Za-z0-9]*)", termenCopy)
    if (m):
        groups = m.groups()
        if (len(groups) == 1 and len(groups[0]) == len(termenCopy)):
            return make_const(termenCopy)
    return False

# Teste pentru constante
# print(esteConstanta("0"))
# print (esteConstanta("012"))
# print(esteConstanta("39505"))
# print(esteConstanta("ABC02"))
# print(esteConstanta("ABC02-"))

# Testeaza daca parametrul este nume si in caz afirmativ intoarce numele
# - daca respecta expresia regulata din definitia numelui
def esteNume(termen):
    termenCopy = termen.strip()
    m = re.match("([A-Za-z_]+)", termenCopy)
    if (m):
        groups = m.groups()
        if (len(groups) == 1 and len(groups[0]) == len(termenCopy)):
            return termenCopy
    return False

# Testeaza daca termenul este o variabila si in caz afirmativ intoarce o variabila creata cu make_var
def esteVariabila(termen):
    termenCopy = termen.strip()
    if (len(termenCopy) > 0 and termenCopy[0] == '?'):
        nume = esteNume(termenCopy[1:])
        if (nume):
            return make_var(nume)
    return False

# Teste pentru variabile
# print(esteNume("Alex"))
# print(esteNume("0Alex"))
# print(esteVariabila("?Alex"))
# print(esteVariabila("?0Alex"))

# Testeaza daca termenul este o functie si in caz afirmativ intoarce o functie creata cu make_function_call
# Ceea ce se afla intre parantezele functiei este tradus in LPOI si dat ca parametru / parametri functiei 
def esteFunctie(termen):
    termenCopy = termen.strip()
    first = termenCopy.find('(')
    last = termenCopy.rfind(')')
    nume = esteNume(termenCopy[0:first])
    if not (nume):
        return False
    termenInauntru = termenCopy[first + 1 : last]
    if (len(termenInauntru.strip()) == 0):
        return make_function_call(nume)
    termeni = suntTermeni(termenInauntru)
    if (termeni):
        return make_function_call(nume, *termeni)
    return False
    
# Testeaza daca termenul este constanta, variabila, functie si intoarce constructia conform 
# cu regulile de creare din fisierul _R
def esteTermen(termen):
    termen = termen.strip()
    constanta = esteConstanta(termen)
    if (constanta):
        return constanta
    variabila = esteVariabila(termen)
    if (variabila):
        return variabila
    functie = esteFunctie(termen)
    if (functie):
        return functie
    return False

# In cazul mai multor termeni separati prin virgula, separ termenii de la primul nivel de imbricare - 
# adica avand #('(') - #(')') == 0 si transform fiecare termen cu functia esteTermen(termen)
# returnez o lista cu termeni creati dupa regulile din _R
def suntTermeni(termen):
    termen = termen.strip()
    termeni = []
    openBracket = 0
    start = 0
    for i in range(0, len(termen)):
        if (termen[i] == '('):
            openBracket += 1
            continue
        if (termen[i] == ')'):
            openBracket -= 1
            continue
        if (termen[i] == ',' and openBracket == 0):
            end = i
            termenNou = termen[start:end]
            termenNou = termenNou.strip()
            termeni.append(termenNou)
            start = i + 1
    termenNou = termen[start:]
    termenNou = termenNou.strip()
    termeni.append(termenNou)
    result = []
    for term in termeni:
        t = esteTermen(term)
        if (t):
            result.append(t)
        else:
            return False
    return result
    
# Teste pentru functii
# funct = esteFunctie("g(?a, ?b)")
# print(funct)
# for arg in get_args(funct):
#     print (arg)

# funct = esteFunctie("f(g(?a, ?b), hello)")
# print(funct)
# for arg in get_args(funct):
#     print (arg)  
    
# Testeaza daca parametrul este atom si in caz afirmativ returneaza un atom creat dupa regulile din _R
# Termenii dintre parantezele atomului sunt si ei transformati dupa regulile din _R si pasati ca parametri atomului
def esteAtom(chunk):
    chunkCopy = chunk.strip()
    first = chunkCopy.find('(')
    last = chunkCopy.rfind(')')
    nume = esteNume(chunkCopy[0:first])
    if not (nume):
        return False
    termenInauntru = chunkCopy[first + 1 : last]
    if (len(termenInauntru.strip()) == 0):
        return make_atom(nume)
    termeni = suntTermeni(termenInauntru)
    if (termeni):
        return make_atom(nume, *termeni)
    return False
    
# Test pentru atom
# atom = esteAtom("Q(f(g(?a, ?b), hello))")
# print (atom)

# Testeaza daca parametrul reprezinta o conditie - adica o lista de atomi separati prin virgula
# Returneaza o lista de atomi creati dupa regulile din _R
def esteConditie(chunk):
    chunk = chunk.strip()
    atomi = []
    openBracket = 0
    start = 0
    for i in range(0, len(chunk)):
        if (chunk[i] == '('):
            openBracket += 1
            continue
        if (chunk[i] == ')'):
            openBracket -= 1
            continue
        if (chunk[i] == ',' and openBracket == 0):
            end = i
            atomNou = chunk[start:end]
            atomNou = atomNou.strip()
            atomi.append(atomNou)
            start = i + 1
    atomNou = chunk[start:]
    atomNou = atomNou.strip()
    atomi.append(atomNou)
    result = []
    for atom in atomi:
        a = esteAtom(atom)
        if (a):
            result.append(a)
        else:
            return False
    return result

# Testeaza daca linia este o interogare si in caz afirmativ returneaza un atom sau o conjunctie de atomi
def esteInterogare(line):
    lineCopy = line.strip()
    if (len(lineCopy) > 0 and lineCopy[0] == '?'):
        conditie = esteConditie(lineCopy[1:])
        if not (conditie):
            return False
        if (len(conditie) == 1):
            return conditie[0]
        else:
            args = conditie[2:]
            return make_and(conditie[0], conditie[1], *args)
    return False

# Test pentru interogare
# print(esteInterogare("? P(?X)"))
# print(esteInterogare("? IWasLyingYesterday(Lion, ?today), IWasLyingYesterday(Unicorn, ?today)"))

# Testeaza daca linia este o afirmatie si in caz afirmativ returneaza o lista cu un atom daca afirmatia este un fapt
# sau o lista cu (un atom concluzie si o lista de premise)
def esteAfirmatie(line):
    lineCopy = line.strip()
    parts = lineCopy.split(':')
    if (len(parts) == 1):
        return [esteAtom(parts[0])]
    elif (len(parts) == 2):
        conditie = esteConditie(parts[1])
        if (conditie):
            return [esteAtom(parts[0]), conditie]
    return False
    
# Test pentru afirmatie
# print(esteAfirmatie("P(2)"))
# print(esteAfirmatie("r(?a) : P(?a), Q(f(g(?a, ?b), hello))")) 
    
# Primește un nume de fișier și întoarce un tuplu de cel puțin două elemente, 
#  dintre care primul este baza de cunștințe formată din afirmațiile din fișier,
#  iar al doilea lista de interogări din fișier
def parse_file(filename):
    queries = []
    kb = []
    primaInterogare = False
    with open(filename) as file:
        for line in file:
            # Linie goala
            if (len(line.strip()) == 0):
                continue
            
            # Inainte de prima interogare liniile care incep cu : sunt comentarii
            # Dupa prima interogare sunt rezultate - niciuna din aceste linii nu afecteaza kb si queries
            if not (primaInterogare):   
                if (esteComentariu(line)):
                    continue
            elif (esteRezultat(line)):
                continue
                
            # Pentru liniile care contin interogari introducem interogarea in lista queries
            interogare = esteInterogare(line)
            if (interogare):
                primaInterogare = True
                queries.append(interogare)
                continue
            # Pentru liniile care contin afirmatii introducem afirmatiile in kb
            else:
                afirmatie = esteAfirmatie(line)
                if (afirmatie):
                    if (len(afirmatie) == 1):
                        concluzie = afirmatie[0]
                        add_statement(kb, concluzie)
                    else:
                        concluzie = afirmatie[0]
                        ipoteze = afirmatie[1]
                        add_statement(kb, concluzie, *ipoteze)
                    continue
    # Asignez nume unice de variabile pentru baza de cunostinte creata
    make_unique_var_names(kb)
    return (kb, queries)

# Testare parsare
result = parse_file("test2.txt")
kb = result[0]
queries = result[1]
print_KB(kb)
for qi in range(len(queries)):
    print("Query: " + print_formula(queries[qi], True))
        # "  (" + str(q[0]) + ")"
        # ("" if answers is None else " Answer(s): " +
        # "".join([(print_subst(s, True) + ";  ") for s in answers[qi]]))


# In[ ]:




# ## Raționament
# 
# ### Cerința 3
# 
# Implementați un motor de raționament cu **backward-chaining** care să poată demonstra dacă o interogare dată este adevărată, și în ce condiții.
# 
# Trebuie să implementați funcția `answer_query`, care primește o bază de cunoștințe și o interogare și întoarce o listă de substituții posibile ale variabilelor din interogare, fiecare dintre substituții făcând ca interogarea să fie adevărată. Funcția întoarce mereu o listă (vidă, atunci când interogarea este mereu falsă).
# 

# In[110]:

from copy import *
from itertools import combinations

# Functii importate din laboratoarele de inteligenta artificiala
def is_positive_literal(L):
    return is_atom(L)

def is_negative_literal(L):
    global neg_name
    return get_head(L) == neg_name and is_positive_literal(get_args(L)[0])

def is_literal(L):
    return is_positive_literal(L) or is_negative_literal(L)

def get_premises(formula):
    result = []
    for premise in get_args(formula):
        if (is_negative_literal(premise)):
            result.append(get_args(premise)[0])
    return result

def get_conclusion(formula):
    result = []
    for premise in get_args(formula):
        if (is_positive_literal(premise)):
            result.append(premise)
    return result[0]

def is_fact(formula):
    return is_positive_literal(formula)

def is_rule(formula):
    if get_head(formula) != get_head(make_or(make_atom("P"), make_atom("P"))):
        return False
    if not is_positive_literal(get_conclusion(formula)):
        return False
    return all(list(map(is_positive_literal, get_premises(formula))))

# Face reuniune intre 2 dictionare
def unifySubstitution(s1, s2):
    result = {}
    for key1, value1 in s1.items():
        result[key1] = value1
        
    for key2, value2 in s2.items():
        if not (key2 in result):
            result[key2] = value2
                
    return result

# Verifica daca parametrul primit contine ca parametru in adancime o functie care poate fi evaluata
# adica are toti parametrii - constante. Evaluaza cu functia eval si inlocuieste functia cu rezultatul
def evaluateFunction(pred):
    if (is_constant(pred) or is_variable(pred)):
        return False
    if (is_function_call(pred)):
        args = get_args(pred)
        replace = []
        for arg in args:
            if (is_constant(arg)):
                replace.append(arg)
            if (is_variable(arg)):
                return False
            if (is_function_call(arg)):
                result = evaluateFunction(arg)
                if not (result):
                    return False
                replace.append(result)
        mystr = get_head(pred)
        myargs = ""
        for arg in get_args(pred):
            val = str(get_value(arg))
            if not (val.isnumeric()):
                val = "'" + val + "'"
            myargs = myargs + "," + val
        myargs = myargs[1:]
        mystr = mystr + "(" + myargs + ")"
        result = make_const(eval(mystr))
        return result
    else:
        replaced = []
        for arg in get_args(pred):
            if (is_function_call(arg)):
                result = evaluateFunction(arg)
                if not (result):
                    return False
                replaced.append(result)
            else:
                replaced.append(arg)
        return replace_args(pred, replaced)
            
    return False

# Testare functie evaluateFunction
# print(evaluateFunction(make_function_call("add", make_const(1), make_const(2))))
# print(evaluateFunction(make_function_call("add", make_const(1), make_var("X"))))
# print(eval("get('ABC', 0)"))

# Algoritmul de backward chaining: porneste de la o baza de cunostinte si o teorema - un scop
def backward_chaining(kb, teoreme, sigma = None, verbose = False):
    if not (sigma):
        sigma = {}
        
    raspunsuri = []
    # Daca nu mai am teoreme returnez o lista cu substitutia
    if (len(teoreme) == 0):
        return [sigma]
    
    if (verbose):
        print ()
        print ("Stack now: ", teoreme)
        print ("Subst now: ", sigma)
        print ("Satisfying: ", teoreme[0])
    
    # Daca mai sunt teoreme, iau prima teorema, aplic substitute pentru a inlocui variabilele substituibile, incerc sa 
    # inlocuiesc functiile care pot fi evaluate
    teorema = teoreme[0]
    newQ = substitute(teorema, sigma)
    canEvaluate = evaluateFunction(newQ)
    if (canEvaluate):
        newQ = canEvaluate
    
    # Pentru fiecare statement din baza de cunostinte incerc sa unific cu teorema actuala
    for k in kb:
        concluzie = None
        premise = []
        newSigma = {}
        
        # Daca k este un fapt si este exact ce trebuia demonstrat, consider demonstratia incheiata pe acest scop
        # neadaugand nimic la teoremele de rezolvat
        if (is_fact(k) and k == newQ):
            if (verbose):
                print ("Successfully unified ", newQ, " with fact ", k)
            newTeoreme = deepcopy(teoreme[1:])
            raspuns = backward_chaining(kb, newTeoreme, unifySubstitution(sigma, newSigma), verbose)
            raspunsuri.extend(raspuns)
            continue
        
        # Daca k este o regula si concluzia este ce trebuia rezolvat, consider demonstratia incheiata pe acest scop
        # dar adaug premisele la teoremele de rezolvat
        if (is_rule(k) and get_conclusion(k) == newQ):
            if (verbose):
                print ("Successfully unified ", newQ, " with conclusion: ", get_conclusion(k), " from rule ", k)
            premise = get_premises(k)
            newTeoreme = deepcopy(teoreme[1:])
            newTeoreme.extend(premise)
            raspuns = backward_chaining(kb, newTeoreme, unifySubstitution(sigma, newSigma), verbose)
            raspunsuri.extend(raspuns)
            continue
        
        # Daca teorema de rezolvat nu este nici egala cu faptul k, nici cu concluzia lui k in cazul in care k este o regula,
        # incerc o unificare intre concluzia lui k si teorema
        if (is_fact(k)):
            concluzie = k  
        elif (is_rule(k)):
            concluzie = get_conclusion(k)
            premise = get_premises(k)
        newSigma = unify(concluzie, newQ, None)
        # Daca nu se poate unifica inseamna ca k actual nu ma ajuta la rezolvarea teoremei actuale si trec la 
        # urmatoarea cunostinta din baza de cunostinte
        if not (newSigma):
            continue

        if (verbose):
            if (is_fact(k)):
                print ("Successfully unified ", newQ, " with fact ", k)
            else:
                print ("Successfully unified ", newQ, " with conclusion ", get_conclusion(k), " from rule ", k)
            
        # Daca a putut unifica adaug premisele la scopurile de demonstrat si reunesc cele 2 substitutii 
        # cea anterioara unificarii si cea rezultata din unificare
        newTeoreme = deepcopy(teoreme[1:])
        newTeoreme.extend(premise)
        raspuns = backward_chaining(kb, newTeoreme, unifySubstitution(sigma, newSigma), verbose)
        raspunsuri.extend(raspuns)
    return raspunsuri 

# Testeaza daca valorile din subst1 si subst2 cu aceeasi cheie sunt egale.
# Daca toate sunt egale returneaza reuniunea substitutiilor, altfel False
# Functie folosita cand am substitutiile pentru fiecare propozitie de demonstrat si query-ul era o conjunctie intre acele
# propozitii. Astfel verific daca varibilele comune propozitiilor au aceleasi valori in substitutii
def substIntersection(subst1, subst2):
    result = {}
    for key1, value1 in subst1.items():
        result[key1] = value1
        
    for key2, value2 in subst2.items():
        if (key2 in result and not value2 == result[key2]):
            return False
        result[key2] = value2
    return result

# Funcția primește o bază de cunoștințe (listă de disjuncții de literali) și o interogare (conjuncție de literali)
#  și întoarce o listă de substituții (dicționare nume-variabilă -> termen), dintre care fiecare,
#  aplicată asupra interogării, o face pe aceasta adevărată
# Dacă parametrul verbose este True, trebuie ilustrați, prin output la consolă, pașii raționamentului.
# Substituțiile trebuie să conțină doar variabile care apar în interogare.
def answer_query(kb, query, verbose = False):
    if (verbose):
        print ()
        print("Query: ", query)
    subst = []
    q = []
    # Pentru un atom de demonstrat trimit direct atomul in backward_chaining
    if (is_atom(query)):
        subst = backward_chaining(kb, [query], None, verbose)
        q = gather_vars(query)
    # Pentru o conjunctie de atomi de demonstrat rezolv fiecare atom in parte apoi verific ca variabilele comune
    # atomilor sa aiba aceleasi valori in substitutii
    else:
        substList = []
        for arg in get_args(query):
            substList.append(backward_chaining(kb, [arg], None, verbose))
        subst = substList[0]
        substList = substList[1:]
        for i in range(0, len(substList)):
            aux = []
            for dict1 in subst:
                for dict2 in substList[i]:
                    intersect = substIntersection(dict1, dict2)
                    if (intersect):
                        aux.append(intersect)
            subst = aux
        for arg in get_args(query):
            qvars = gather_vars(arg)
            for v in qvars:
                if not (v in q):
                    q.append(v)
    result = []
    # Din substitutii extrag doar variabilele care apar in teoremele de demonstrat si creez substitutiile de returnat
    for s in subst:
        r = {}
        for key, value in s.items():
            if (key in q):
                r[key] = value
        if not (r in result):
            result.append(r)
    
    if (verbose):
        print()
        if (len(result) == 0):
            print ("Query ", query, " is False")
        elif (len(result) == 1 and result[0] == {}):
            print ("Query ", query, " is True")
        else:
            print ("Query ", query, " is True for the next substitutions: ")
            for r in result: 
                print (r)
    return result

# Teste pentru answer_query si pentru verbose
# result = parse_file("test1.txt")
# kb = result[0]
# queries = result[1]
# ans = answer_query(kb, queries[0], True)
# ans = answer_query(kb, queries[1], True)
# ans = answer_query(kb, queries[len(queries) - 1], True)


# In[ ]:




# In[ ]:




# ## Interogare interactivă
# 
# ### Cerința 3
# 
# Implementați funcția `interactive_query` care execută interogări pe baza de cunoștințe la cererea utilizatorului.
# * Funcția trebuie să preia de la tastatură o linie în care utilizatorul scrie o interogare (atomi despărțiți prin virgulă) (pentru input puteți folosi `raw_input` sau `input` în Python 2 sau 3, respectiv.)
# * Dacă inputul de la utilizator este "q", atunci funcția se întoarce.
# * Se execută interogarea, folosind motorul de inferență, rezultatul este afișat, și apoi se acceptă o nouă interogare din partea utilzatorului.

# In[111]:

# Iau input de la tastatura si ma astept sa primesc interogari
def interactive_query(kb):
    while(True):
        interogatie = input('Write a query: ')
        # Pentru interogarea q ies din modul interactiv
        if (interogatie == "q"):
            break
        if (len(interogatie) > 0 and interogatie[0] != '?'):
            interogatie = "?" + interogatie
        result = esteInterogare(interogatie)
        # Pentru o linie care nu contine o interogare valida returnez un mesaj sugestiv
        if not (result):
            print ("This is not a valid query: ", interogatie)
            continue
        # Pentru o interogare valida returnez False, daca nu primesc nicio substitutie, True daca primesc ca
        # substitutie un dictionar gol si o printez o lista de substitutii daca s-au gasit substitutii care fac teorema True
        response = answer_query(kb, result)
        if (len(response) == 0):
            print (False)
        elif (len(response) == 1 and response[0] == {}):
            print (True)
        else:
            for r in response: 
                print (r)
            
# Teste pentru interactive_query
#interactive_query(parse_file("test1.txt")[0])
#interactive_query(parse_file("test2.txt")[0])
#interactive_query(KB_America)


# In[ ]:




# In[ ]:




# ### Testare pentru întreaga temă

# In[112]:

# Testing

total_points = 0

def test_with(filename, expected_answers = [], kb = None, queries = None, points = None):
    global total_points
    
    new_tests()
    
    if filename is None and kb is not None and queries is not None:
        result = (kb, queries)
    else:
        result = parse_file(filename)
    print_KB(result[0])
    index = 0
    if len(result[1]) < len(expected_answers):
        print("FAILED: There should be " + str(len(expected_answers)) + " queries extracted from the file.")
    for q in result[1]:
        expected = [] if len(expected_answers) - 1 < index else expected_answers[index]
        expected = [{}] if expected == True else ([] if expected == False else expected)
        passed = test3(q, answer_query(result[0], q), expected)
        if passed and points is not None:
            if isinstance(points, list):
                if index < len(points):
                    total_points += points[index]
                    print("\t\t\t\t+ ", points[index], "puncte")
            else:
                total_points += points
                print("\t\t\t\t+ ", points, "puncte")
        index += 1
        
test1_solutions = [
    True,
    False,
    False,
    False,
    True,
    [{"y": make_const(3)}],
    [{"X": make_const(1)}, {"X": make_const(2)}],
    True,
    True,
    [{"a": make_const(1)}, {"a": make_const(2)}],
    [{"x": make_const(1)}, {"x": make_const(2)}],
    [{"y": make_const(1)}, {"y": make_const(2)}],
    [{"a": make_const(2), "b": make_const(3)}, {"a": make_const(1), "b": make_const(3)}],
    ]

test2_solutions = [
    True, True, True,
    False, False, False,
    True, True, False, False
    ]

print("===================== PARSARE (parsarea va fi verificată integral la prezentare)")
r = parse_file("test1.txt")
passed = True
if not isinstance(r, tuple) or len(r) < 2 or not isinstance(r[0], list) or not len(r[0]):
    print("FAILED.")
else:
    for s in r[0]:
        if not check_sentence(s):
            passed = False
    if passed:
        total_points += 20
        print("OK\n\t\t\t\t+ 20 puncte")

print("===================== ROUND A")
test_with("test1.txt", test1_solutions, points = [2.5, 2.5, 2.5, 2.5, 5, 5, 5, 5, 5, 2, 2, 3, 3])

print("===================== ROUND B")
test_with("test3.txt", [
        [
            {"animal": make_const("Lion"), "day": make_const("Monday")},
            {"animal": make_const("Lion"), "day": make_const("Thursday")}, 
            {"animal": make_const("Unicorn"), "day": make_const("Sunday")},
            {"animal": make_const("Unicorn"), "day": make_const("Thursday")}], 
        [{"today": make_const("Thursday")}]], points = [5, 10])

print("===================== ROUND C")
test_with(None, [
        True,
        [{"x": make_const("West")}, {"x": make_const("General_Awesome")}],
        True,
        [{"x": make_const("West")}],
        True,
        [{"x": make_const("America")}, {"x": make_const("General_Awesome")}],
        False, False, False,
        [{"x": make_const("Nono"), "y": make_const("M1"), "z": make_const("West")}],
    ], KB_America, [
        make_atom("American", make_const("West")),
        make_atom("American", make_var("x")),
        make_atom("Criminal", make_const("West")),
        make_atom("Criminal", make_var("x")),
        make_atom("Awesome", make_const("America")),
        make_atom("Awesome", make_var("x")),
        make_atom("Criminal", make_const("General_Awesome")),
        make_atom("Criminal", make_const("Nono")),
        make_atom("American", make_const("Nono")),
        make_atom("Sells", make_var("z"), make_var("y"), make_var("x")),
    ], 1)

print("===================== BONUS")
test_with("test2.txt", test2_solutions, points = 2)

print("TOTAL:", total_points, "puncte din 100.")
print("mai rămâne de verificat manual interogarea interactivă pt 10p.")

#interactive_query(r[0])

