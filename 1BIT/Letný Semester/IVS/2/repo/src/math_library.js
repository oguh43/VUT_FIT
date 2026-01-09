/** 
 * @module MathLibrary
 * @description Houses all used math functions.
 * @author Hugo Bohácsek (xbohach00) 
 * @author Filip Jenis (xjenisf00) 
*/
const fs = require('fs');
const path = require('path');

const wasmFilePath = path.join(__dirname, 'math_library.wasm');
Math.fmod = (a, b) => {return a % b}

function loadWasm() {
    const wasmBytes = fs.readFileSync(wasmFilePath);
    const wasmModule = new WebAssembly.Module(wasmBytes);
    const imports = { 
        env: {
            log: Math.log,
            exp: Math.exp,
            pow: Math.pow,
            fmod: Math.fmod
        }
    };
    const wasmInstance = new WebAssembly.Instance(wasmModule, imports);

    return wasmInstance.exports;
}


const wasmFunctions = loadWasm();

module.exports = {
    /**
     * Adds two numbers together.
     * 
     * @param {number} num1 - The first number.
     * @param {number} num2 - The second number.
     * @returns {number} The sum of num1 and num2.
     */
    addNumbers: wasmFunctions.addNumbers,
    /**
     * Subtracts two numbers.
     * 
     * @param {number} num1 - The first number.
     * @param {number} num2 - The second number.
     * @returns {number} The subtraction of num1 and num2.
     */
    subtractNumbers: wasmFunctions.subtractNumbers,
    /**
     * Multiply two numbers.
     * 
     * @param {number} num1 - The first number.
     * @param {number} num2 - The second number.
     * @returns {number} The multiplication of num1 and num2.
     */
    multiplyNumbers: wasmFunctions.multiplyNumbers,
    /**
     * Divide two numbers.
     * 
     * @param {number} num1 - The first number.
     * @param {number} num2 - The second number.
     * @returns {number} The divident of num1 and num2.
     * @todo Division by zero should not return NaN, but should raise an error.
     */
    divideNumbers: wasmFunctions.divideNumbers,
    /**
     * Return the natural logarithm of a given number.
     * 
     * @param {number} num1 - Number.
     * @returns {number} The natural logarithm of num1.
     */
    lnNumber: wasmFunctions.lnNumber,
    /**
     * Return the logarithm of a given number.
     * 
     * @param {number} num1 - Number.
     * @param {number} num1 - Base.
     * @returns {number} The logarithm of num1 at base num2.
     */
    logNumber: wasmFunctions.logNumber,
    /**
     * Return the natural exponent of a given number.
     * 
     * @param {number} num1 - Number.
     * @returns {number} The natural exponent of num1.
     */
    expNumber: wasmFunctions.expNumber,
    /**
     * Return a number rounded to the closest whole number.
     * 
     * @param {number} num1 - Number.
     * @returns {number} The rounded number num1.
     */
    roundNumber: wasmFunctions.roundNumber,
    /**
     * Return an absolute value of a number.
     * 
     * @param {number} num1 - Number.
     * @returns {number} The absolute number num1.
     */
    absNumber: wasmFunctions.absNumber,
    /**
     * Return factorial of given number.
     * 
     * @param {number} num - Number.
     * @returns {number} Factorial of num
     */
    factorialNumber: wasmFunctions.factorialNumber,
    /**
     * Return base raised to a power.
     * 
     * @param {number} base Base of the power 
     * @param {number} exponent Exponent determines how many times will the base be multiplied
     */
    powerNumber: wasmFunctions.powerNumber,
    /**
     * Return index root of a number
     * 
     * @param {number} num Number (radicand) 
     * @param {number} index Index of the root
     * @returns Root of number num.
     * @todo Division by zero should not return NaN, but should raise an error.
     */
    rootNumber: wasmFunctions.rootNumber
};

