### Lets see how well this works

##### CLI Options

- -debug, -nodebug

- -assemble [PathIn] [PathOut]
- -disassemble [PathIn] [PathOut OR "console"]
- -exec [PathIn]
- -compile [PathIn] [PathOut]

##### Things I'm Changing
- Safer code... safe pointers?
- Tokenizer does matching with a word tree rather than certain very specific tokens
- Formalize the grammar of the language somewhat
- No more `Constructor(varIn) : var(varIn)` just `var` instead of `varIn`

##### Formatting?
- enums with strings should have `Enum`, `enumCount`, and `enumStrings` defined