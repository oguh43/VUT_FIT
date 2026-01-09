/** 
 * @module Preload
 * @description Main js file that controls the calculator.
 * @author Hugo Bohácsek (xbohach00), Lucia Klčová (xklcovl00)
*/

const { addNumbers, subtractNumbers, multiplyNumbers, divideNumbers, lnNumber, logNumber, expNumber, roundNumber, absNumber, factorialNumber, powerNumber, rootNumber } = require("./math_library.js");
const {Titlebar} = require("custom-electron-titlebar");
const {shell} = require('electron');
var lastRes = "";
var errorMsg = "";
var pdfOpen = false;
var pdfWindow = null;
var pdfTimer = null;
window.addEventListener('DOMContentLoaded', () => {
	new Titlebar({
		icon: "./calc_ico.png",
		maximizable: false,
		enableMnemonics: false,
		menu: {}
	})
	document.getElementsByClassName("cet-menubar")[0].remove()
	const {MathfieldElement, convertLatexToMarkup} = require("mathlive");

	const history = document.getElementById("history");

	const mfe = new MathfieldElement();
	mfe.mathVirtualKeyboardPolicy = "sandboxed";
	document.body.appendChild(mfe);
	mfe.addEventListener("focusout", () => {mfe.focus()});
	mfe.focus();
	mfe.addEventListener("beforeinput", (e)=>{processBeforeInput(e, mfe)});
	const inpEl = document.getElementsByTagName("math-field")[0];
	inpEl.style.width = "100%";
	mathVirtualKeyboard.show();
	layout = {
		displayEditToolbar : false,
		rows: [
			[{latex: "+", label: "&#x2b;", class: "big-op hide-shift"}, { latex: '-', label: '&#x2212;', class: 'big-op hide-shift'}, {latex: "\\times", label: "&times;", class: "big-op hide-shift"}, {latex: "\\frac{#@}{#?}", class: "small"},  "\\sqrt[#?]{#@}", "#@^{#?}"],
			["[7]", "[8]", "[9]", "e^{#@}", "\\ln#@", {latex: "\\log_{#@}#?", class: "small"}],
			["[4]", "[5]", "[6]", "!", {latex:"(", label:"(", class: "hide-shift"}, {latex:")", label:")", class: "hide-shift"}],
			["[1]", "[2]", "[3]", "\\left|#@\\right|", "[left]", "[right]"],
			[{latex: "0", label: "0", class: "hide-shift"}, {latex: ".", label: ".", class: "hide-shift"}, {label: "?", latex: "?", class: "tex"}, "\\left\\lfloor#@\\right\\rfloor", {label: "[backspace]", width: 1}, {label: "[return]", width: 1}]
		]
	};
	mathVirtualKeyboard.layouts = layout;

	document.addEventListener("click", (e)=>{processClick(e, mfe, history, convertLatexToMarkup, inpEl)});

	mfe.addEventListener("keydown", (e) => (handleKeyDown(e, mfe, history, convertLatexToMarkup, inpEl)), { capture: true });

	window.addEventListener("resize", ()=>{repositionInput(inpEl)});
	setTimeout(()=>{repositionInput(inpEl)}, 500);

	document.getElementsByTagName("math-field")[0].addEventListener("onblur", ()=>{
		this.focus();
	});
	window.addEventListener("keyup", handleKeyUp);
});

/**
 * Create a new window with the documentation.
*/
function showPdf() {
	if (pdfOpen){
		return;
	}
	pdfWindow = window.open('../dokumentace.pdf')
	if (!pdfWindow){
		return;
	}
	pdfOpen = true;
	pdfTimer = setInterval(function() { 
		if(pdfWindow.closed) {
			clearInterval(pdfTimer);
			pdfOpen = false;
		}
	}, 1000);
}

/**
 * Processes a before input event.
 * 
 * @param {HTMLEvent} e - Event to be processed.
 * @param {HTMLElement} mfe - MathfieldElement to be processed.
*/
function processBeforeInput(e, mfe){
	if (e.data == "?"){
		showPdf()
		e.preventDefault();
		return;
	}
	if ([/*"!", */"\\times", "-", "+", "\\frac{#@}{#?}"/*, "\\left\\lfloor#@\\right\\rfloor", "\\left|#@\\right|", "#@^{#?}", "\\sqrt[#?]{#@}", "e^{#@}", "\\ln#@", "\\log_{#@}#?"*/].includes(e.data) && mfe.value == ""){
		if (lastRes == ""){
			return;
		} else if (Number.isFinite(lastRes)){
			mfe.value = lastRes.toString();
		} else if (lastRes == "Syntax Error!"){
			return;
		} else if (lastRes == NaN || lastRes == Infinity || lastRes == -Infinity){
			return;
		} else {
			mfe.value = lastRes;
		}
	}
}

/**
 * Processes a click event.
 * 
 * @param {HTMLEvent} e - Event to be processed.
 * @param {HTMLElement} mfe - MathfieldElement to be processed.
 * @param {HTMLElement} history - History element to be processed.
 * @param {function} convertLatexToMarkup - Function to convert latex to markup.
 * @param {HTMLElement} inpEl - Element to be processed.
*/
function processClick(e, mfe, history, convertLatexToMarkup, inpEl){
	if (isContained(e.target, "history")){
		let src = getTop(e.target, "history");
		if (!src){
			return;
		}
		let buf = src.dataset.original;
		mfe.value = buf;
	}
	let all = new Set([e.target.children[1]?.classList?.contains("MLK__shift"), e.target.nextSibling?.classList?.contains("MLK__shift"), e.target?.href?.baseVal , e.target?.href?.baseVal])
	let skip=false;
	if ((e.target.children[1]?.classList?.contains("MLK__shift") == false) || (e.target.nextSibling?.classList?.contains("MLK__shift") == false || (e.target?.href?.baseVal != "#svg-commit" && e.target?.href?.baseVal != undefined))){
		if (e.target.classList != undefined){
			if (e.target.classList.contains("svg-glyph")){
				if (e.target.children[0] != undefined){
					if (e.target?.href?.baseVal == "#svg-commit"){
						skip=true;
					}
				}
			}
		}
		if (skip == false){
			if (isContained(e.target, "history")){
				let src = getTop(e.target, "history");
				if (!src){
					return;
				}
				let buf = src.dataset.original;
				mfe.value = buf;
			}
			return;
		}
	}
	if (skip == false){
		if (e.target.classList != undefined){
			if (e.target.classList.contains("svg-glyph")){
				if (e.target.children[0] != undefined){
					if (e.target.children[0]?.href?.baseVal != "#svg-commit"){
						return
					}
				}
			}
		}
		if (all.size == 1){
			if (all.has(undefined)){
				return;
			}
		}
	}
	if (e.target.id.startsWith("ML__") && e.target.tagName.toLowerCase() == "div"){
		if (e.target.children[0] != undefined){
			if (e.target.children[0].tagName.toLowerCase() == "svg"){
				if (e.target.children[0].children[0] != undefined){
					if (e.target.children[0].children[0].tagName.toLowerCase() == "use"){
						if (e.target.children[0].children[0].href.baseVal != "#svg-commit"){
							return;
						}
					}
				}
			}
		}
	}
	addToHistory(mfe, history, convertLatexToMarkup, inpEl);
}

/**
 * Adds the current equation to the history.
 * 
 * @param {HTMLElement} mfe - MathfieldElement to be processed.
 * @param {HTMLElement} history - History element to be processed.
 * @param {function} convertLatexToMarkup - Function to convert latex to markup.
 * @param {HTMLElement} inpEl - Element to be processed.
*/
function addToHistory(mfe, history, convertLatexToMarkup, inpEl){
	if (mfe.value == ""){
		return;
	}
	const ce = new window.ComputeEngine();
	parsed = ce.parse(mfe.value, {canonical: false}).json;
	let res;
	if (parsed[0] != "Error"){
		res = process(parsed);
	} else{
		res = parsed.join(" ");
		res = "Syntax Error!";
	}
	if (errorMsg != "" || res[0] == "Sequence" || res[0] == "Negate"){
		res = errorMsg;
		res = "Syntax Error!";
		errorMsg = "";
	}
	if (history.children[0]?.dataset?.original == mfe.value){
		mfe.value = "";
		return;
	}
	if (res % 1 != 0){
		lastRes = mfe.value
	} else {
		lastRes = res;
	}

	if (res == "Syntax Error!"){
		res = "Syntax\\ Error!";
		lastRes = "";
	}

	history.innerHTML = `<p class="history">${convertLatexToMarkup(mfe.value)} ${convertLatexToMarkup("=")} ${convertLatexToMarkup(res)}</p>` + history.innerHTML;
	history.children[0].dataset.original = mfe.value;
	mfe.value = "";
	for (let i = 0; i < history.children.length; i++){
		if (isCollision(history.children[i], inpEl)){
			history.removeChild(history.children[i]);
			break;
		}
	}
}

/**
 * Handles key presses, dispatches actions according to set rules.
 * 
 * @param {HTMLEvent} e - Event to be processed.
 * @param {HTMLElement} mfe - MathfieldElement to be processed.
 * @param {HTMLElement} history - History element to be processed.
 * @param {function} convertLatexToMarkup - Function to convert latex to markup.
 * @param {HTMLElement} inpEl - Element to be processed.
*/
function handleKeyDown(e, mfe, history, convertLatexToMarkup, inpEl){
	try {
		document.querySelectorAll(".warning")[0].style.background = "#ff0000";
	}
	catch {
	}
	if (e.key == "Enter" || e.code == "NumpadEnter"){
		addToHistory(mfe, history, convertLatexToMarkup, inpEl);
	}
	let allowedKeys = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "!", "+", "-", "*", "/", "^", ".", "ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight", "Backspace", "Delete", "(", ")"];
	if (!allowedKeys.includes(e.key)){
		e.preventDefault();
	}
}

function handleKeyUp(){
	try {
		document.querySelectorAll(".action")[2].style.background = "#1B5165";
	}
	catch {
	}
}

/**
 * Get if two given elements collide/ overlap.
 * 
 * @param {HTMLElement} a - Element 1.
 * @param {HTMLElement} b - Element 2.
 * @returns {bool} Whether or not they collide.
*/
function isCollision(a, b){
	var aRect = a.getBoundingClientRect();
    var bRect = b.getBoundingClientRect();

    return !(
        ((aRect.top + aRect.height) < (bRect.top)) ||
        (aRect.top > (bRect.top + bRect.height)) ||
        ((aRect.left + aRect.width) < bRect.left) ||
        (aRect.left > (bRect.left + bRect.width))
    );
}

/**
 * Repositions the input field to the top of the keyboard, changes the background color of the inpEl and disables its bordders + shadows.
 * 
 * @param {HTMLElement} inpEl - Element to reposition.
*/
function repositionInput(inpEl){
	let low = document.getElementsByClassName("MLK__backdrop")[0].offsetHeight;
	inpEl.style.bottom = `${low}px`;
	inpEl.style.maxWidth = `${document.body.offsetWidth-17}px`;
	document.querySelectorAll(".MLK__backdrop")[0].style.background = "#9FC4CB";
	document.querySelectorAll(".MLK__backdrop")[0].style.boxShadow = "0px 0px 0px 0px";
	document.querySelectorAll(".MLK__backdrop")[0].style.borderTop = "0px";
	
	document.querySelectorAll(".action").forEach(div => div.style.background = "#1B5165");
	document.querySelectorAll(".action").forEach(div => div.style.color = "#ffffff ");
}

/**
 * See if source element in encapsulated in target element.
 * 
 * @param {HTMLElement} src - Starting element.
 * @param {string} targetClass - What class are we looking for.
 * @returns {bool} Whether or not we are encapsulated.
*/
function isContained(src, targetClass){
	while (src != document.body){
		if (src?.classList?.contains(targetClass)){
			return true;
		}
		try{
			src = src.parentNode;
		}catch (err){
			if (err instanceof TypeError){
				break;
			}
		}
	}
	return false;
}

/**
 * Gets the top element matching given parameter.
 * @function
 * @param {HTMLElement} src - Starting element.
 * @param {string} targetClass - What class are we looking for.
 * @returns {bool|HTMLElement} The target element. If not found, `false`.
*/
function getTop(src, targetClass){
	if (src?.classList?.contains(targetClass)){
		return src;
	}
	while (src != document.body){
		if (src.parentNode?.classList?.contains(targetClass)){
			return src.parentNode;
		}
		src = src.parentNode;
	}
	return false;
}

/**
 * Processes AST of the equation
 * @function
 * @param {Object} tree - The systax tree, formatted as specified in math-json.
 * @returns {string} The result of the operations.
 */
function process(tree){
	if (Array.isArray(tree)){
		if (tree[0] == "Error"){
			errorMsg = tree.join(" ");
			return tree;
		}
		let buf;
		if (tree.length == 1){
			errorMsg = "Syntax Error!";
			return "Error";
		}
		switch (tree[0]) {
			case "Add":
				buf = 0;
				for (let i = 1; i < tree.length; i++){
					buf = addNumbers(buf, parseFloat(process(tree[i])));
				}
				return buf;
			case "Subtract":
				buf = process(tree[1]);
				for (let i = 2; i < tree.length; i++){
					buf = subtractNumbers(buf, parseFloat(process(tree[i])));
				}
				return buf;
			case "Multiply":
				buf = process(tree[1]);
				for (let i = 2; i < tree.length; i++){
					buf = multiplyNumbers(buf, parseFloat(process(tree[i])));
				}
				return buf;
			case "Divide":
				if (tree[1] == "Nothing" || tree[2] == "Nothing"){
					errorMsg = "Syntax Error!";
					return "Error";
				}
				return divideNumbers(parseFloat(process(tree[1])), parseFloat(process(tree[2])));
			case "Factorial":
				return factorialNumber(parseFloat(process(tree[1])));
			case "Factorial2":
				return factorialNumber(factorialNumber(parseFloat(process(tree[1]))));
			case "Root":
				if (tree[1] == "Nothing"){
					errorMsg = "Syntax Error!";
					return "Error";
				}
				if (tree[2] == "Nothing"){
					tree[2] = 2;
				}
				return rootNumber(parseFloat(process(tree[1])), parseFloat(process(tree[2])));
			case "Power":
				if (tree[1] == "Nothing"){
					errorMsg = "Syntax Error!";
					return "Error";
				}
				if (tree[2] == "Nothing"){
					tree[2] = 2;
				}
				if (tree[1] == "e"){
					return expNumber(parseFloat(process(tree[2])));
				}
				return powerNumber(parseFloat(process(tree[1])), parseFloat(process(tree[2])));
			case "Floor":
				if (tree[1] == "Nothing" || ["Error", "Nothing"].includes(process(tree[1]))){
					errorMsg = "Syntax Error!";
					return "Error";
				}
				return roundNumber(process(tree[1]));
			case "Abs":
				if (tree[1] == "Nothing" || ["Error", "Nothing"].includes(process(tree[1]))){
					errorMsg = "Syntax Error!";
					return "Error";
				}
				return absNumber(process(tree[1]));
			case "Ln":
				if (tree[1] == "Nothing"){
					errorMsg = "Syntax Error!";
					return "Error";
				}
				return lnNumber(process(tree[1]));
			case "Log":
				if (tree[1] == "Nothing"){
					errorMsg = "Syntax Error!";
					return "Error";
				}
				if (tree[2] == "Nothing"){
					tree[2] = 10;
				}
				return logNumber(process(tree[1]), process(tree[2]));
			case "Delimiter":
				if (tree.length > 1){
					return process(tree[1]);
				}
				errorMsg = "Syntax Error!";
				return "Error"
			case "InvisibleOperator":
				if (tree[1][0] == "Decrement"){
					return process(["Subtract", parseFloat(process(tree[2])), multiplyNumbers(parseFloat(process(tree[1][1])), -1)] );
				}
				tree[0] = "Multiply";
				return process(tree);
			default:
				return tree;
		}
	}else{
		if (Number(tree) == tree){
			return tree;
		}
		return tree.num;
	}
}
