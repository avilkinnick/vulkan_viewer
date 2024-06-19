#ifndef BASE_H
#define BASE_H

typedef enum Result
{
    SUCCESS,
    FAIL
} Result;

void printError(const char* pFormat, ...);

#endif // BASE_H
