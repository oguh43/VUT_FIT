#!/usr/bin/env node

/** 
 * @module StandardDeviation
 * @description Calculates performance.
 * @author Štefan Dubnička (xdubnis00), Hugo Bohácsek (xbohach00) 
*/
const { addNumbers, subtractNumbers, multiplyNumbers, divideNumbers, lnNumber, logNumber, expNumber, roundNumber, absNumber, factorialNumber, powerNumber, rootNumber } = require('../math_library.js');
const fs = require("node:fs");
const {performance} = require("perf_hooks")

/**
 * Function to generate a table for the output data
 * 
 * @param {Array} data - Data to be formatted into a table.
 * @returns {String} Data formatted into a pseudo table.
*/
function generateTable(data) {
    const columnLengths = data.reduce((acc, row) => {
        row.forEach((cell, index) => {
            const len = cell.toString().length;
            acc[index] = Math.max(acc[index] || 0, len);
        });
        return acc;
    }, []);

    let table = '';
    data.forEach(row => {
        row.forEach((cell, index) => {
            table += cell.toString().padEnd(columnLengths[index] + 2);
        });
        table += '\n';
    });

    return table;
}

// Read input from stdin
stdin_data=fs.readFileSync(0, 'utf-8');


//process.stdout.write("\nTiming...\n")


// Whitespace detection
whitespace = "";
// Look for first appearence of a whitespace character 
for (let i = 0; i < stdin_data.length; i++) {
	if ((stdin_data[i] == "\t") || (stdin_data[i] == "\n") || (stdin_data[i] == " ")) {
		whitespace = stdin_data[i];
		break;
	}
}	

// Make input into list, splitting by the found whitespace
nums = stdin_data.split(whitespace);

// Init counter lists for timing the function calls
time_addNumbers = []
var add_start = 0
time_subtractNumbers = []
var subtract_start = 0
time_multiplyNumbers = []
var multiply_start = 0
time_divideNumbers = []
var divide_start = 0
time_rootNumber = []
var root_start


var stddev_start = performance.now()

stddev = 0;
std_sum = 0;

arith_sum = 0;

// Standard mean algorithm with extra writes to the timing variables 
// (xi)^2
for (let i = 0; i < nums.length; i++) {
	// Parse the input into floats if is right format.
	if (isNaN(parseFloat(nums[i]))) {
		nums.splice(i, 1);
		//console.log(nums);
	}
	else {
		nums[i] = parseFloat(nums[i]);
		multiply_start = performance.now()
		xi_squared = multiplyNumbers(nums[i], nums[i]);
		time_multiplyNumbers.push(performance.now() - multiply_start)
	
		//console.log("xi_squared: " + xi_squared);

		add_start = performance.now()
		std_sum = addNumbers(std_sum, xi_squared);
		time_addNumbers.push(performance.now()-add_start);

		add_start = performance.now()
		arith_sum = addNumbers(arith_sum, nums[i]);
		time_addNumbers.push(performance.now()-add_start);

		//console.log("std_sum: " + std_sum);
	}
}

// N^2
multiply_start = performance.now()
arith_sum = multiplyNumbers(arith_sum, arith_sum);
time_multiplyNumbers.push(performance.now() - multiply_start)

// N^2
multiply_start = performance.now()
n_squared = multiplyNumbers(nums.length, nums.length);
time_multiplyNumbers.push(performance.now() - multiply_start)

//console.log("N na druhu: " + n_squared)

// (xi)^2 / N
divide_start = performance.now()
arithmetic_sq = divideNumbers(arith_sum, n_squared);
time_divideNumbers.push(performance.now() - divide_start)

//console.log("arithmetic_sq: " + arithmetic_sq);

multiply_start = performance.now()
arithmetic_sq = multiplyNumbers(nums.length, arithmetic_sq);
time_multiplyNumbers.push(performance.now() - multiply_start)

//console.log("arithmetic_sq: " + arithmetic_sq);

// (xi)^2 - [(xi)^2 / N]
subtract_start = performance.now();
std_sum = subtractNumbers(std_sum, arithmetic_sq);
time_subtractNumbers.push(performance.now() - subtract_start)

// 1 / (N -1)
subtract_start = performance.now();
div1nm1 = subtractNumbers(nums.length, 1);
time_subtractNumbers.push(performance.now() - subtract_start)

// sqrt( [1 / (N -1)] * [(xi)^2 - [(xi)^2 / N]] )
divide_start = performance.now();
div1nm1 = divideNumbers(1, div1nm1);
time_divideNumbers.push(performance.now() - divide_start)

multiply_start = performance.now()
m = multiplyNumbers(div1nm1, std_sum)
time_multiplyNumbers.push(performance.now() - multiply_start)

root_start = performance.now()
roo = rootNumber(m, 2);
time_rootNumber.push(performance.now() - root_start)

stddev_start = performance.now() - stddev_start

//console.log("\nStandard Deviation value: " + roo + "\n");
console.log(roo);
//console.log(`Total time spent: ${stddev_start}ms\n`)


// Output data
const output_data = [
    ['Name', 'Total time spent', 'Time spent on average', 'Number of calls'],
    ['addNumbers()', `${time_addNumbers.reduce((pv, cv) => pv + cv, 0)}ms`, `${time_addNumbers.reduce((pv, cv) => pv + cv, 0)/time_addNumbers.length}ms`, `${time_addNumbers.length}`],
    ['substractNumbers()', `${time_subtractNumbers.reduce((pv, cv) => pv + cv, 0)}ms`, `${time_subtractNumbers.reduce((pv, cv) => pv + cv, 0)/time_subtractNumbers.length}ms`, `${time_subtractNumbers.length}`],
    ['multiplyNumbers()', `${time_multiplyNumbers.reduce((pv, cv) => pv + cv, 0)}ms`, `${time_multiplyNumbers.reduce((pv, cv) => pv + cv, 0)/time_multiplyNumbers.length}ms`, `${time_multiplyNumbers.length}`],
	['divideNumbers()', `${time_divideNumbers.reduce((pv, cv) => pv + cv, 0)}ms`, `${time_divideNumbers.reduce((pv, cv) => pv + cv, 0)/time_divideNumbers.length}ms`, `${time_divideNumbers.length}`],
	['rootNumber()', `${time_rootNumber.reduce((pv, cv) => pv + cv, 0)}ms`, `${time_rootNumber.reduce((pv, cv) => pv + cv, 0)/time_rootNumber.length}ms`, `${time_rootNumber.length}`]
];
// Generate and print the table
out_gen = generateTable(output_data);

//console.log(out_gen);



// Format the output file
out_form = "Standard Deviation value: " + roo + "\n\n" + `Total time spent: ${stddev_start}ms\n\n`;
out_form += out_gen;

// Write the formatted output to a file
fs.writeFileSync('./vystup.txt', out_form, err => {
	if (err) {
		console.error(err);
	} else {
		// success
	}
});

try {
	fs.writeFileSync('./vystup.txt', out_form);
} catch (err) {
	console.error(err);
}


process.exit(0);
