#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "structs.h"

#define MAX_REGEX_LENGTH 32

#define ALT_PRIORITY 0
#define CONCAT_PRIORITY 1
#define CLOS_PRIORITY 2

typedef struct NFA{
    int* states;
    char* alphabet;
    Transition* transitions;
    int* acceptable_states;

    int head;
    int tail;

    int state_amount;
    int transition_amount;
    int acceptable_state_amount;

} NFA;

int NFA_next_state(NFA *nfa){
    if(nfa->state_amount == 0){
        nfa->states = malloc(1*sizeof(int));
        (nfa->states)[0] = 0;
    }
    else{
        nfa->states = realloc(nfa->states, 1*sizeof(int));
    }
}

typedef struct Fragment{
    int start_index;
    int end_index;
} Fragment;

bool altercation(Fragment *left_fragment, Fragment *right_fragment, int *priority, int depth, int i){
    if(ALT_PRIORITY < *priority && depth == 0){
        left_fragment->end_index = i;
        right_fragment->start_index = i+1;
        *priority = ALT_PRIORITY;
        return true;
    }
    return false;
}

bool concatenation(Fragment fragment, Fragment *left_fragment, Fragment *right_fragment, int *priority, int depth, int i, char next_char){
    if(CONCAT_PRIORITY < *priority && depth == 0){
        if(i >= fragment.start_index+1){
            left_fragment->end_index = i;
            right_fragment->start_index = i;
            *priority = CONCAT_PRIORITY;
            return true;
        }
        else{
            if(next_char != '\0' && next_char != '*'){
                left_fragment->end_index = i+1;
                right_fragment->start_index = i+1;
                *priority = CONCAT_PRIORITY;
                return true;
            } 
        }
    }
    return false;
}

bool closure(Fragment fragment, Fragment *left_fragment, Fragment *right_fragment, int *priority, bool *final_split, int depth, bool split_found, int i){
    if(CLOS_PRIORITY < *priority && depth == 0){
        if(i == fragment.end_index-1 && split_found == false){
            left_fragment->end_index = i;
            *final_split = true;
            *priority = CLOS_PRIORITY;
            return true;
        }
        else{
            left_fragment->end_index = i;
            right_fragment->start_index = i+1;
            *priority = CLOS_PRIORITY;
            return true;
        }
    }
    return false;
}

bool parenthesis(Fragment fragment, Fragment *left_fragment, bool *final_split, bool split_found, int i){
    if(i == fragment.end_index-1 && split_found == false){
        left_fragment->start_index += 1;
        left_fragment->end_index = i;
        *final_split = true;
        return true;
    }
    return false;
}


int find_split_point(char *str, Fragment fragment, bool recursion){
    Fragment left_fragment = {fragment.start_index, fragment.start_index};
    Fragment right_fragment = {fragment.end_index, fragment.end_index};

    int min_priority = 3;
    int parenthesis_depth = 0;

    bool final_split = false;
    bool split_found = false;
    bool found_solution = false;

    for(int i = fragment.start_index;i<fragment.end_index;i++){
        printf("%c", str[i]);
    }
    printf("\n");

    if(fragment.start_index == fragment.end_index-1){
        return 0;
    }

    else{
        for(int i = fragment.start_index;i<fragment.end_index;i++){
            char current_char = str[i];
            if(current_char == '|'){
                found_solution = altercation(&left_fragment, &right_fragment, &min_priority, parenthesis_depth, i);
            }
            else if(current_char == '*'){
                found_solution = closure(fragment, &left_fragment, &right_fragment, &min_priority, &final_split, parenthesis_depth, split_found, i);
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

            if(found_solution == true){
                split_found = true;
            }
    }
    }

    if (parenthesis_depth != 0){
        printf("Parenthesis Mismatch -> %d", parenthesis_depth);
        return 0;
    }

    if(recursion == true){
        if(final_split == true){
            find_split_point(str, left_fragment, true);
        }
        else{
            find_split_point(str, left_fragment, true);
            find_split_point(str, right_fragment, true);
        }
    }
}

int main() {
    printf("Scanner...\n");
    char regex[] = "(((ab|abc|a)*(bc|c|cde)*)*((abc|ab|(a|b|c)*)*(de|d|(ab|abc)*)*)|((ab|abc|ababc)*((bc|c|cde)*|(a|b|c)*))*)*";
    Fragment fragment_start = {0, strlen(regex)};
    find_split_point(regex, fragment_start, true);

    printf("Structs...\n");

    Array arr;
    int t[5]  = {1,2,3,4,5};
    initialize_int(&arr, t, 5);
    print_int("%d", arr);
    return 0;
}