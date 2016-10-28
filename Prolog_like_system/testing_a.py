from HuleaAlexandru19930205_R import *

dummy = make_atom("P")
dummy2 = make_atom("Q", make_var("x"))
L1 = [dummy, dummy, dummy, dummy]
L2 = [dummy2, dummy2, dummy2, dummy2]

and_name = get_head(make_and(dummy, dummy))
or_name = get_head(make_or(dummy, dummy))
neg_name = get_head(make_neg(dummy))

# re-initializes test counter
def new_tests():
    global idx
    idx = 0

# tests that v (value) equals c (check)
def test(v, c):
    global idx
    print("Test ", idx, ":", "OK" if v == c else "Failed, got [" + str(v) + "], should be [" + str(c) + "]")
    idx += 1

# checks if 2 substitutions are equal
def eq_s(s1, s2):
    if isinstance(s1, dict) and isinstance(s2, dict):
        return len(s1) == len(s2) and not [v for v in s1 if v not in s2 or s1[v] != s2[v]]
    return s1 == s2
# tests that one substitution is equal to the other
def test2(sv, sc):
    global idx
    print("Test ", idx, ":", "OK, got [" + str(sv) + "]" if eq_s(sv, sc)
                      else "Failed, got [" + str(sv) + "], should be [" + str(sc) + "]")
    idx += 1

# tests if two substitution lists are equivalent
def eq_s_plus(answersV, answersC):
    if not isinstance(answersV, list) or not isinstance(answersC, list) or len(answersV) != len(answersC):
        return False
    for answer in answersC:
        if not [aV for aV in answersV if eq_s(aV, answer)]:
            return False
    return True
# tests that two answers for interrogations are equal
def test3(query, av, ac):
    global idx
    passed = eq_s_plus(av, ac)
    print(("OK" if passed else "FAILED"), "Test", idx, ":", print_formula(query, True), ":",
              "got: " + print_answer(av, True) if passed else
              "\n\tgot: " + print_answer(av, True) + "\n\tshould be: " + print_answer(ac, True))
    idx += 1
    return passed

    

# Afișează formula f. Dacă argumentul return_result este True, rezultatul nu este afișat la consolă, ci întors.
def print_formula(f, return_result = False):
    ret = ""
    if is_term(f):
        if is_constant(f):
            ret += str(get_value(f))
        elif is_variable(f):
            ret += "?" + get_name(f)
        elif is_function_call(f):
            ret += get_head(f) + "[" + "".join([print_formula(arg, True) + "," for arg in get_args(f)])[:-1] + "]"
        else:
            ret += "???"
    elif is_atom(f):
        ret += get_head(f) + "(" + "".join([print_formula(arg, True) + ", " for arg in get_args(f)])[:-2] + ")"
    elif is_sentence(f):
        # negation, conjunction or disjunction
        args = get_args(f)
        if len(args) == 1:
            ret += get_head(f) + print_formula(args[0], True)
        else:
            ret += "(" + get_head(f) + "".join([" " + print_formula(arg, True) for arg in get_args(f)]) + ")"
    else:
        ret += "???"
    return ret if return_result else print(ret)
    
# Afișează substituția s
def print_subst(s, return_result = False):
    ret = ""
    if s == False:
        ret = "False"
    elif s == {}:
        ret = "Empty"
    else:
        for var_name in s:
            ret += var_name + " -> " + print_formula(s[var_name], True) + "  "
    return ret if return_result else print(ret)
    
# Afișează lista de substituții a, cu valorile speciale "True", dacă lista conține {}, și "False", dacă lista este vidă.
def print_answer(a, return_result = False):
    ret = ""
    if a == []:
        ret = "False"
    elif [s for s in a if s == {}]:
        ret = "True"
    else:
        ret = "".join([(print_subst(s, True) + ";  ") for s in a])
    return ret if return_result else print(ret)
    
def print_KB(KB):
    print("KB:")
    for s in KB:
        print("\t\t\t" + print_formula(s, True))
        
from collections import Iterable

PRINT_SUCCESS = False

def fail(message):
    print(message)
    return False
def success(message, formula):
    if PRINT_SUCCESS:
        print(message + ": " + print_formula(formula, True))
    return True

def check_term(T):
    if not is_term(T):
        return fail("Should be a term but is not:" + print_formula(T, True))
    if is_constant(T):
        if get_value(T) is None:
            return fail("Value of constant is None")
        return success("OK constant", T)
    if is_variable(T):
        if get_name(T) is None:
            return fail("Name of variable is None")
        return success("OK variable", T)
    if is_function_call(T):
        if get_head(T) is None:
            return fail("Function reference (head) is None")
#        if not callable(get_head(T)):
#            return fail(get_head(T) + " is not a callable object")
        if [arg for arg in get_args(T) if not check_term(arg)]:
            return fail("Argument check for function " + str(get_head(T)) + " failed")
        return success("OK function call", T)
    return fail("Term is not a constant, variable or function call.")
    
def check_atom(A):
    if not is_atom(A):
        return fail("Should be an atom but is not:" + print_formula(A, True))
    elif get_head(A) is None:
        return fail("Head of atom is None")
    elif [arg for arg in get_args(A) if not check_term(arg)]:
        return fail("Argument check for predicate " + str(get_head(A)) + " failed")
    return success("OK atom", A)

def check_sentence(S):
    if not is_sentence(S):
        return fail("Should be a sentece but is not: " + print_formula(S, True))
    if is_atom(S):
        if not check_atom(S):
            return fail("Atomic sentence check failed for " + print_formula(S, True))
        else:
            return success("OK sentence", S)
    if get_head(S) is None:
        return fail("Sentece head is None")
    if get_args(S) is None or not isinstance(get_args(S), Iterable):
        return fail("Sentence argument list is not iterable")
    head = get_head(S)
    args = get_args(S)
    nargs = len(args)
    if head == neg_name:
        if nargs != 1:
            return fail("Negation has " + ("too many" if nargs > 1 else "too few") + "arguments (" + nargs + ")")
        if not check_sentence(get_args(S)[0]):
            return fail("Argument check for negation has failed")
        return success("OK negation", S)
    if head in [and_name, or_name]:
        if nargs < 2:
            return fail("Operator has too few arguments (" + nargs + ")")
        if [arg for arg in get_args(S) if not check_sentence(arg)]:
            return fail("Argument check for " + head + " has failed")
        return success("OK " + head, S)
    return fail("Sentence is of unknown type: " + print_formula(S, True))


def make_statement(conclusion, hypotheses):
    L = list(hypotheses)
    if not L:
        return conclusion
    L = [make_neg(s) for s in L]
    L.append(conclusion)
    return make_or(*L)

def add_statement(kb, conclusion, *hypotheses):
    s = make_statement(conclusion, hypotheses)
    if check_sentence(s) is not None:
        kb.append(s)
        #print("Added statement " + print_formula(s, True) + "  (" + str(s) + ")")
        return True
    print("Sentence does not check out ", result)
    return False


var_no = 0;
def assign_next_var_name():
    global var_no
    var_no += 1
    return "v" + str(var_no)

def gather_vars(S):
    if is_variable(S):
        return [get_name(S)]
    if has_args(S):
        res = []
        for a in get_args(S):
            res = res + gather_vars(a)
        return res
    return []

def make_unique_var_names_in_sentence(sentence):
    varL = gather_vars(sentence)
    subst = {}
    for var in varL:
        subst[var] = make_var(assign_next_var_name())
    return substitute(sentence, subst)


def make_unique_var_names(KB):
    for idx in range(len(KB)):
        KB[idx] = make_unique_var_names_in_sentence(KB[idx])
            

            
# Aplică în formula f toate elementele din substituția dată și întoarce formula rezultată
def substitute(f, substitution):
    if substitution is None:
        return None
    if is_variable(f) and (get_name(f) in substitution):
        return substitute(substitution[get_name(f)], substitution)
    if has_args(f):
        return replace_args(f, [substitute(arg, substitution) for arg in get_args(f)])
    return f

# KB America

# KB 1
# based on an example in Artificial Intelligence - A Modern Approach
KB_America = []
#0 Mr West is a US general
add_statement(KB_America, make_atom("USGeneral", make_const("West")))
#1 General Awesome is also a US general
add_statement(KB_America, make_atom("USGeneral", make_const("General_Awesome")))
#2 General Awesome is Awesome
add_statement(KB_America, make_atom("Awesome", make_const("General_Awesome")))
#3 Nono is an enemy of America
add_statement(KB_America, make_atom("Enemy", make_const("Nono"), make_const("America")))
#4 M1 is a type of missle
add_statement(KB_America, make_atom("Missle", make_const("M1")))
#5 Nono has the M1 missle
add_statement(KB_America, make_atom("Owns", make_const("Nono"), make_const("M1")))

#6 any US general is an American
add_statement(KB_America, make_atom("American", make_var("x")), make_atom("USGeneral", make_var("x")))
#7 any missle is a weapon
add_statement(KB_America, make_atom("Weapon", make_var("x")), make_atom("Missle", make_var("x")))
#8 if anyone owns a missle, it is General West that sold them that missle
add_statement(KB_America, make_atom("Sells", make_const("West"), make_var("y"), make_var("x")), make_atom("Owns", make_var("x"), make_var("y")), make_atom("Missle", make_var("y")))
#9 any American who sells weapons to a hostile is a criminal
add_statement(KB_America, make_atom("Criminal", make_var("x")), make_atom("Weapon", make_var("y")), make_atom("Sells", make_var("x"), make_var("y"), make_var("z")), make_atom("Hostile", make_var("z")), make_atom("American", make_var("x")))
#10 any enemy of America is called a hostile
add_statement(KB_America, make_atom("Hostile", make_var("x")), make_atom("Enemy", make_var("x"), make_const("America")))
#11 America is awesome if at least an American is awesome
add_statement(KB_America, make_atom("Awesome", make_const("America")), make_atom("American", make_var("x")), make_atom("Awesome", make_var("x")))

make_unique_var_names(KB_America)



# funcții utile pentru teste


def get(triangle, index):
    vertices = list(triangle)
    vertices.sort()
    v0 = vertices[0]
    v1 = vertices[1]
    v2 = vertices[2]
    return [v0+v1, v1+v2, v0+v2][index]

def compute_triangle(a, b, c):
    sorted = [a, b, c]
    sorted.sort()
    return sorted[0] + sorted[1] - sorted[2]

def getShortest(a, b, c):
    return getsorted(a, b, c, 0)
def getMiddle(a, b, c):
    return getsorted(a, b, c, 1)
def getLongest(a, b, c):
    return getsorted(a, b, c, 2)
def getsorted(a, b, c, i):
    sorted = [a, b, c]
    sorted.sort()
    return sorted[i]


def compute_pitagoras(s, m, l):
    return l*l - m*m - s*s
    