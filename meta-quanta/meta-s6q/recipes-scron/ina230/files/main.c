#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "ina230.h"

int main(int argc, char *argv[])
{
    int bus = 0;
    int address = 0;

    bus = atoi(argv[1]);
    address = atoi(argv[2]);

    printf("Debug: Bus=%2d, Address=%2d\n", bus, address);

    if (ina230_init(bus, address) != SUCCESS)
    {
        exit(1);
    }

    exit(0);
}
