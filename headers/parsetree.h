#ifndef SPACE_PARSETREE_H_
#define SPACE_PARSETREE_H_

#include "../headers/Token.h"

struct Node {
    char *value;
    TOKENTYPES type;
    struct Node *children;
};

#endif  // SPACE_PARSETREE_H_