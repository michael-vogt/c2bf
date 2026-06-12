#include <fstream>

#include "lexer.hpp"
#include "parser.hpp"
#include "codegen.hpp"
#include <iostream>
#include <io.h>

using namespace std;

string readFile(const string& filename) {
    ifstream file(filename);
    return string(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
}

int main(int argc, char ** argv) {
    string code;

    if (argc > 1) {
        code = readFile(argv[1]);
    } else {
        if (_isatty(_fileno(stdin))) {
            cout << "Programmcode: (Eingabe abschliessen mit ENTER, CTRL+Z, ENTER (Windows) / CTRL+D (Linux/macOS))\n";
        }

        stringstream buffer;
        buffer << cin.rdbuf();
        code = buffer.str();
    }

    if (argc > 1) {
        code = readFile(argv[1]);
    }

    cout << "C program:\n" << code << endl;

    Lexer lexer(code);
    Parser parser(lexer.tokenize());
    const unique_ptr<Program> program = std::move(parser.parse());
    //auto program = parser.parse().get();
    string bfCode = BrainfuckCodegen::generate(*program);

    cout << "BF program:\n" << bfCode << endl;
}
