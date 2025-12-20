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

typedef struct Transition{
    int state_from;
    int state_to;
    char trans_char;
} Transition;

typedef struct NFA{
    int* states;
    char* alphabet;
    Transition* transitions;
    int* acceptable_states;

    int head;
    int tail;

} NFA;

int NFA_initialize(NFA *nfa){
    nfa->states = dynarray_create(int);
    nfa->alphabet = dynarray_create(char);
    nfa->transitions = dynarray_create(Transition);
    nfa->acceptable_states = dynarray_create(int);
}

int NFA_next_state(NFA *nfa){
    int next_int = dynarray_length(nfa->states);
    dynarray_push(nfa->states, next_int);

    return next_int;
}

Transition NFA_add_transition(NFA *nfa, int _from, int _to, char _trans_char){
    Transition trans;
    trans.state_from = _from;
    trans.state_to = _to;
    trans.trans_char = _trans_char;
    dynarray_push(nfa->transitions, trans);
    dynarray_push(nfa->alphabet, _trans_char);

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

int concatenation(Fragment fragment, Fragment *left_fragment, Fragment *right_fragment, int *priority, int depth, int i, char next_char){
    if(CONCAT_PRIORITY < *priority && depth == 0){
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
    if(pass_priority <= *priority && depth == 0){
        if(i == fragment.end_index-1){
            left_fragment->end_index = i;
            *final_split = true;
            *priority = pass_priority;
            return true;
        }
        else{
            //printf("EARLY\n");
            //left_fragment->end_index = i;
            //right_fragment->start_index = i+1;
            //*priority = pass_priority;
            //return true;
        }
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


int find_split_point(NFA* nfa, char* str, Fragment fragment, bool recursion){
    Fragment left_fragment = {fragment.start_index, fragment.start_index};
    Fragment right_fragment = {fragment.end_index, fragment.end_index};

    int min_priority = 4;
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
        Transition trans = NFA_add_transition(nfa, state_head, state_tail, str[fragment.start_index]);
        return 0;
    }

    else{
        for(int i = fragment.start_index;i<fragment.end_index;i++){
            char current_char = str[i];
            if(current_char == '|'){
                found_solution = altercation(&left_fragment, &right_fragment, &min_priority, parenthesis_depth, i);
            }
            else if(current_char == '*'){
                found_solution = closure(fragment, &left_fragment, &right_fragment, &min_priority, &final_split, parenthesis_depth, split_found, CLOS_PRIORITY, i);
            }
            else if(current_char == '$'){
                found_solution = closure(fragment, &left_fragment, &right_fragment, &min_priority, &final_split, parenthesis_depth, split_found, FIN_PRIORITY, i);
            }
            else if(current_char == '('){
                found_solution = concatenation(fragment, &left_fragment, &right_fragment, &min_priority, parenthesis_depth, i, '\0');
                parenthesis_depth += 1;
            }
            else if(current_char == ')'){
                parenthesis_depth -= 1;
                found_solution = parenthesis(fragment, &left_fragment, &final_split, split_found, i);
            }
            else{
                found_solution = concatenation(fragment, &left_fragment, &right_fragment, &min_priority, parenthesis_depth, i, str[i+1]);
            }

            if(found_solution > 0){
                split_found = true;
            }
    }
    }

    if (parenthesis_depth != 0){
        printf("Parenthesis Mismatch -> %d", parenthesis_depth);
        return 0;
    }

    //printf("(%d, %d) (%d, %d)\n",left_fragment.start_index, left_fragment.end_index, right_fragment.start_index, left_fragment.end_index);

    if(recursion == true){
        if(final_split == true){
            find_split_point(nfa, str, left_fragment, true);
        }
        else{
            switch(min_priority){
                case ALT_PRIORITY:
                    printf("ALTERCATION\n");
                    break;
                case CONCAT_PRIORITY:
                    printf("CONCATENATION\n");
                    break;
                default:
                    printf("Something is not right\n");
            }
            find_split_point(nfa, str, left_fragment, true);
            find_split_point(nfa, str, right_fragment, true);
        }
    }
}
// final test ->  "(((ab|abc|a)*(bc|c|cde)*)*((abc|ab|(a|b|c)*)*(de|d|(ab|abc)*)*)|((ab|abc|ababc)*((bc|c|cde)*|(a|b|c)*))*)*"

int main() {
    printf("Scanner...\n");

    NFA nfa;
    NFA_initialize(&nfa);
    char regex[] = "(((ab|abc|a)*(bc|c|cde)*)*((abc|ab|(a|b|c)*)*(de|d|(ab|abc)*)*)|((ab|abc|ababc)*((bc|c|cde)*|(a|b|c)*))*)*";
    Fragment fragment_start = {0, strlen(regex)};
    find_split_point(&nfa, regex, fragment_start, true);


    //Fragment fragment_start1 = {0, 4};
    //find_split_point(&nfa, regex, fragment_start1, false);
    //Fragment fragment_start2 = {5, 4};
    //find_split_point(&nfa, regex, fragment_start2, false);

    printf("Structs...\n");
}