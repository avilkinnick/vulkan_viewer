#include <stdlib.h>

#include "Application.h"

int main(int argc, char* argv[])
{
    Application application;
    if (createApplication(&application) != SUCCESS)
    {
        printError("Failed to create application!");
        return EXIT_FAILURE;
    }

    destroyApplication(&application);

    return EXIT_SUCCESS;
}
