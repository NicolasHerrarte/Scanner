#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "dynarray.h"

#define MAX_REGEX_LENGTH 32

#define ALT_PRIORITY 0
#define CONCAT_PRIORITY 1
#define CLOS_PRIORITY 2
#define FIN_PRIORITY 3
#define MAX_PRIORITY 4

#define len_nfa_states(nfa) dynarray_length(nfa.states)
#define bool_table_generator(ret_type, f_name) ret_type* f_name##_b_table_to_list(bool* b_table, int table_size){\
    ret_type* sub_list = dynarray_create(ret_type);\
    for(int i = 0; i < table_size;i++){\
        if(b_table[i] == true){\
            ret_type tmp = (ret_type) i;\
            dynarray_push(sub_list, tmp);\
        }\
    }\
    return sub_list;\
}

bool_table_generator(int, int)
bool_table_generator(unsigned char, char)

typedef struct Transition{
    int state_from;
    int state_to;
    char trans_char;
} Transition;

typedef struct FA{
    int* states;
    int initial_state;
    bool alphabet[256];
    Transition* transitions;
    int* acceptable_states;
} FA;

typedef struct SSubSet{
    bool* states;
    int length;
    int states_count;
} SSubSet;


SSubSet SSS_initialize_empty(int states_length){
    SSubSet subset;
    subset.length = states_length;
    subset.states_count = 0;
    subset.states = malloc(states_length * sizeof(bool));
    memset(subset.states, 0, states_length * sizeof(bool));

    return subset;
}

SSubSet SSS_initialize(int states_length, int* add_states, int states_amount){
    SSubSet subset;
    subset.length = states_length;
    subset.states_count = 0;
    subset.states = malloc(states_length * sizeof(bool));
    memset(subset.states, 0, states_length * sizeof(bool));

    for(int i = 0;i<states_amount;i++){
        if(subset.states[add_states[i]] == false){
            subset.states[add_states[i]] = true;
            subset.states_count += 1;
        }  
    }

    return subset;
}

void SSS_add(SSubSet* subset, int new_states){
    if(subset->states[new_states] == false){
            subset->states[new_states] = true;
            subset->states_count += 1;
    }  
    subset->states[new_states] = true;
}

void SSS_remove(SSubSet* subset, int rem_states){
    if(subset->states[rem_states] == true){
            subset->states[rem_states] = false;
            subset->states_count -= 1;
    }  
}

bool SSS_equal(SSubSet subset1, SSubSet subset2){
    for(int i = 0;i<subset1.length;i++){
        if(subset1.states[i] != subset2.states[i]){
            return false;
        }
    }
    return true;
}

bool SSS_in(SSubSet subset1, int state){
    return subset1.states[state];
}

bool SSS_list_in(SSubSet* subset_list, SSubSet elem){
    for(int i = 0;i<dynarray_length(subset_list);i++){
        if(SSS_equal(subset_list[i], elem)){
            return true;
        }
    }

    return false;
}


int* SSS_to_list(SSubSet subset){
    return int_b_table_to_list(subset.states, subset.length);
}

void SSS_print(SSubSet subset){
    for(int i = 0; i < subset.length;i++){
        if(subset.states[i] == true){
            printf("%d, ", i);
        } 
    }
    printf("\n");
}

void print_transition(Transition t){
    printf("State: ");
    printf("%d ", t.state_from);
    printf("- ");
    printf("%c ", t.trans_char);
    printf("-> State: ");
    printf("%d", t.state_to);
    printf("\n");
}

void FA_print(FA fa){
    printf("-- States --\n");
    for(int i = 0; i < dynarray_length(fa.states);i++){
        printf("%d, ", fa.states[i]);
    }
    printf("\n");
    printf("-- Acceptable States --\n");
    for(int i = 0; i < dynarray_length(fa.acceptable_states);i++){
        printf("%d, ", fa.acceptable_states[i]);
    }
    printf("\n");
    printf("-- Alphabet --\n");
    for(int i = 0; i < 256;i++){
        if(fa.alphabet[i] == true){
            printf("%c, ", (unsigned char) i);
        }
        
    }
    printf("\n");
    printf("-- Transitions --\n");
    for(int i = 0; i < dynarray_length(fa.transitions);i++){
        print_transition(fa.transitions[i]);
    }

    printf("-- Starting State --\n");
    printf("- %d\n", fa.initial_state);
}

void states_print(int* states){
    printf("[ ");
    for(int i = 0;i<dynarray_length(states);i++){
        printf("%d, ", states[i]);
    }
    printf("]\n");
}

int FA_initialize(FA *fa){
    fa->states = dynarray_create(int);
    fa->transitions = dynarray_create(Transition);
    fa->acceptable_states = dynarray_create(int);
    memset(fa->alphabet, 0, 256);
}

int FA_next_state(FA *fa){
    int next_int = dynarray_length(fa->states);
    dynarray_push(fa->states, next_int);

    return next_int;
}

void FA_add_acceptable_state(FA *fa, int acceptable_state){
    dynarray_push(fa->acceptable_states, acceptable_state);
}

Transition NFA_add_transition(FA *nfa, int _from, int _to, char _trans_char){
    Transition trans;
    trans.state_from = _from;
    trans.state_to = _to;
    trans.trans_char = _trans_char;
    dynarray_push(nfa->transitions, trans);

    if(_trans_char != '@'){
        int int_repr = (unsigned char) _trans_char;
        nfa->alphabet[int_repr] = 1;
    }

    return trans;
}

typedef struct Fragment{
    int start_index;
    int end_index;
} Fragment;


int altercation(Fragment *left_fragment, Fragment *right_fragment, int *priority, int depth, int i){
    if(ALT_PRIORITY < *priority && depth == 0){
        left_fragment->end_index = i;
        right_fragment->start_index = i+1;
        *priority = ALT_PRIORITY;
        return true;
    }
    return false;
}

int concatenation(Fragment fragment, Fragment *left_fragment, Fragment *right_fragment, int *priority, int depth, int i, char next_char, char prev_char){
    if(CONCAT_PRIORITY < *priority && depth == 0 && prev_char != '$'){
        if(i >= fragment.start_index+1){
            left_fragment->end_index = i;
            right_fragment->start_index = i;
            *priority = CONCAT_PRIORITY;
            return true;
        }
        else{
            if(next_char != '\0' && next_char != '*' && next_char != '$'){
                left_fragment->end_index = i+1;
                right_fragment->start_index = i+1;
                *priority = CONCAT_PRIORITY;
                return true;
            } 
        }
    }
    return false;
}

int closure(Fragment fragment, Fragment *left_fragment, Fragment *right_fragment, int *priority, bool *final_split, int depth, bool split_found, int pass_priority, int i){
    if(pass_priority <= *priority && depth == 0 && i == fragment.end_index-1){
        left_fragment->end_index = i;
        *final_split = true;
        *priority = pass_priority;
        return true;
    }
    return false;
}

int fin_type(Fragment fragment, Fragment *left_fragment, Fragment *right_fragment, int *priority, bool *final_split, int depth, int i){
    if(FIN_PRIORITY <= *priority && depth == 0 && i == fragment.end_index-2){
        left_fragment->end_index = i;
        *final_split = true;
        *priority = FIN_PRIORITY;
        return true;
    }
    return false;
}

int parenthesis(Fragment fragment, Fragment *left_fragment, bool *final_split, bool split_found, int i){
    if(i == fragment.end_index-1 && split_found == false){
        left_fragment->start_index += 1;
        left_fragment->end_index = i;
        *final_split = true;
        return true;
    }
    return false;
}


Fragment find_split_point(FA* nfa, char* str, Fragment fragment, bool final_state, bool recursion){
    Fragment left_fragment = {fragment.start_index, fragment.start_index};
    Fragment right_fragment = {fragment.end_index, fragment.end_index};

    int min_priority = MAX_PRIORITY;
    int parenthesis_depth = 0;

    bool final_split = false;
    bool split_found = false;
    bool found_solution = false;

    for(int i = fragment.start_index;i<fragment.end_index;i++){
        printf("%c", str[i]);
    }
    printf("\n");

    if(fragment.start_index == fragment.end_index-1){
        int state_head = FA_next_state(nfa);
        int state_tail = FA_next_state(nfa);

        if(final_state == true){
            FA_add_acceptable_state(nfa, state_tail);
        }
        Transition trans = NFA_add_transition(nfa, state_head, state_tail, str[fragment.start_index]);
        Fragment char_fragment = {state_head, state_tail};

        nfa->initial_state = state_head;
        return char_fragment;
    }

    else{
        for(int i = fragment.start_index;i<fragment.end_index;i++){
            char current_char = str[i];
            char previous_char;
            if(i == 0){
                previous_char = '\0';
            }
            else{
                previous_char = str[i-1];
            }

            if(current_char == '|'){
                found_solution = altercation(&left_fragment, &right_fragment, &min_priority, parenthesis_depth, i);
            }
            else if(current_char == '*'){
                found_solution = closure(fragment, &left_fragment, &right_fragment, &min_priority, &final_split, parenthesis_depth, split_found, CLOS_PRIORITY, i);
            }
            else if(current_char == '$'){
                found_solution = fin_type(fragment, &left_fragment, &right_fragment, &min_priority, &final_split, parenthesis_depth, i);
            }
            else if(current_char == '('){
                found_solution = concatenation(fragment, &left_fragment, &right_fragment, &min_priority, parenthesis_depth, i, '\0', previous_char);
                parenthesis_depth += 1;
            }
            else if(current_char == ')'){
                parenthesis_depth -= 1;
                found_solution = parenthesis(fragment, &left_fragment, &final_split, split_found, i);
            }
            else{
                found_solution = concatenation(fragment, &left_fragment, &right_fragment, &min_priority, parenthesis_depth, i, str[i+1], previous_char);
            }

            if(found_solution > 0){
                split_found = true;
            }
    }
    }

    if (parenthesis_depth != 0){
        printf("Parenthesis Mismatch -> %d", parenthesis_depth);
        Fragment error_fragment = {0, 0};
        return error_fragment;
    }

    //printf("(%d, %d) (%d, %d)\n",left_fragment.start_index, left_fragment.end_index, right_fragment.start_index, left_fragment.end_index);

    if(recursion == true){
        if(final_split == true){
            switch(min_priority){
                case CLOS_PRIORITY:
                    Fragment nfa_only_fragment = find_split_point(nfa, str, left_fragment, false, true);

                    int state_head_alt = FA_next_state(nfa);
                    int state_tail_alt = FA_next_state(nfa);
                    
                    Transition trans_start = NFA_add_transition(nfa, state_head_alt, nfa_only_fragment.start_index, '@');
                    Transition trans_tail = NFA_add_transition(nfa, nfa_only_fragment.end_index, state_tail_alt, '@');
                    Transition trans_ret = NFA_add_transition(nfa, nfa_only_fragment.end_index, nfa_only_fragment.start_index, '@');
                    Transition trans_bounce = NFA_add_transition(nfa, state_head_alt, state_tail_alt, '@');

                    Fragment alt_fragment = {state_head_alt, state_tail_alt};

                    if(final_state == true){
                        FA_add_acceptable_state(nfa, state_tail_alt);
                    }

                    nfa->initial_state = state_head_alt;
                    return alt_fragment;
                case FIN_PRIORITY:
                    Fragment final_tag_fragment = find_split_point(nfa, str, left_fragment, true, true);
                    return final_tag_fragment;
                case MAX_PRIORITY:
                    Fragment same_fragment = find_split_point(nfa, str, left_fragment, false, true);
                    if(final_state == true){
                        FA_add_acceptable_state(nfa, same_fragment.end_index);
                    }
                    return same_fragment;
                default:
                    printf("Something is not right");
            }
        }
        else{
            Fragment nfa_left_fragment = find_split_point(nfa, str, left_fragment, false, true);
            Fragment nfa_right_fragment = find_split_point(nfa, str, right_fragment, false, true);

            switch(min_priority){
                case ALT_PRIORITY:
                    //printf("ALTERCATION\n");
                    int state_head_alt = FA_next_state(nfa);
                    int state_tail_alt = FA_next_state(nfa);

                    Transition trans_0 = NFA_add_transition(nfa, state_head_alt, nfa_left_fragment.start_index, '@');
                    Transition trans_1 = NFA_add_transition(nfa, state_head_alt, nfa_right_fragment.start_index, '@');
                    Transition trans_2 = NFA_add_transition(nfa, nfa_left_fragment.end_index, state_tail_alt, '@');
                    Transition trans_3 = NFA_add_transition(nfa, nfa_right_fragment.end_index, state_tail_alt, '@');

                    Fragment alt_fragment = {state_head_alt, state_tail_alt};

                    if(final_state == true){
                        FA_add_acceptable_state(nfa, state_tail_alt);
                    }

                    nfa->initial_state = state_head_alt;
                    return alt_fragment;
                    
                case CONCAT_PRIORITY:
                    //printf("CONCATENATION\n");
                    
                    Transition trans = NFA_add_transition(nfa, nfa_left_fragment.end_index, nfa_right_fragment.start_index, '@');
                    Fragment concat_fragment = {nfa_left_fragment.start_index, nfa_right_fragment.end_index};

                    if(final_state == true){
                        FA_add_acceptable_state(nfa, nfa_right_fragment.end_index);
                    }

                    nfa->initial_state = nfa_left_fragment.start_index;
                    return concat_fragment;

                default:
                    printf("Something is not right\n");
            }
        }
    }
}

SSubSet e_closure(FA nfa, SSubSet states_closure){
    int* states = SSS_to_list(states_closure);
    int* inspect_states = dynarray_create(int);

    for(int i = 0;i<states_closure.states_count;i++){
        dynarray_push(inspect_states, states[i]);
    }

    while(dynarray_length(inspect_states) > 0){
        int n;

        dynarray_pop(inspect_states, &n);
        for(int i = 0;i<dynarray_length(nfa.transitions);i++){
            if(nfa.transitions[i].state_from == n && nfa.transitions[i].trans_char == '@'){
                if(!SSS_in(states_closure, nfa.transitions[i].state_to)){
                    dynarray_push(inspect_states, nfa.transitions[i].state_to);
                    SSS_add(&states_closure, nfa.transitions[i].state_to);
                }
            }
        }
    }

    dynarray_destroy(inspect_states);

    return states_closure;
}

int* NFA_transition_function(FA nfa, int state, char c){
    int* out_transitions = dynarray_create(int);
    for(int i = 0;i<dynarray_length(nfa.transitions);i++){
        if(nfa.transitions[i].state_from == state && nfa.transitions[i].trans_char == c){
            dynarray_push(out_transitions, nfa.transitions[i].state_to);
        }
    }

    return out_transitions;
}

SSubSet delta(FA nfa, SSubSet q, char c){
    SSubSet delta_out = SSS_initialize_empty(len_nfa_states(nfa));
    int* q_list = SSS_to_list(q);
    for(int i = 0;i<q.length;i++){
        int* state_transitions = NFA_transition_function(nfa, q_list[i], c);
        for(int j = 0;j<dynarray_length(state_transitions);j++){
            SSS_add(&delta_out, state_transitions[j]);
        }

        dynarray_destroy(state_transitions);
    }

    return delta_out;
}

void NtoDFA(FA nfa){

    int alphabet_length = 0;
    for(int i = 0;i<256;i++){
        if(nfa.alphabet[i] == true){
            alphabet_length++;
        }
    }

    char* alphabet_list = char_b_table_to_list(nfa.alphabet, 256);

    SSubSet n0 = SSS_initialize(len_nfa_states(nfa), &nfa.initial_state, 1);
    SSubSet q0 = e_closure(nfa, n0);
    
    SSubSet* Q = dynarray_create(SSubSet);
    SSubSet** T = dynarray_create(SSubSet*);
    SSubSet* worklist = dynarray_create(SSubSet);

    dynarray_push(Q, q0);
    dynarray_push(worklist, q0);

    while(dynarray_length(worklist) > 0){
        SSubSet q;
        dynarray_pop(worklist, &q);

        SSubSet* q_slot = malloc(alphabet_length * sizeof(SSubSet));
        dynarray_push(T, q_slot);

        for(int i = 0;i<alphabet_length; i++){
            char c = alphabet_list[i];
            SSubSet t = e_closure(nfa, delta(nfa, q, c));

            q_slot[i] = t;
            printf("Char %c\n", c);
            SSS_print(t);

            if(t.states_count > 0 && !SSS_list_in(Q, t)){
                printf("Add\n");
                dynarray_push(Q, t);
                dynarray_push(worklist, t);
            }
        }
    }

    
}


// final test ->  "(((ab|abc|a)*(bc|c|cde)*)*((abc|ab|(a|b|c)*)*(de|d|(ab|abc)*)*)|((ab|abc|ababc)*((bc|c|cde)*|(a|b|c)*))*)*"

int main() {
    printf("Scanner...\n");

    FA nfa;
    FA_initialize(&nfa);
    char regex[] = "a(b|c)*";
    Fragment fragment_start = {0, strlen(regex)};
    find_split_point(&nfa, regex, fragment_start, false, true);

    FA_print(nfa);

    NtoDFA(nfa);

    //int states[1] = {0};

    //SSubSet ss_states = SSS_initialize(len_nfa_states(nfa), states, 1);
    
    //SSubSet delta_test = delta(nfa, ss_states, 'a');
    //SSS_print(delta_test);

    //SSubSet test_closure = e_closure(nfa, delta_test);
    //SSS_print(test_closure);
    


    //Fragment fragment_start1 = {0, 4};
    //find_split_point(&nfa, regex, fragment_start1, false);
    //Fragment fragment_start2 = {5, 4};
    //find_split_point(&nfa, regex, fragment_start2, false);

    printf("\nStructs...\n");
}