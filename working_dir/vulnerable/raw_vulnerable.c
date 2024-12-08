#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
int FLAG = 1;
const char *SAFE = "SAFE\0";
const char *UNSAFE = "UNSAFE\0";
char output_str[2048];

// Custom print function
void vulnerable_put(char *str)
{
    strcat(output_str, str);
}

// Modify printf calls to use vulnerable_put
void UAF_function(void *buffer_1)
{
    memcpy(buffer_1 + 16, UNSAFE, 8);
    memcpy(buffer_1 + 32, UNSAFE, 8);
}

void secret_stack_function()
{
    vulnerable_put("Oops! You are hacked!\n");
    printf("%s\n", output_str);
    exit(0);
}

void vulnerable_heap_function()
{
    vulnerable_put("\n---testing vulnerable_heap_function---\n");
    void *buffer_1 = malloc(8);
    void *buffer_2 = malloc(8);

    memcpy(buffer_2, SAFE, 8);
    free(buffer_1);
    vulnerable_put("SAFE before UAF: ");
    vulnerable_put((char *)buffer_2);
    vulnerable_put("\n");
    if (memcmp(buffer_2, SAFE, 8) != 0)
        printf("ERROR");

    UAF_function(buffer_1);

    vulnerable_put("SAFE after UAF: ");
    vulnerable_put((char *)buffer_2);
    free(buffer_2);
    if (memcmp(buffer_2, UNSAFE, 8) != 0)
        printf("ERROR");

    vulnerable_put("\n");
}

void vulnerable_stack_function()
{
    uintptr_t secret_addr = (uintptr_t)&secret_stack_function;
    
    vulnerable_put("\n---testing vulnerable_stack_function---\n");
    char buffer[64];
    vulnerable_put("Please do not print anything longer than 64 characters!\n");
    vulnerable_put("Your input: ");
    
    for (int i = 0; i < 64; i++) {
        buffer[i] = 'A';
    }
    memcpy(buffer + 72, &secret_addr, sizeof(secret_addr));
    vulnerable_put(buffer);
}


int main()
{
    performance_test();
    vulnerable_heap_function();
    vulnerable_stack_function();
    //Not reachable
    vulnerable_put("Back in main, program will now exit normally.\n");
    return 0;
}