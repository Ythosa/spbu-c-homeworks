#include "binary.h"

#define BITS_IN_BYTE 8

struct BinaryNumber {
    size_t size;
    bool* bits;
};

BinaryNumber* binaryNumberCreate(size_t size)
{
    BinaryNumber* binaryNumber = malloc(sizeof(BinaryNumber));

    binaryNumber->size = size;
    binaryNumber->bits = calloc(size, sizeof(bool));

    return binaryNumber;
}

void binaryNumberFree(BinaryNumber* binaryNumber)
{
    free(binaryNumber->bits);
    free(binaryNumber);
}

void binaryNumberLeftShift(BinaryNumber* binaryNumber, size_t bitsCount)
{
    for (size_t i = 0; i < binaryNumber->size; i++) {
        size_t bitPosition = i + bitsCount;
        if (bitPosition < binaryNumber->size)
            binaryNumber->bits[i] = binaryNumber->bits[bitPosition];
        else
            binaryNumber->bits[i] = 0;
    }
}

void binaryNumberRightShift(BinaryNumber* binaryNumber, size_t bitsCount)
{
    for (int i = (int)binaryNumber->size - 1; i >= 0; i--) {
        int bitPosition = i - (int)bitsCount;
        if (bitPosition >= 0)
            binaryNumber->bits[i] = binaryNumber->bits[bitPosition];
        else
            binaryNumber->bits[i] = 0;
    }
}

BinaryNumber* binaryNumberMultiply(BinaryNumber* leftOperand, BinaryNumber* rightOperand)
{
    if (leftOperand->size != rightOperand->size) {
        return NULL;
    }

    BinaryNumber* result = binaryNumberCreate(leftOperand->size);

    while (binaryNumberToInteger(rightOperand) != 0) {
        if (rightOperand->bits[rightOperand->size - 1] == 1) {
            BinaryNumber* newResult = binaryNumberAdd(result, leftOperand);
            binaryNumberFree(result);
            result = newResult;
        }

        binaryNumberLeftShift(leftOperand, 1);
        binaryNumberRightShift(rightOperand, 1);
    }

    return result;
}

void binaryNumberInvert(BinaryNumber* binaryNumber)
{
    for (int i = 0; i < binaryNumber->size; i++)
        binaryNumber->bits[i] = 1 - binaryNumber->bits[i];
}

BinaryNumber* binaryNumberFromInteger(int number)
{
    BinaryNumber* binaryNumber = binaryNumberCreate(sizeof(number) * BITS_IN_BYTE);

    int numberAbs = abs(number);
    for (int i = 0; numberAbs > 0; i++) {
        binaryNumber->bits[binaryNumber->size - i - 1] = numberAbs % 2;
        numberAbs /= 2;
    }

    if (number < 0) {
        binaryNumberInvert(binaryNumber);
        BinaryNumber* one = binaryNumberFromInteger(1);
        BinaryNumber* binaryNumberPlusOne = binaryNumberAdd(binaryNumber, one);
        binaryNumberFree(one);
        binaryNumberFree(binaryNumber);
        binaryNumber = binaryNumberPlusOne;
    }

    return binaryNumber;
}

int binaryNumberToInteger(BinaryNumber* binaryNumber)
{
    int integerNumber = 0;
    int currentTwoPower = 1;
    for (int i = 0; i < binaryNumber->size; i++) {
        integerNumber += currentTwoPower * binaryNumber->bits[binaryNumber->size - i - 1];
        currentTwoPower *= 2;
    }

    return integerNumber;
}

BinaryNumber* binaryNumberAdd(BinaryNumber* leftOperand, BinaryNumber* rightOperand)
{
    if (leftOperand->size != rightOperand->size)
        return NULL;

    BinaryNumber* resultBinaryNumber = binaryNumberCreate(leftOperand->size);
    int buf = 0;
    for (int i = (int)resultBinaryNumber->size - 1; i >= 0; i--) {
        int s = leftOperand->bits[i] + rightOperand->bits[i];
        resultBinaryNumber->bits[i] = (s + buf) % 2;
        buf = (s + buf) / 2;
    }

    return resultBinaryNumber;
}

void binaryNumberPrint(BinaryNumber* binaryNumber, char* end, FILE* destination)
{
    for (int i = 0; i < binaryNumber->size; i++)
        fprintf(destination, "%d ", binaryNumber->bits[i]);

    if (end)
        fprintf(destination, "%s", end);
}
