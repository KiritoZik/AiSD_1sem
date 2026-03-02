#include <stdio.h>
#include <stdlib.h>

typedef struct TreeNode{
    int value;
    struct TreeNode* first_child;
    struct TreeNode* next_brother;
} TreeNode;

typedef struct Tree{
    TreeNode* root;
    int knotCount;
} Tree;

TreeNode* create_node(int value){
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    if(!newNode){fprintf(stderr, "ERROR MALLOC"); exit(EXIT_FAILURE);}
    newNode -> value = value;
    newNode -> first_child = NULL;
    newNode -> next_brother = NULL;
    return newNode;
}

Tree* create_tree(int value){
    Tree* newTree = (Tree*)malloc(sizeof(Tree));
    if(!newTree){fprintf(stderr, "ERROR MALLOC"); exit(EXIT_FAILURE);}
    TreeNode* root = create_node(value);
    newTree -> root = root;
    newTree -> knotCount = 1;
    return newTree;
}

TreeNode* find_node(TreeNode* nowNode, int nowValue){
    if(!nowNode) return NULL;

    if((nowNode->value) == nowValue){
        return nowNode;
    }

    TreeNode* find_in_child = find_node(nowNode->first_child, nowValue);

    if(find_in_child != NULL){
        return find_in_child;
    }

    return find_node(nowNode->next_brother, nowValue);
}

void add_knot(Tree* myTree, int parentValue, int value){
    TreeNode* parent = find_node(myTree->root, parentValue);
    if(!parent){fprintf(stderr, "NOT FIND PARENT"); return;}
    if(parent->first_child == NULL){
        parent->first_child = create_node(value);
    }else{
        TreeNode* current = parent -> first_child;
        while(current -> next_brother != NULL){
            current = current -> next_brother;
        }
        current -> next_brother = create_node(value);
    }
    myTree -> knotCount ++;
}

void print_tree(TreeNode* nowNode, int level){
    // добавить проверку на пустот
    if(!nowNode) return;
    printf("%*s", level*4, "");
    printf("%d\n", nowNode->value);
    print_tree(nowNode->first_child, level+1);
    print_tree(nowNode->next_brother, level);
}

int count_listik(TreeNode* nowNode){
    if(nowNode==NULL) return 0;

    if(nowNode->first_child == NULL){
        return 1 + count_listik(nowNode->next_brother);
    }

    return count_listik(nowNode->first_child) + count_listik(nowNode->next_brother);
}

TreeNode* cut_delete_node(TreeNode* root, int valueChild){
    if(root==NULL) return NULL;
    
    //delete if node is first_child
    if(root->first_child != NULL && root->first_child->value == valueChild){
        TreeNode* target = root -> first_child;
        root -> first_child = target -> next_brother;
        target -> next_brother = NULL;
        return target;
    }

    //delete if node is brother
    TreeNode* child = root->first_child;
    while(child != NULL && child->next_brother != NULL){
        if(child->next_brother->value == valueChild){
            TreeNode* target = child->next_brother;
            child->next_brother = target -> next_brother;
            target -> next_brother = NULL;
            return target;
        }
        child = child -> next_brother;
    }

    //in depth and in width
    TreeNode* res = cut_delete_node(root->first_child, valueChild);
    if (res) return res;
    return cut_delete_node(root->next_brother, valueChild);
}

int clear_node(TreeNode* node){
    if(!node) return 0;
    int count = 1;

    TreeNode* child = node -> first_child;
    while(child){
        TreeNode* next = child -> next_brother;
        count += clear_node(child);
        child = next;
    }
    free(node);
    return count;
}

void delete_node(Tree* myTree, int delete_value){
    if(myTree -> root-> value == delete_value){fprintf(stderr, "КОРЕНЬ УДАЛИТЬ НЕЛЬЗЯ"); return;}

    TreeNode* target = cut_delete_node(myTree -> root, delete_value);

    if(target){
        int deletedCount = clear_node(target);
        myTree -> knotCount -= deletedCount;
        printf("Удалено узлов: %d\n", deletedCount);
    }else{
        printf("Узел %d не найден!\n", delete_value);
    }

}

int main(void){
    int n, koren;
    int parentValue, value, delete_value;
    printf("Input koren tree\n");
    scanf("%d", &koren);
    Tree* myTree = create_tree(koren);
    while(1){
        printf("1 - add knot\n2 - print tree\n3 - count listik\n4 - delete node\n5 - exit\n");
        scanf("%d", &n);
        switch(n){
            case 1:
            scanf("%d %d", &parentValue, &value);
            add_knot(myTree, parentValue, value);
            break;

            case 2:
            printf("Tree:\n");
            print_tree(myTree->root, 0);
            break;

            case 3:
            printf("%d\n", count_listik(myTree->root));
            break;

            case 4:
            scanf("%d", &delete_value);
            delete_node(myTree, delete_value);
            break;

            case 5:
            return 0;
        }
        printf("\n");
        printf("------------------\n");
    }
}


