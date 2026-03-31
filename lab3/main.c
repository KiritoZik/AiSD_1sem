#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAXSTACK 512

typedef struct Node {
    char value[32];
    struct Node* left;
    struct Node* right;
} Node;

Node* createNode(const char* val) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        fprintf(stderr, "malloc error\n");
        exit(1);
    }
    strncpy(newNode->value, val, sizeof(newNode->value) - 1);
    newNode->value[sizeof(newNode->value) - 1] = '\0';
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

void freeTree(Node* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

// Стек узлов
typedef struct {
    Node* data[MAXSTACK];
    int top;
} NodeStack;

// Стек операторов и скобок.
typedef struct {
    char data[MAXSTACK];
    int top;
} OpStack;

// Инициализация/операции со стеком узлов.
static void ns_init(NodeStack* s) { s->top = -1; }
static int ns_push(NodeStack* s, Node* node) {
    if (s->top + 1 >= MAXSTACK) return 0;
    s->data[++s->top] = node;
    return 1;
}
static Node* ns_pop(NodeStack* s) { return (s->top >= 0) ? s->data[s->top--] : NULL; }
static void ns_clear(NodeStack* s) {
    while (s->top >= 0) {
        Node* n = s->data[s->top--];
        if (!n) continue;
        freeTree(n);
    }
}

// Инициализация/операции со стеком операторов.
static void os_init(OpStack* s) { s->top = -1; }
static int os_push(OpStack* s, char op) {
    if (s->top + 1 >= MAXSTACK) return 0;
    s->data[++s->top] = op;
    return 1;
}
static char os_pop(OpStack* s) { return (s->top >= 0) ? s->data[s->top--] : 0; }
static char os_peek(const OpStack* s) { return (s->top >= 0) ? s->data[s->top] : 0; }
static int os_empty(const OpStack* s) { return s->top < 0; }

// Приоритет операций для Дейкстры.
int priority(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

// Свернуть один оператор: взять op из стека операторов и 2 узла из стека узлов, собрать поддерево.
static int buildSubTree(NodeStack* nodes, OpStack* ops) {
    if (os_empty(ops)) return 0;
    if (nodes->top < 1) return 0;

    char opCh = os_pop(ops);
    char opStr[2] = { opCh, '\0' };
    Node* opNode = createNode(opStr);

    opNode->right = ns_pop(nodes);
    opNode->left = ns_pop(nodes);

    if (!ns_push(nodes, opNode)) {
        freeTree(opNode->left);
        freeTree(opNode->right);
        free(opNode);
        return 0;
    }

    return 1;
}

// Построить дерево выражения по алгоритму Дейкстры (2 стека: операторы и операнды).
Node* buildTree(const char* expr) {
    NodeStack nodes;
    OpStack ops;
    ns_init(&nodes);
    os_init(&ops);
    char prev = 0;

    for (int i = 0; expr[i] != '\0'; i++) {
        char token = expr[i];
        if (isspace((unsigned char)token)) continue;

        if (isdigit((unsigned char)token) || token == '.' ||
            (token == '-' && (prev == 0 || prev == '(' || prev == 'o') &&
             (isdigit((unsigned char)expr[i + 1]) || expr[i + 1] == '.'))) {
            char buffer[32];
            int j = 0;
            int dotSeen = 0;

            if (token == '-') {
                buffer[j++] = expr[i++];
            }

            while (isdigit((unsigned char)expr[i]) || expr[i] == '.') {
                if (expr[i] == '.') {
                    if (dotSeen) {
                        ns_clear(&nodes);
                        return NULL;
                    }
                    dotSeen = 1;
                }
                if (j >= 31) {
                    ns_clear(&nodes);
                    return NULL;
                }
                buffer[j++] = expr[i++];
            }
            buffer[j] = '\0';
            i--;
            if (!ns_push(&nodes, createNode(buffer))) {
                ns_clear(&nodes);
                return NULL;
            }
            prev = 'n';
        }
        else if (token == '(') {
            if (!os_push(&ops, token)) {
                ns_clear(&nodes);
                return NULL;
            }
            prev = '(';
        }
        else if (token == ')') {
            while (!os_empty(&ops) && os_peek(&ops) != '(') {
                if (!buildSubTree(&nodes, &ops)) {
                    ns_clear(&nodes);
                    return NULL;
                }
            }
            if (os_empty(&ops)) {
                ns_clear(&nodes);
                return NULL;
            }
            os_pop(&ops);
            prev = ')';
        }
        else {
            if (!(token == '+' || token == '-' || token == '*' || token == '/')) {
                ns_clear(&nodes);
                return NULL;
            }
            while (!os_empty(&ops) && os_peek(&ops) != '(' && priority(os_peek(&ops)) >= priority(token)) {
                if (!buildSubTree(&nodes, &ops)) {
                    ns_clear(&nodes);
                    return NULL;
                }
            }
            if (!os_push(&ops, token)) {
                ns_clear(&nodes);
                return NULL;
            }
            prev = 'o';
        }
    }

    while (!os_empty(&ops)) {
        if (os_peek(&ops) == '(') {
            ns_clear(&nodes);
            return NULL;
        }
        if (!buildSubTree(&nodes, &ops)) {
            ns_clear(&nodes);
            return NULL;
        }
    }

    if (nodes.top != 0) {
        ns_clear(&nodes);
        return NULL;
    }
    return ns_pop(&nodes);
}


void transformTree(Node** rootPtr) {
    if (*rootPtr == NULL) return;

    transformTree(&((*rootPtr)->left));
    transformTree(&((*rootPtr)->right));

    Node* root = *rootPtr;

    // Ищем паттерн (a/b)*(c/d)
    if (strcmp(root->value, "*") == 0 &&
        root->left && strcmp(root->left->value, "/") == 0 &&
        root->right && strcmp(root->right->value, "/") == 0) {

        Node* leftDiv = root->left;
        Node* rightDiv = root->right;

        Node* newNum = createNode("*");
        newNum->left = leftDiv->left;
        newNum->right = rightDiv->left;

        Node* newDen = createNode("*");
        newDen->left = leftDiv->right;
        newDen->right = rightDiv->right;

        strcpy(root->value, "/");
        root->left = newNum;
        root->right = newDen;

        free(leftDiv);
        free(rightDiv);
    }
}

// Печать дерева
void printTree(Node* root, int level) {
    if (root == NULL) return;
    printTree(root->right, level + 1);
    for (int i = 0; i < level; i++) printf("       ");
    printf("%s\n", root->value);
    printTree(root->left, level + 1);
}

// Печать выражения в инфиксном виде со скобками.
void printInfix(Node* root) {
    if (root == NULL) return;
    int isOp = (root->left || root->right);
    if (isOp) printf("(");
    printInfix(root->left);
    printf("%s", root->value);
    printInfix(root->right);
    if (isOp) printf(")");
}

// Тест
void runTest(const char* expr) {
    printf("==================================================\n");
    printf("ИСХОДНОЕ ВЫРАЖЕНИЕ: %s\n", expr);

    Node* tree = buildTree(expr);
    if (!tree) {
        printf("Ошибка разбора выражения.\n");
        return;
    }

    printf("\nТекст ДО: ");
    printInfix(tree);
    printf("\n");

    printf("\nДерево ДО преобразования:\n");
    printTree(tree, 0);

    transformTree(&tree);

    printf("\n--------------------------------------------------\n");
    printf("Текст ПОСЛЕ: ");
    printInfix(tree);
    printf("\n");

    printf("Дерево ПОСЛЕ преобразования:\n");
    printTree(tree, 0);

    printf("==================================================\n\n");

    freeTree(tree);
}

int main() {
    char expr[2048];
    printf("Введите выражение (Enter для встроенных тестов):\n");
    if (fgets(expr, sizeof(expr), stdin)) {
        int onlySpaces = 1;
        for (int i = 0; expr[i] != '\0'; i++) {
            if (!isspace((unsigned char)expr[i])) { onlySpaces = 0; break; }
        }
        if (!onlySpaces) {
            runTest(expr);
            return 0;
        }
    }

    runTest("(10/2)*(6/3)");
    runTest("(15.5/3.1)*(100/10)");
    runTest("5+((8/2)*(9/3))");
    return 0;
}