#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define MAX_REGEX_LENGTH 32

#define ALT_PRIORITY 0
#define CONCAT_PRIORITY 1
#define CLOS_PRIORITY 2


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

    int state_amount;
    int transition_amount;
    int acceptable_states_amount;

} NFA;

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
                if(CLOS_PRIORITY < min_priority && parenthesis_depth == 0){
                    if(i == fragment.end_index-1 && split_found == false){
                        final_split = true;
                        left_fragment.end_index = i;
                        min_priority = CLOS_PRIORITY;
                        found_solution = true;
                    }
                    else{
                        left_fragment.end_index = i;
                        right_fragment.start_index = i+1;
                        min_priority = CLOS_PRIORITY;
                        found_solution = true;
                    }
                }
            }
            else if(current_char == '('){
                if(CONCAT_PRIORITY < min_priority && parenthesis_depth == 0){
                    if(i >= fragment.start_index+1){
                        left_fragment.end_index = i;
                        right_fragment.start_index = i;
                        min_priority = CONCAT_PRIORITY;
                        found_solution = true;
                    }
                }
                parenthesis_depth += 1;
            }
            else if(current_char == ')'){
                parenthesis_depth -= 1;
                if(i == fragment.end_index-1 && split_found == false){
                    final_split = true;
                    left_fragment.start_index += 1;
                    left_fragment.end_index = i;
                    found_solution = true;
                }
            }
            else{
                if(CONCAT_PRIORITY < min_priority && parenthesis_depth == 0){
                    if(i >= fragment.start_index+1){
                        left_fragment.end_index = i;
                        right_fragment.start_index = i;
                        min_priority = CONCAT_PRIORITY;
                        found_solution = true;
                    }
                    else{
                        if(str[i+1] != '*'){
                            left_fragment.end_index = i+1;
                            right_fragment.start_index = i+1;
                            min_priority = CONCAT_PRIORITY;
                            found_solution = true;
                        } 
                    }
                }
            }

            if(found_solution == true){
                split_found = true;
            }
    }
    }

    //printf("%d\n", final_split);
    //printf("(%d, %d)", start_index, left_index);
    //printf("(%d, %d)\n", right_index, end_index);
    //printf("(%c, %c)", str[start_index], str[left_index]);
    //printf("(%c, %c)\n", str[right_index], str[end_index-1]);

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
    char regex[] = "((((ab|abc|ababc)(bc|c|cde)*)|((ab|abc)*(bc|cde|de)))*(((abc|ab)(bc|cde)*)|((ab|abc)*(bc|c)))*((ab|abc|ababc)*(bc|c)*)*)*((((ab|abc)c*(de|d)|(abc|bc)*(ab|c))((ab|bc)d*|abc))*((ab|abc)d|c*))*(((abc|ab)(bc|cde)*)|(ab|bc)*(cd|de))*((ab|abc)*(bc|c))*)";
    //printf("%d\n", strlen(regex));
    Fragment fragment_start = {0, strlen(regex)};
    find_split_point(regex, fragment_start, true);
    return 0;
}