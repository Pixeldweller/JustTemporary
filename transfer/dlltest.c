// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jabcode.h"

double WINAPI myCexport(double x)
{
    return x * 2;
}

int test(int x)
{
    saveImage(NULL, (jab_char*)"Test");

    return 0;
}
