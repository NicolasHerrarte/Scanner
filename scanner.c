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


int find_split_point(char *str, int start_index, int end_index, bool recursion){
    int left_index = start_index;
    int right_index = end_index-1;
    int min_priority = 3;
    int parenthesis_depth = 0;

    bool final_split = false;
    bool split_found = false;

    for(int i = start_index;i<end_index;i++){
        printf("%c", str[i]);
    }
    printf("\n");

    if(start_index == end_index-1){
        return 0;
    }

    else{
        for(int i = start_index;i<end_index;i++){
        char current_char = str[i];
        if(current_char == '|'){
            if(ALT_PRIORITY < min_priority && parenthesis_depth == 0){
                left_index = i;
                right_index = i+1;
                min_priority = ALT_PRIORITY;
                split_found = true;
            }
        }
        else if(current_char == '*'){
            if(CLOS_PRIORITY < min_priority && parenthesis_depth == 0){
                if(i == end_index-1 && split_found == false){
                    final_split = true;
                    left_index = i;
                    min_priority = CLOS_PRIORITY;
                }
                else{
                    left_index = i;
                    right_index = i+1;
                    min_priority = CLOS_PRIORITY;
                    split_found = true;
                }
            }
        }
        else if(current_char == '('){
            if(CONCAT_PRIORITY < min_priority && parenthesis_depth == 0){
                if(i >= start_index+1){
                    left_index = i;
                    right_index = i;
                    min_priority = CONCAT_PRIORITY;
                    split_found = true;
                }
            }
            parenthesis_depth += 1;
        }
        else if(current_char == ')'){
            parenthesis_depth -= 1;
            if(i == end_index-1 && split_found == false){
                final_split = true;
                start_index += 1;
                left_index = i;
            }
        }
        else{
            if(CONCAT_PRIORITY < min_priority && parenthesis_depth == 0){
                if(i >= start_index+1){
                    left_index = i;
                    right_index = i;
                    min_priority = CONCAT_PRIORITY;
                    split_found = true;
                }
                else{
                    if(str[i+1] != '*'){
                        left_index = i+1;
                        right_index = i+1;
                        min_priority = CONCAT_PRIORITY;
                        split_found = true;
                    } 
                }
            }
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
            find_split_point(str, start_index, left_index, true);
        }
        else{
            find_split_point(str, start_index, left_index, true);
            find_split_point(str, right_index, end_index, true);
        }
    }
}

int main() {
    printf("Scanner...\n");
    char regex[] = "((((ab|abc|ababc)(bc|c|cde)*)|((ab|abc)*(bc|cde|de)))*(((abc|ab)(bc|cde)*)|((ab|abc)*(bc|c)))*((ab|abc|ababc)*(bc|c)*)*)*((((ab|abc)c*(de|d)|(abc|bc)*(ab|c))((ab|bc)d*|abc))*((ab|abc)d|c*))*(((abc|ab)(bc|cde)*)|(ab|bc)*(cd|de))*((ab|abc)*(bc|c))*)";
    //printf("%d\n", strlen(regex));
    find_split_point(regex, 0, strlen(regex), true);
    return 0;
}