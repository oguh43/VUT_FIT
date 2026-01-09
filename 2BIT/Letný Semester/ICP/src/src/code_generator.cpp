/**
* @author Filip Jenis (xjenisf00)
*/

#include "../headers/code_generator.h"

std::string CodeGenarator::generatedFileHeader = R"===(
#ifndef GENERATED_AUTOMATON_H
#define GENERATED_AUTOMATON_H

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <regex>
#include <variant>
#include <cstring>
#include <iostream>
#include <optional>
#include <cstdint>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR   -1
    typedef int SOCKET;
#endif

class IOComm{
private:
    SOCKET sock;
    struct sockaddr_in localAddress;
    struct sockaddr_in remoteAddress;

public:
    IOComm(int port);
    ~IOComm();

    bool editorConnected;

    bool send(const std::string& message);
    std::optional<std::string> receive(int timeout);
    bool setNonBlocking();
};

enum class TokenType {
    VARIABLE,        /**< Variable identifier */
    NUMBER,          /**< Numeric literal */
    OPERATOR,        /**< Mathematical operator */
    FUNCTION,        /**< Function identifier */
    SPECIAL_KEYWORD, /**< Special language keywords */
    EQUALS,          /**< Assignment operator */
    GET,             /**< 'get' keyword for input retrieval */
    OUTPUT,          /**< 'output' keyword for output generation */
    TO,              /**< 'to' keyword for output destination */
    IF,              /**< 'if' keyword for condition */
    IS,              /**< 'is' keyword for condition */
    DEFINED,         /**< 'defined' keyword for checking defined inputs */
    END              /**< End of expression marker */
};

struct Token {
    TokenType type;      /**< The type of this token */
    std::string value;   /**< The string value of this token */

    Token(TokenType type, const std::string& value) : type(type), value(value) {}
};

typedef std::variant<int, float, std::string> variableType;

class Transition;

class State{
public:
    std::string name;
    std::string outputExpression;
    std::vector<Transition*> outgoing;
    std::chrono::steady_clock::time_point start;

    State(std::string name, std::string outputExpression): name(name), outputExpression(outputExpression){}
};

class Timer{
public:
    std::chrono::steady_clock::time_point start;
    std::chrono::milliseconds duration;
    bool active;

    Timer(std::chrono::milliseconds duration): duration(duration), active(false){}
};

class Transition{
public:
    State* from;
    State* to;
    std::string guardExpression;
    std::string timeout;
    Timer* timeoutTimer;

    Transition(State* from, State* to, std::string guardExpression, std::string timeout, Timer *timeoutTimer): from(from), to(to), guardExpression(guardExpression), timeout(timeout), timeoutTimer(timeoutTimer){}

    bool evaluateTransitionCondition(std::unordered_map<std::string, std::string> &inputs, std::unordered_map<std::string, variableType> &variables);
};

class AutomatonEngine{
private:
    std::unordered_map<std::string, State*> states;
    std::unordered_map<std::string, std::string> inputs;
    std::unordered_map<std::string, std::string> outputs;
    std::unordered_map<std::string, variableType> variables;
    State* currentState;
    IOComm comm;

    void checkTimers();
    void resetTimers();
    void executeOutputAction();
    variableType evaluateExpression(std::string expr);
    void tick();
    void sendStateChange(std::string newState);
    void sendVariableChange(std::string variableName, std::string value);
    void sendOutputChange(std::string outputName, std::string value);
    void sendInitialMessage();

public:
    AutomatonEngine();
    ~AutomatonEngine();

    void createState(std::string name, std::string outputExpression);
    void createTransition(std::string from, std::string to, std::string expr, std::string timeout);
    void createInputPointer(std::string name);
    void createOutputPointer(std::string name);
    void createVariable(std::string name, variableType data);
    void run();
};

#endif //GENERATED_AUTOMATON_H
)===";

std::string CodeGenarator::helperFunctions = R"===(

std::string getDigits(const std::string& expr, size_t& pos) {
    std::string number;
    bool hasDecimal = false;

    while (pos < expr.length() && (std::isdigit(expr[pos]) || (expr[pos] == '.' && !hasDecimal))) {
        if (expr[pos] == '.') {
            hasDecimal = true;
        }
        number += expr[pos++];
    }

    return number;
}

std::string getIdentifier(const std::string& expr, size_t& pos) {
    std::string identifier;

    while (pos < expr.length() && (std::isalnum(expr[pos]) || expr[pos] == '_')) {
        identifier += expr[pos++];
    }

    return identifier;
}

std::vector<Token> tokenize(const std::string& expression) {
    std::vector<Token> tokens;
    std::string token;

    for (size_t i = 0; i < expression.length(); i++) {
        char c = expression[i];

        // Skip whitespace
        if (std::isspace(c)) {
            continue;
        }

        // Handle numbers
        if (std::isdigit(c) || c == '.') {
            std::string number = getDigits(expression, i);
            i--;
            tokens.emplace_back(TokenType::NUMBER, number);
        }
            // Handle identifiers (variables, keywords)
        else if (std::isalpha(c) || c == '_') {
            std::string identifier = getIdentifier(expression, i);
            i--;

            // Check for special keywords
            if (identifier == "get") {
                tokens.emplace_back(TokenType::GET, identifier);
            } else if (identifier == "output") {
                tokens.emplace_back(TokenType::OUTPUT, identifier);
            } else if (identifier == "to") {
                tokens.emplace_back(TokenType::TO, identifier);
            } else if (identifier == "if") {
                tokens.emplace_back(TokenType::IF, identifier);
            } else if (identifier == "is") {
                tokens.emplace_back(TokenType::IS, identifier);
            } else if (identifier == "defined") {
                tokens.emplace_back(TokenType::DEFINED, identifier);
            } else if (identifier == "elapsed") {
                tokens.emplace_back(TokenType::SPECIAL_KEYWORD, identifier);
            } else {
                tokens.emplace_back(TokenType::VARIABLE, identifier);
            }
        }
            // Handle operators
        else if (c == '+' ||c == '-' || c == '*' || c == '/' || c == '=') {
            if (c == '=') {
                tokens.emplace_back(TokenType::EQUALS, "=");
            } else {
                tokens.emplace_back(TokenType::OPERATOR, std::string(1, c));
            }
        }
    }

    tokens.emplace_back(TokenType::END, "");
    return tokens;
}

std::string variantToString(const variableType& value) {
    return std::visit([](const auto& val) -> std::string {
        if constexpr (std::is_same_v<std::decay_t<decltype(val)>, int>) {
        return std::to_string(val);
    } else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, float>) {
        return std::to_string(val);
    } else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::string>) {
        return val;
    }
    }, value);
}
)===";

std::string CodeGenarator::transitionFunctions = R"===(
bool Transition::evaluateTransitionCondition(std::unordered_map <std::string, std::string> &inputs, std::unordered_map<std::string, variableType> &variables) {
    size_t startpos = 0, endpos;
    std::string expr;
    bool result = false;
    std::smatch match;
    variableType leftValue, rightValue;
    if (guardExpression == ""){
		return true;
	}
    while ((endpos = guardExpression.find(";", startpos)) != std::string::npos) {
        expr = guardExpression.substr(startpos, endpos - startpos);
        if (std::regex_match(expr, match, std::regex("^(.+) ?(==|!=) ?(.+)$"))){
            if (variables.find(match[1]) != variables.end()){
                leftValue = variables[match[1]];
            }else{
                return false;
            }
            if (variables.find(match[3]) != variables.end()){
                rightValue = variables[match[3]];
            }else{
                if (std::holds_alternative<int>(leftValue)){
                    rightValue = std::stoi(match[3]);
                }else if (std::holds_alternative<float>(leftValue)){
                    rightValue = std::stof(match[3]);
                }else{
                    rightValue = match[3];
                }
            }
            if (match[2] == "=="){
                if (leftValue == rightValue){
                    result = result || startpos == 0;
                }else{
                    return false;
                }
            }else{
                if (leftValue != rightValue){
                    result = result || startpos == 0;
                }else{
                    return false;
                }
            }
        }else if (std::regex_match(expr, match, std::regex("^got (.+) from (.+)$"))) {
            if (inputs.find(match[2]) != inputs.end()) {
                if (match[1] == inputs[match[2]]) {
                    result = result || startpos == 0;
                }else{
                    return false;
                }
            } else {
                return false;
            }
        }else{
            return false;
        }
        startpos = endpos + 1;
    }
    return result;
}
)===";

std::string CodeGenarator::automatonEngineFunctions = R"===(
void AutomatonEngine::createState(std::string name, std::string outputExpression) {
    State* state = new State(name, outputExpression);
    states[name] = state;
    if (currentState == nullptr){
        currentState = state;
    }
}

void AutomatonEngine::createTransition(std::string from, std::string to, std::string expr, std::string timeout) {
    Timer *timer = new Timer(std::chrono::milliseconds(std::stoll(timeout)));
    states.at(from)->outgoing.push_back(new Transition(states[from], states[to], expr, timeout, timer));
}

void AutomatonEngine::resetTimers() {
    for (auto t: currentState->outgoing) {
        t->timeoutTimer->active = false;
    }
}

void AutomatonEngine::checkTimers() {
    auto now = std::chrono::steady_clock::now();
    for (auto t: currentState->outgoing) {
        if (t->timeoutTimer->active){
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - t->timeoutTimer->start);
            if (elapsed >= t->timeoutTimer->duration){
                t->timeoutTimer->active = false;
                if (t->to != t->from){
                    resetTimers();
                    std::cout << "Action: State changed from " << t->from->name << " to " << t->to->name << std::endl;
                    sendStateChange(t->to->name);
                    currentState->start = std::chrono::steady_clock::now();
                }
                currentState = t->to;
                executeOutputAction();
            }
        }
    }
}

variableType AutomatonEngine::evaluateExpression(std::string expr) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentState->start - now);

    std::vector<Token> tokens = tokenize(expr);
    int pos = 0;

    bool firstVal = true;
    variableType result;
    variableType currentValue;
    std::string currentOp = "+";

    while (pos < tokens.size() && tokens[pos].type != TokenType::END) {
        Token token = tokens[pos++];

        switch (token.type) {
            case TokenType::VARIABLE: {
                // Replace variable with its value
                auto it = variables.find(token.value);
                if (it != variables.end()) {
                    currentValue = it->second;
                } else {
                    if (tokens.size() == 2){
						currentValue = token.value;
					}else{
						throw std::runtime_error("Unknown variable: " + token.value);
					}
                }
                break;
            }
            case TokenType::NUMBER: {
                // Parse number
                try {
                    if (token.value.find('.') != std::string::npos) {
                        currentValue = std::stof(token.value);
                    } else {
                        currentValue = std::stoi(token.value);
                    }
                } catch (const std::exception& e) {
                    throw std::runtime_error("Invalid number: " + token.value);
                }
                break;
            }
            case TokenType::SPECIAL_KEYWORD: {
                // Handle kw "elapsed"
                if (token.value == "elapsed") {
                    currentValue = static_cast<int>(elapsed.count());
                } else {
                    throw std::runtime_error("Unknown keyword: " + token.value);
                }
                break;
            }
            case TokenType::OPERATOR: {
                // Store operator for next value
                currentOp = token.value;
                continue; // Skip to next token
            }
            default:
                throw std::runtime_error("Unexpected token: " + token.value);
        }

        if (firstVal){
            result = currentValue;
            firstVal = false;
        }else{
            if (std::holds_alternative<int>(result) && std::holds_alternative<int>(currentValue)){
                int val1 = std::get<int>(result);
                int val2 = std::get<int>(currentValue);

                if (currentOp == "+") result = val1 + val2;
                else if (currentOp == "-") result = val1 - val2;
                else if (currentOp == "*") result = val1 * val2;
                else if (currentOp == "/") {
                    if (val2 == 0) throw std::runtime_error("Division by zero");
                    result = val1 / val2;
                }
                else throw std::runtime_error("Unsupported operator: " + currentOp);
            }else if ((std::holds_alternative<float>(result) && (std::holds_alternative<int>(currentValue) || std::holds_alternative<float>(currentValue))) || (std::holds_alternative<int>(result) && std::holds_alternative<float>(currentValue))){
                float val1 = (std::holds_alternative<float>(result)) ? std::get<float>(result) : static_cast<float>(std::get<int>(result));
                float val2 = (std::holds_alternative<float>(currentValue)) ? std::get<float>(currentValue) : static_cast<float>(std::get<int>(currentValue));

                if (currentOp == "+") result = val1 + val2;
                else if (currentOp == "-") result = val1 - val2;
                else if (currentOp == "*") result = val1 * val2;
                else if (currentOp == "/") {
                    if (val2 == 0) throw std::runtime_error("Division by zero");
                    result = val1 / val2;
                }
                else throw std::runtime_error("Unsupported operator: " + currentOp);
            }else if (std::holds_alternative<std::string>(result) && currentOp == "+"){
                result = std::get<std::string>(result) + variantToString(currentValue);
            }else{
                throw std::runtime_error("Incompatible types for operation " + currentOp);
            }
        }
    }

    return result;
}

void AutomatonEngine::executeOutputAction() {
    size_t startpos = 0, endpos;
    std::string expr;
    std::smatch match, match2;
    variableType exprResult;
    while ((endpos = currentState->outputExpression.find(";", startpos)) != std::string::npos){
        expr = currentState->outputExpression.substr(startpos, endpos - startpos);

        if(std::regex_match(expr, match, std::regex("^if (.+) is defined ?$"))){
            if (!(inputs.find(match[1]) != inputs.end() && !inputs[match[1]].empty())){
                return;
            }
        }else if (std::regex_match(expr, match, std::regex("^([^= ]+) ?= ?(.+)$"))){
            if (variables.find(match[1]) != variables.end()){
                std::string varExpression = match.str(2);
                if (std::regex_match(varExpression, match2, std::regex("^ ?get +([^ ]+)$"))){
                    if (inputs.find(match2[1]) != inputs.end()){
                        variables[match[1]] = inputs[match2[1]];
                        sendVariableChange(match[1], inputs[match2[1]]);
                        std::cout << "Action: Variable " << match[1] << " set to " << inputs[match2[1]] << std::endl;
                    }
                }else {
                    exprResult = evaluateExpression(match.str(2));
                    variables[match[1]] = exprResult;
                    sendVariableChange(match[1], variantToString(exprResult));
                    std::cout << "Action: Variable " << match[1] << " set to " << variantToString(exprResult) << std::endl;
                }
            }
        }else if (std::regex_match(expr, match, std::regex("^output ([^ ]+) to (.+)$"))){
            if (outputs.find(match[2]) != outputs.end()){
                exprResult = evaluateExpression(match.str(1));
                outputs[match[2]] = variantToString(exprResult);
                sendOutputChange(match[2], variantToString(exprResult));
                std::cout << "Action: Output " << match[2] << " set to " << variantToString(exprResult) << std::endl;
            }
        }

        startpos = endpos + 1;
    }
}

void AutomatonEngine::tick() {
    checkTimers();

    for (Transition* t: currentState->outgoing) {
        if (t->evaluateTransitionCondition(inputs, variables)){
            if (std::stoll(t->timeout) > 0){
                if (!t->timeoutTimer->active){
                    t->timeoutTimer->start = std::chrono::steady_clock::now();
                    t->timeoutTimer->active = true;
                }
                continue;
            }
            if (t->to != t->from){
                resetTimers();
                std::cout << "Action: State changed from " << t->from->name << " to " << t->to->name << std::endl;
                sendStateChange(t->to->name);
                currentState->start = std::chrono::steady_clock::now();
            }
            currentState = t->to;
            executeOutputAction();
        }
    }
}

void AutomatonEngine::sendStateChange(std::string newState) {
    std::string msg;
    msg = {static_cast<char>(0x03)};
    msg += "\r\n";
    msg += newState + "\r\n";
    if (!(comm.send(msg))){
        std::cerr << "Failed to send" << std::endl;
        std::exit(1);
    }
}

void AutomatonEngine::sendVariableChange(std::string variableName, std::string value) {
    std::string msg;
    msg = {static_cast<char>(0x04)};
    msg += "\r\n";
    msg += variableName + "\r\n";
    msg += value + "\r\n";
    if (!(comm.send(msg))){
        std::cerr << "Failed to send" << std::endl;
        std::exit(1);
    }
}

void AutomatonEngine::sendOutputChange(std::string outputName, std::string value) {
    std::string msg;
    msg = {static_cast<char>(0x05)};
    msg += "\r\n";
    msg += outputName + "\r\n";
    msg += value + "\r\n";
    if (!(comm.send(msg))){
        std::cerr << "Failed to send" << std::endl;
        std::exit(1);
    }
}

void AutomatonEngine::sendInitialMessage() {
    std::string msg;
    std::string inputsMsg;
    std::string outputsMsg;
    std::string variablesMsg;
    msg += static_cast<char>(0x06);
    msg += "\r\n";
    msg += FSM_DEFINITION_PATH;
    msg += "\r\n";
    msg += currentState->name + "\r\n";
    for (auto i: inputs){
        inputsMsg += i.first + "\r\n" + i.second + "\r\n";
    }
    msg += std::to_string(uint16_t(inputsMsg.length())) + "\r\n";
    msg += inputsMsg;
    for (auto o: outputs){
        outputsMsg += o.first + "\r\n" + o.second + "\r\n";
    }
    msg += std::to_string(uint16_t(outputsMsg.length())) + "\r\n";
    msg += outputsMsg;
    for (auto v: variables){
        variablesMsg += v.first + "\r\n" + variantToString(v.second) + "\r\n";
    }
    msg += std::to_string(uint16_t(variablesMsg.length())) + "\r\n";
    msg += variablesMsg + "\r\n";
    if (!(comm.send(msg))){
        std::cerr << "Failed to send" << std::endl;
        std::exit(1);
    }
}

void AutomatonEngine::run() {
    std::cout << "Executing FSM " << FSM_NAME << "..." << std::endl;
    std::cout << "Entry point: Entered state " << currentState->name << std::endl;
    executeOutputAction();
    bool cond = true;
    while (cond) {
        tick();
        auto msg = comm.receive(100);
        if (msg.has_value()){
            switch (int((*msg)[0])){
                case 0x00:
                    sendInitialMessage();
                    std::cout << "Connection: Remote editor has established connection with this FSM" << std::endl;
                    break;
                case 0x01:
                    {
                        size_t inputPtrNameEdge = (*msg).find("\r\n", 2);
                        if (inputs.find((*msg).substr(3, inputPtrNameEdge - 3)) != inputs.end()){
                            size_t inputValueEdge = (*msg).find("\r\n", inputPtrNameEdge + 2);
                            inputs[(*msg).substr(3, inputPtrNameEdge -3)] = (*msg).substr(inputPtrNameEdge + 2, inputValueEdge - inputPtrNameEdge - 2);
                            std::cout << "Input: got " << (*msg).substr(inputPtrNameEdge + 2, inputValueEdge - inputPtrNameEdge - 2) << " from " << (*msg).substr(3, inputPtrNameEdge - 3) << std::endl;
                        }
                    }
                    break;
                case 0x02:
                    cond = false;
                    break;
                case 0x07:
                    {
                        if (comm.editorConnected){
                            comm.editorConnected = false;
                            std::cout << "Connection: Remote editor has disconnected from this FSM" << std::endl;
                        }
                    }
                    break;
            }
        }
    }
}

void AutomatonEngine::createInputPointer(std::string name) {
    inputs[name] = "";
}

void AutomatonEngine::createOutputPointer(std::string name) {
    outputs[name] = "";
}

void AutomatonEngine::createVariable(std::string name, variableType data) {
    variables[name] = data;
}

AutomatonEngine::AutomatonEngine(): currentState(nullptr), comm(56789) {
    comm.setNonBlocking();
}

AutomatonEngine::~AutomatonEngine() {
     for (auto state: states) {
        for (Transition* t: state.second->outgoing) {
            delete t->timeoutTimer;
            delete t;
        }
        delete state.second;
    }
}
)===";

std::string CodeGenarator::networkFunctions = R"===(
IOComm::IOComm(int port) {
#ifdef _WIN32
    WSDATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET){
        std::cerr << "Failed to create socket" << std::endl;
        std::exit(1);
    }

    std::memset(&localAddress, 0, sizeof(localAddress));
    localAddress.sin_family = AF_INET;
    localAddress.sin_addr.s_addr = INADDR_ANY;
    localAddress.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*)&localAddress, sizeof(localAddress)) == SOCKET_ERROR){
        std::cerr << "Bind failed" << std::endl;
        std::exit(1);
    }
    editorConnected = false;
}

IOComm::~IOComm() {
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
}

bool IOComm::setNonBlocking() {
#ifdef _WIN32
    u_long nonBlocking = 1;
    return ioctlsocket(sock, FIONBIO, &nonBlocking) == 0;
#else
    int flags = fcntl(sock, F_GETFL, 0);
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}

std::optional<std::string> IOComm::receive(int timeout) {
    fd_set readfs;
    FD_ZERO(&readfs);
    FD_SET(sock, &readfs);

    timeval tv{};
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    int ready = select(sock + 1, &readfs, nullptr, nullptr, &tv);

    if (ready > 0 && FD_ISSET(sock, &readfs)) {
        char buffer[2048];
        struct sockaddr_in fromAddr{};
        socklen_t fromlen = sizeof(fromAddr);

        int received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *) &fromAddr, &fromlen);

        if (received > 0) {
            if (!editorConnected && buffer[0] != 0x07){
                remoteAddress = fromAddr;
                editorConnected = true;
            }
            buffer[received] = '\0';
            return std::string(buffer);
        } else if (received < 0) {
            std::cerr << "Failed to receive" << std::endl;
            std::exit(1);
        }
    }
    return std::nullopt;
}

bool IOComm::send(const std::string &message) {
    if (!editorConnected) return true;
    int sent = sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr*)&remoteAddress, sizeof(remoteAddress));
    return sent != SOCKET_ERROR;
}
)===";


/**
 * @brief Generates C++ code from the internal representation of a Moore Machine
 *
 * @param machine Internal representation of a Moore Machine
 * @param filename Path for the generated code
 * @return True if generation is successful, false otherwise
 */
bool CodeGenarator::generateCode(const MooreMachine &machine, const std::string &filename) {
    // Save text form of generated automaton
    MachineFileHandler::saveToFile(machine, filename + ".fsm");
    // Open file for writing
    std::string cppFilename = filename + ".cpp";
    std::ofstream file(cppFilename);
    if (!file.is_open()) {
        return false;
    }

    // Write static blocks of code
    file << generatedFileHeader << std::endl;
    file << "#define FSM_NAME \"" << machine.getName() << "\"" << std::endl;
    file << "#define FSM_DEFINITION_PATH \"" << filename << ".fsm" << "\"" << std::endl;
    file << networkFunctions << std::endl;
    file << helperFunctions << std::endl;
    file << transitionFunctions << std::endl;
    file << automatonEngineFunctions << std::endl;

    // Write main function head
    file << "int main(){" << std::endl;
    file << "\t" << "AutomatonEngine ae;" << std::endl;

    // Generate representation of all inputs
    auto inputPointers = machine.getInputPointers();
    for (const auto& pointer : inputPointers) {
        file << "\t" << "ae.createInputPointer(\"" << pointer << "\");" << std::endl;
    }

    // Generate representation of all outputs
    auto outputPointers = machine.getOutputPointers();
    for (const auto& pointer : outputPointers) {
        file << "\t" << "ae.createOutputPointer(\"" << pointer << "\");" << std::endl;
    }

    // Generate representation of all variables
    auto variables = machine.getVariables();
    for (const auto& varPair : variables) {
        const auto &var = varPair.second;
        file << "\t" << "ae.createVariable(\"" << var.getName() << "\", " << var.getValueString() << ");" << std::endl;
    }

    // Generate all states
    State* initialState = const_cast<MooreMachine&>(machine).getInitialState();

    if (initialState) {
        file << "\t" << "ae.createState(\"" << initialState->getName() << "\", \"";

        auto outputs = initialState->getOutputs();
        if (!outputs.empty()) {
            for (const auto& output : outputs) {
                // Check if this is a variable expression or regular output
                if (output.value.find("=") != std::string::npos ||
                    output.value.find("+") != std::string::npos ||
                    output.value.find("-") != std::string::npos ||
                    output.value.find("*") != std::string::npos ||
                    output.value.find("/") != std::string::npos) {
                    file << output.value;
                } else {
                    file << "output " << output.value << " to " << output.target;
                }
                // Add condition if present
                if (output.hasCondition) {
                    file << " if " << output.inputPtr << " is defined";
                }

                file << ";";
            }
        }
        file << "\");" << std::endl;
    }

    for (State* state : const_cast<MooreMachine&>(machine).getAllStates()) {
        // Skip the initial state (already written)
        if (initialState && state->getId() == initialState->getId()) {
            continue;
        }
        file << "\t" << "ae.createState(\"" << state->getName() << "\", \"";
        auto outputs = state->getOutputs();
        if (!outputs.empty()) {
            for (const auto& output : outputs) {
                // Check if this is a variable expression or regular output
                if (output.value.find("=") != std::string::npos ||
                    output.value.find("+") != std::string::npos ||
                    output.value.find("-") != std::string::npos ||
                    output.value.find("*") != std::string::npos ||
                    output.value.find("/") != std::string::npos) {
                    file << output.value;
                } else {
                    file << "output " << output.value << " to " << output.target;
                }
                // Add condition if present
                if (output.hasCondition) {
                    file << " if " << output.inputPtr << " is defined";
                }

                file << ";";
            }
        }
        file << "\");" << std::endl;
    }

    // Generate all transitions
    for (State* sourceState : const_cast<MooreMachine&>(machine).getAllStates()) {
        auto transitions = const_cast<MooreMachine&>(machine).getTransitionsFromState(sourceState->getId());

        // Group transitions by target state
        std::unordered_map<std::string, std::vector<Transition*>> transitionsByTarget;
        for (Transition* transition : transitions) {
            transitionsByTarget[transition->getTargetId()].push_back(transition);
        }

        // Write each target group
        for (const auto& targetGroup : transitionsByTarget) {
            State* targetState = const_cast<MooreMachine&>(machine).getState(targetGroup.first);
            file << "\t" << "ae.createTransition(\"" << sourceState->getName() << "\", \"" << targetState->getName() << "\", \"";
            for (Transition* transition : targetGroup.second) {
                // Write all input conditions
                for (const auto& inputCondition: transition->getInputConditions()) {
                    if (inputCondition.isBooleanExpr) {
                        file << inputCondition.leftOperand << " " << inputCondition.operation << " " << inputCondition.rightOperand;
                    }else{
                        file << "got " << inputCondition.value << " from " << inputCondition.source;
                    }
                    file << ";";
                }
                file << "\", \"";
                if (transition->getTimeout() > 0) {
                    file << transition->getTimeout() << "\");" << std::endl;
                }else{
                    file << "0\");" << std::endl;
                }
            }
        }
    }

    file << "\t" << "ae.run();" << std::endl;
    file << "\t" <<"return 0;" << std::endl;
    file << "}" << std::endl;

    if (!file.good()){
        return false;
    }
    file.close();
    std::string outputFilename;

    // Set executable name according to the target platform
#ifdef _WIN32
    outputFilename = filename + ".exe";
#else
    outputFilename = filename;
#endif

    compileGeneratedCode(QString::fromStdString(cppFilename), QString::fromStdString(outputFilename));
    return true;
}


/**
 * @brief Compiles the generated C++ code
 *
 * @param sourcePath Path of the generated C++ file
 * @param outputPath Path of the compiled executable
 */
void CodeGenarator::compileGeneratedCode(const QString& sourcePath, const QString& outputPath) {
    QProcess compiler;
    // Select compiler according to the platform
#ifdef _WIN32
    QString compilerPath = "g++.exe";
#else
    QString compilerPath = "g++";
#endif

    QStringList args;
    args << sourcePath << "-std=c++17" << "-o" << outputPath;

    compiler.start(compilerPath, args);
    compiler.waitForFinished();

    QByteArray out = compiler.readAllStandardOutput();
    QByteArray err = compiler.readAllStandardError();
    int exitCode = compiler.exitCode();

    if (exitCode == 0) {
        qDebug() << "Compilation successful.";
    } else {
        qWarning() << "Compilation failed:" << err;
    }
}
