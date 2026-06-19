#include "lexer.h"
#include "parser.h"
#include "vm.h"
#include <stdio.h>

int main()
{
    vm_init();

    char line[1024];

    while(1)
    {
        printf("pew> ");

        if(!fgets(line,sizeof(line),stdin))
            break;

        execute_source(line);
    }

    return 0;
}