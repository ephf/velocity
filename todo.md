# Velocity TODO List

> latest revision: v0.1.0 (unstable)

### Bugs & Source Code

- [ ] Move generics to section at the top of `struct Node` (fixes an issue with generics inputted as type arguments, making it hard for the compiler to check underlying types and correctly parse structure field access)
- [ ] Create a `call_node` function that can scan for the generic section and run macros, and call the node's `prototype` function
- [ ] Make the `resolve_identifier` function also find either types or variables in its found scopes with an argument or some offset
- [ ] *Add functionality for type arguments in `resolve_identifier` aka parse type arguments*
    - This may include adding specific scopes for inserted type arguments for structures and namespaces
- [ ] Create `language.md` guide
- [ ] Add `as_` prefix before unions types in `struct Node` to enhance readability
- [ ] Separate underlying code from places like [postfix.c](/src/parse/postfix.c)
    - [ ] `.` operation aka field access
- [ ] Create macro system based off generic system after generics are moved from being separate nodes
- [ ] Create a vector clone function in [lib.c](/src/lib.c) and replace multiple instances of vector cloning
- [ ] Move generics vector cloning to the end of generics parsing to keep the original vector **or** include a bit or attribute if a generic vector was just instantiated (removes the extra clone at the beginning or end of parsing)
- [ ] Separation of `const` variables for constants and expressions (at the moment, `const` is more of a macro)
- [ ] Combination of [generics.c](/src/parse/node/generics.c) and [symbol/generics.c](/src/parse/node/symbol/generics.c)
- [ ] Fix inserted tab issue with namespaces (able to be seen when compiling the std library)
- [ ] Add fields to the internal string type **or** select internal types (like `str`) from the standard library
- [ ] Cast numeric and pointer types to specific types when operations are used on them
- [x] Rewrite and rename [tok.c](/src/tok.c)
    - [x] Create the `Token` structure instead of the basic `str`
        - [x] Remove `.id` from `str`
    - [x] Complete rewrite and renaming of functions in this file
    - [x] More abilities to provide customized error messages from the tokenizer
    - [x] Differentiate `expect` and `try` functions (right now just `gimme`)
- [ ] Make `type_match` able to assign types to autos both ways
- [ ] Rename `TokenType` enums to `tType`
- [ ] Add more specific error functions

### Language Features

- [ ] control flow
    - [ ] `if` `else` statements
    - [ ] `while` loops
    - [ ] `for` loops
- [ ] `clone` type keyword and checking un-cloned structures
    - This may also include a `clone` keyword for initializing `auto` or un-typed variables
- [ ] `main` function defaulting to `int` instead of `void`
- [ ] Comments using `//` and `/*` `*/` syntax
- [ ] Array types using surrounding syntax (eg. `[Type]`)
- [ ] Ability using a symbol or keyword to extract the `char*` from a string literal (currently gets processed into a `struct str`)
- [ ] `import` statements

### Compiler Features

- [ ] `vlc` input file arguments
- [x] better error messages
    - [x] rewrite error message `Node`
    - [x] collect line info and character position in tokens
        - [x] *create `Token` type*
    - [x] More specialized error messages