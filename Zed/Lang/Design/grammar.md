### Tokenization

```
Symbol Chars: ~ ` ! @ # $ %  ^ & * _ - + = | \ : ; , . ? / ( ) [ ] { } < >
 Symbol Sets: \(^-^)/ === += -= *= /= %= == <= >= ++ -- // /* */
    Numerals: 0-9
    Id Chars: Chars: a-z A-Z 0-9 _
 Other Chars: " '
   Separator: Anything else (including whitespace)
```

Parsing is done left-to-right with the following rules:
 - If `//` is encountered, ignore it and all characters up to the next line break
 - If `/*` is encountered, ignore it and all characters up to (and including) the next occurence of `*/`
 - If `"` is encountered, a new empty string is formed:
   - If `"` is encountered, a null terminator is added and the string is complete
   - If `\` is encountered, match the character immediately after it:
     - `n` -> line break
     - `t` -> tab character
     - `0` -> null terminator (ASCII code 0)
     - `c` -> ANSI escape prefix
     - Anything other char -> just that char
   - Any other char is appended to the string
 - If `'` is encountered, a the next char forms a char, unless it is a `\` and then the escape sequence
   is matched as above for strings. The char after that must be a `'` or an error is thrown
 - If a Numeral is encountered, chars are consumed up to (but excluding) the next non-Numeral
   - If the next char is a `.`, then Numerals are again consumed up to (but excluding) the next
     non-Numeral. The whole set (Numerals, `.`, more Numerals) forms a float
   - Otherwise, the numerals form an int
   - \* there is technically `0b` and `0x` etc. but I haven't written that here yet
 - If an Id Char is encountered, chars are consumed up to (but excluding) the next non-Id Char. These
   form an identifier
 - If a Symbol Char is encountered, chars are consumed up to (but excluding) the next non-Symbol Char.
   These are then broken up into several Symbol Sets/Chars greedily from left to right, always taking
   the largest Symbol Set/Char that matche the start of the list of chars