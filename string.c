#include <stdio.h>

#define MAX_STR_LEN 50
#define NUM_STRINGS 5

int main() {
    // Declare and initialize an array of character arrays (strings)
    char strings[NUM_STRINGS][MAX_STR_LEN] = {
        "Hello",
        "World",
        "How",
        "Are",
        "You"
    };

    // Print the strings
    for (int i = 0; i < NUM_STRINGS; i++) {
        printf("%s\n", strings[i]);
    }

    return 0;
}