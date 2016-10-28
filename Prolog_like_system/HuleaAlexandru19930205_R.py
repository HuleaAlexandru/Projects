### Reprezentare - construcție

# întoarce un termen constant, cu valoarea specificată
def make_const(value):
    return ("constanta", value)

# întoarce un termen care este o variabilă, cu numele specificat
def make_var(name):
    return ("variabila", name)

# întoarce un termen care este un apel al funcției cu numele specificat, pe restul argumentelor date.
# E.g. pentru a construi termenul add[1, 2, 3] vom apela make_function_call(add, 1, 2, 3)
# !! ATENȚIE: python dă args ca tuplu cu restul argumentelor, nu ca listă. Se poate converti la listă cu list(args)
def make_function_call(name, *args):
    return ("functie", str(name), list(args))

# întoarce o formulă formată dintr-un atom care este aplicarea predicatului dat pe restul argumentelor date
# !! ATENȚIE: python dă args ca tuplu cu restul argumentelor, nu ca listă. Se poate converti la listă cu list(args)
def make_atom(predicate, *args):
    return ("atom", predicate, list(args))

# întoarce o formulă care este negarea propoziției date
# get_args(make_neg(s1)) va întoarce [s1]
def make_neg(sentence):
    return ("neg", [sentence])

# întoarce o formulă care este conjuncția propozițiilor date
# e.g. apelul make_and(s1, s2, s3, s4) va întoarce o structură care este conjuncția s1, s2, s3, s4
#  și get_args pe această structură va întoarce [s1, s2, s3, s4]
def make_and(sentence1, sentence2, *others):
    args = [sentence1, sentence2]
    args.extend(list(others))
    return ("and", args)

# întoarce o formulă care este disjuncția propozițiilor date
# e.g. apelul make_or(s1, s2, s3, s4) va întoarce o structură care este disjuncția s1, s2, s3, s4
#  și get_args pe această structură va întoarce [s1, s2, s3, s4]
def make_or(sentence1, sentence2, *others):
    args = [sentence1, sentence2]
    args.extend(list(others))
    return ("or", args)

# întoarce o copie a formulei sau apelul de funcție date, în care argumentele au fost înlocuite cu lista new_args
# e.g. pentru formula p(x, y), înlocuirea argumentelor cu lista [1, 2] va rezulta în formula p(1, 2)
# Noua listă de argumente trebuie să aibă aceeași lungime cu numărul de argumente inițial din formulă
def replace_args(formula, new_args):
    if (is_function_call(formula) and len(get_args(formula)) == len(new_args)):
        return make_function_call(get_head(formula), *new_args)
    if (is_sentence(formula)):
        if (len(get_args(formula)) == len(new_args)):
            if (is_atom(formula)):
                    return make_atom(get_head(formula), *new_args)
            else:
                if (get_head(formula) == "neg"):
                    return make_neg(*new_args)
                elif (get_head(formula) == "and"):
                    return make_and(new_args[0], new_args[1], *new_args[2:])
                elif (get_head(formula) == "or"):
                    return make_or(new_args[0], new_args[1], *new_args[2:])
    return None
    
    
### Reprezentare - verificare

# întoarce adevărat dacă f este un termen
def is_term(f):
    return is_constant(f) or is_variable(f) or is_function_call(f)

# întoarce adevărat dacă f este un termen constant
def is_constant(f):
    return f[0] == "constanta"

# întoarce adevărat dacă f este un termen ce este o variabilă
def is_variable(f):
    return f[0] == "variabila"

# întoarce adevărat dacă f este un apel de funcție
def is_function_call(f):
    return f[0] == "functie"

# întoarce adevărat dacă f este un atom (aplicare a unui predicat)
def is_atom(f):
    return f[0] == "atom"

# întoarce adevărat dacă f este o propoziție validă
def is_sentence(f):
    return f[0] == "atom" or f[0] == "neg" or f[0] == "and" or f[0] == "or"

def has_args(f):
    return is_function_call(f) or is_sentence(f)


### Reprezentare - verificare

# pentru constante (de verificat), se întoarce valoarea constantei. Altfel, None
def get_value(f):
    if (is_constant(f)):
        return f[1]
    return None

# pentru variabile (de verificat), se întoarce numele variabilei. Altfel, None
def get_name(f):
    if (is_variable(f)):
        return f[1]
    return None

# pentru apeluri de funcții, se întoarce numele funcției; pentru atomi, se întoarce numele predicatului; 
# pentru propoziții compuse, se întoarce un șir de caractere care reprezintă conectorul logic (e.g. ~, A sau V)
# altfel, None
def get_head(f):
    if (is_function_call(f)):
        return f[1]
    if (is_atom(f)):
        return f[1]
    if (is_sentence(f)):
        return f[0]
    return None

# pentru propoziții sau apeluri de funcții, se întoarce lista de argumente.  Altfel, None.
# Vezi și "Important:", mai sus.
def get_args(f):
    if (is_function_call(f)):
        return f[2]
    if (is_sentence(f)):
        if (is_atom(f)):
            return f[2]
        else:
            return f[1]
    return None