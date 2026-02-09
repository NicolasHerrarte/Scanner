## Highlights

* **Custom Regex Pre-processor**: Implements a `regex_prep` function that handles character class expansion (e.g., `[0-9]`, `[a-z]`) and handles escape character sequences for NFA construction.
* **Thompson’s Construction Algorithm**: Utilizes a recursive `find_split_point` function to transform expanded regular expressions into Non-deterministic Finite Automata (NFA) using epsilon transitions.
* **DFA Conversion via Subset Construction**: Features a robust `NtoDFA` implementation that applies $\epsilon$-closure and delta transition functions to convert NFAs into Deterministic Finite Automata.
* **Deterministic Lexical Analysis**: Provides optimized string and file-based tokenization by traversing DFA states and identifying the longest matching acceptable prefix.
* **Configurable Token Filtering**: Supports a category-based ignore system within the `scanner_loop` to automatically skip specific tokens during the scanning process.
* **State Machine Exporting**: Includes built-in tools to export NFA and DFA structures, including transition sets and alphabet tables, to external files for verification.
* **Strict Memory Control**: Built with manual memory management in C, leveraging a dynamic array interface for efficient storage of states and generated tokens.

## Overview

This implementation is based on Engineering A Compiler 2nd Edition by Cooper and Torczon. It is an educational project for anyone interested in Compiler Theory

## Usage

### 1. Generating a Tokenizer and Scanning Input
To create a scanner, you define a regular expression string and use `MakeFA`. This function processes the regex through the NFA and DFA stages automatically. You can then pass the resulting DFA to `scanner_loop_string` to tokenize an input.

```c

char *num_regex = "([0]|[1-9][0-9]*)$02|text$03|( ( )*)$01"; 
FA dfa = MakeFA(num_regex, "debug_log.txt", false);

char *input_text ="123 text  0 text 10101";
int ignore_space[] = {1};
Token* tokens = scanner_loop_string(dfa, input_text, ignore_space, 1);

print_token_seq(tokens);

```
### A file can also be used for scanning instead 
To create a scanner, you define a regular expression string and use `MakeFA`. This function processes the regex through the NFA and DFA stages automatically. You can then pass the resulting DFA to `scanner_loop_string` to tokenize an input.

```c
Token* tokens = scanner_loop_file(dfa, directory, ignore_space, 1);
```

## Important Recommendations and Warnings

### Category ID Constraints
When assigning categories to your regex patterns, **always use integer values greater than 0**. 

The internal scanning logic validates matches and identifies acceptable states by checking if `category > 0`. Using `0` or negative values (like `-1`) will cause the engine to ignore those states or treat them as invalid, preventing the scanner from ever returning those tokens.

### Priority and Conflict Resolution
In cases where two different regex classes (categories) could potentially produce the same token from the same input string, the engine resolves the conflict using a priority-based system:

* **Greater Number = Higher Priority**: If a lexeme matches multiple categories, the category with the **highest integer value** will be assigned to the token.
* **Example**: If Category `10` matches the string "if" as an identifier and Category `50` matches "if" as a keyword, the scanner will prioritize Category `50`.

---