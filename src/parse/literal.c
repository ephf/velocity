#include "type.c"

#include "node/data/constant.c"
#include "node/data/variable.c"

Node Literal(str* tokenizer) {
    str token = next(tokenizer);

    switch(token.id) {
        case 'n': return parse_number_constant(token);
        case '"': return parse_string_constant(token);
        case 'i': return find_variable(token);
    }

    return unexpected_token(token);
}