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

typedef struct Transition{
    int state_from;
    int state_to;
    char trans_char;
} Transition;

typedef struct NFA{
    int* states;
    char alphabet[256];
    Transition* transitions;
    int* acceptable_states;
} NFA;

void print_transition(Transition t){
    printf("State: ");
    printf("%d ", t.state_from);
    printf("- ");
    printf("%c ", t.trans_char);
    printf("-> State: ");
    printf("%d", t.state_to);
    printf("\n");
}

void NFA_print(NFA nfa){
    printf("-- States --\n");
    for(int i = 0; i < dynarray_length(nfa.states);i++){
        printf("%d, ", nfa.states[i]);
    }
    printf("\n");
    printf("-- Acceptable States --\n");
    for(int i = 0; i < dynarray_length(nfa.acceptable_states);i++){
        printf("%d, ", nfa.acceptable_states[i]);
    }
    printf("\n");
    printf("-- Alphabet --\n");
    for(int i = 0; i < 256;i++){
        if(nfa.alphabet[i] == true){
            printf("%c, ", (unsigned char) i);
        }
        
    }
    printf("\n");
    printf("-- Transitions --\n");
    for(int i = 0; i < dynarray_length(nfa.transitions);i++){
        print_transition(nfa.transitions[i]);
    }
    printf("\n");
}

int NFA_initialize(NFA *nfa){
    nfa->states = dynarray_create(int);
    nfa->transitions = dynarray_create(Transition);
    nfa->acceptable_states = dynarray_create(int);
    memset(nfa->alphabet, 0, 256);
}

int NFA_next_state(NFA *nfa){
    int next_int = dynarray_length(nfa->states);
    dynarray_push(nfa->states, next_int);

    return next_int;
}

void NFA_add_acceptable_state(NFA *nfa, int acceptable_state){
    dynarray_push(nfa->acceptable_states, acceptable_state);
}

Transition NFA_add_transition(NFA *nfa, int _from, int _to, char _trans_char){
    Transition trans;
    trans.state_from = _from;
    trans.state_to = _to;
    trans.trans_char = _trans_char;
    dynarray_push(nfa->transitions, trans);

    int int_repr = (unsigned char) _trans_char;
    nfa->alphabet[int_repr] = 1;

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


Fragment find_split_point(NFA* nfa, char* str, Fragment fragment, bool final_state, bool recursion){
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
        int state_head = NFA_next_state(nfa);
        int state_tail = NFA_next_state(nfa);

        if(final_state == true){
            NFA_add_acceptable_state(nfa, state_tail);
        }
        Transition trans = NFA_add_transition(nfa, state_head, state_tail, str[fragment.start_index]);
        Fragment char_fragment = {state_head, state_tail};
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

                    int state_head_alt = NFA_next_state(nfa);
                    int state_tail_alt = NFA_next_state(nfa);
                    
                    Transition trans_start = NFA_add_transition(nfa, state_head_alt, nfa_only_fragment.start_index, '@');
                    Transition trans_tail = NFA_add_transition(nfa, nfa_only_fragment.end_index, state_tail_alt, '@');
                    Transition trans_ret = NFA_add_transition(nfa, nfa_only_fragment.end_index, nfa_only_fragment.start_index, '@');
                    Transition trans_bounce = NFA_add_transition(nfa, state_head_alt, state_tail_alt, '@');

                    Fragment alt_fragment = {state_head_alt, state_tail_alt};

                    if(final_state == true){
                        NFA_add_acceptable_state(nfa, state_tail_alt);
                    }
                    return alt_fragment;
                case FIN_PRIORITY:
                    Fragment final_tag_fragment = find_split_point(nfa, str, left_fragment, true, true);
                    return final_tag_fragment;
                case MAX_PRIORITY:
                    Fragment same_fragment = find_split_point(nfa, str, left_fragment, false, true);
                    if(final_state == true){
                        NFA_add_acceptable_state(nfa, same_fragment.end_index);
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
                    int state_head_alt = NFA_next_state(nfa);
                    int state_tail_alt = NFA_next_state(nfa);

                    Transition trans_0 = NFA_add_transition(nfa, state_head_alt, nfa_left_fragment.start_index, '@');
                    Transition trans_1 = NFA_add_transition(nfa, state_head_alt, nfa_right_fragment.start_index, '@');
                    Transition trans_2 = NFA_add_transition(nfa, nfa_left_fragment.end_index, state_tail_alt, '@');
                    Transition trans_3 = NFA_add_transition(nfa, nfa_right_fragment.end_index, state_tail_alt, '@');

                    Fragment alt_fragment = {state_head_alt, state_tail_alt};

                    if(final_state == true){
                        NFA_add_acceptable_state(nfa, state_tail_alt);
                    }
                    return alt_fragment;
                    
                case CONCAT_PRIORITY:
                    //printf("CONCATENATION\n");
                    
                    Transition trans = NFA_add_transition(nfa, nfa_left_fragment.end_index, nfa_right_fragment.start_index, '@');
                    Fragment concat_fragment = {nfa_left_fragment.start_index, nfa_right_fragment.end_index};

                    if(final_state == true){
                        NFA_add_acceptable_state(nfa, nfa_right_fragment.end_index);
                    }
                    return concat_fragment;

                default:
                    printf("Something is not right\n");
            }
        }
    }
}
// final test ->  "(((ab|abc|a)*(bc|c|cde)*)*((abc|ab|(a|b|c)*)*(de|d|(ab|abc)*)*)|((ab|abc|ababc)*((bc|c|cde)*|(a|b|c)*))*)*"

int main() {
    printf("Scanner...\n");

    NFA nfa;
    NFA_initialize(&nfa);
    char regex[] = "(((ab|abc|a)$E*(bc|c|cde)*)*((abc|ab|(a|b|c)*)*(de|d|(ab|abc)*)*)|((ab|abc|ababc)*((bc|c|cde)*|(a|b|c)*))*)*";
    Fragment fragment_start = {0, strlen(regex)};
    find_split_point(&nfa, regex, fragment_start, false, true);

    NFA_print(nfa);

    //Fragment fragment_start1 = {0, 4};
    //find_split_point(&nfa, regex, fragment_start1, false);
    //Fragment fragment_start2 = {5, 4};
    //find_split_point(&nfa, regex, fragment_start2, false);

    printf("Structs...\n");
}