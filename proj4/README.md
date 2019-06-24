# Simple Instruction Level Debugger

## Environment Requirements
* readline library
* capstone library

## Features
* all requirements
* input history
    * `tab` can auto complete path
    * `↑` can use history command
* modulize, easy to add new command
    * use `BUILDIN_REGISTER(name, shortname) { ... }` to define new command
    * no need to modify other code
