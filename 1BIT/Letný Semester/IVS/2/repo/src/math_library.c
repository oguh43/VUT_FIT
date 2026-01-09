/**
 * @file math_library.c
 * @brief Houses all used math functions.
 * @author Hugo Bohácsek (xbohach00)
 * @author Filip Jenis (xjenisf00)
 */
// emcc math_library.c -O2 -s WASM=1 -s SIDE_MODULE=1 -o math_library.wasm
#include <math.h>
#include <stdio.h>
#include <emscripten/emscripten.h>

/**
 * @brief Adds two numbers together.
 * @param num1 The first number.
 * @param num2 The second number.
 * @return The sum of num1 and num2.
 */
EMSCRIPTEN_KEEPALIVE double addNumbers(double num1, double num2){
    return num1 + num2;
}

/**
 * @brief Subtracts two numbers.
 * @param num1 The first number.
 * @param num2 The second number.
 * @return The subtraction of num1 and num2.
 */
EMSCRIPTEN_KEEPALIVE double subtractNumbers(double num1, double num2){
    return num1 - num2;
}

/**
 * @brief Multiply two numbers.
 * @param num1 The first number.
 * @param num2 The second number.
 * @return The multiplication of num1 and num2.
 * @todo Division by zero should not return NaN, but should raise an error.
 */
EMSCRIPTEN_KEEPALIVE double multiplyNumbers(double num1, double num2){
    return num1 * num2;
}

/**
 * @brief Divide two numbers.
 * @param num1 The first number.
 * @param num2 The second number.
 * @return The divident of num1 and num2.
 * @todo Division by zero should not return NaN, but should raise an error.
 */
EMSCRIPTEN_KEEPALIVE double divideNumbers(double num1, double num2){
    if (num2 == 0){
        return NAN;
    }
    return num1 / num2;
}

/**
 * @brief Return the natural logarithm of a given number.
 * @param num Number.
 * @return The natural logarithm of num.
 */
EMSCRIPTEN_KEEPALIVE double lnNumber(double num){
    double res = log(num);
    return res;
}

/**
 * @brief Return the logarithm of a given number.
 * @param num1 Number.
 * @param num1 Base.
 * @return The logarithm of num1 at base num2.
 */
EMSCRIPTEN_KEEPALIVE double logNumber(double num1, double num2){
    double res = divideNumbers(lnNumber(num2), lnNumber(num1));
    return res;
}

/**
 * @brief Return the natural exponent of a given number.
 * @param num Number.
 * @return The natural exponent of num.
 */
EMSCRIPTEN_KEEPALIVE double expNumber(double num){
    double res = exp(num);
    return res;
}

/**
 * @brief Return a number rounded to the closest whole number.
 * @param num Number.
 * @return The rounded number num.
 */
EMSCRIPTEN_KEEPALIVE int roundNumber(double num){
    int low = num;
    int high = addNumbers(num, 1);
    if (num - low < high - num){
        return low;
    }
    return high;
}

/**
 * @brief Return an absolute value of a number.
 * @param num Number.
 * @return The absolute number num.
 */
EMSCRIPTEN_KEEPALIVE double absNumber(double num){
    if (num < 0){
        return multiplyNumbers(num, -1);
    }
    return num;
}

/**
 * @brief Return factorial of given number.
 * @param num Number.
 * @return Factorial of num.
 */
EMSCRIPTEN_KEEPALIVE double factorialNumber(int num) {
    if (num < 0){
        return -1;
    } else if (num == 0){
        return 1;
    }
    double result = 1; 
    for (int i = 1; i <= num; i++) { 
        result = multiplyNumbers(result, i);
    } 
    return result; 
}

/**
 * @brief Return base raised to a power.
 * @param base Base of the power.
 * @param exponent Exponent determines how many times will the base be multiplied.
 */
EMSCRIPTEN_KEEPALIVE double powerNumber(double base, double exponent){
    if (base == 0){
        if (exponent > 0){
            return 0;
        } else if (exponent == 0){
            return 1;
        } else {
            return INFINITY;
        }
    }
    if (exponent == 0){
        return 1;
    } else if (exponent < 0){
        return divideNumbers(1, powerNumber(base, absNumber(exponent)));
    } else {
        return pow(base, exponent);
    }
}

/**
 * @brief Return index root of a number.
 * @param num Number (radicand).
 * @param index Index of the root.
 * @return Root of number num.
 * @todo Raise an error instead of returning NaN in error cases.
 */
EMSCRIPTEN_KEEPALIVE double rootNumber(double num, double index){
    if (index == 0){
        return NAN;
    } else if (fmod(index, 2) == 0 && num < 0){
        return NAN;
    } else {
        return powerNumber(num, divideNumbers(1, index));
    }
}
