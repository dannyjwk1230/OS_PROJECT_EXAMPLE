#include <stdio.h>
#include <string.h>
#include "commands/basic.h"
#include "fs.h"

static void print_node_path(const FsNode *node)
{
    const FsNode *stack[128];
    int count = 0;

    for (const FsNode *current = node; current != NULL && count < 128; current = current->parent) {
        stack[count++] = current;
    }

    if (count <= 1) {
        printf("/\n");
        return;
    }

    for (int i = count - 2; i >= 0; i--) {
        printf("/%s", stack[i]->name);
    }
    printf("\n");
}

static int search_tree(const FsNode *node, const char *keyword)
{
    int matches = 0;

    if (node == NULL) {
        return 0;
    }

    if (strstr(node->name, keyword) != NULL) {
        print_node_path(node);
        matches++;
    }

    for (const FsNode *child = node->first_child; child != NULL; child = child->next_sibling) {
        matches += search_tree(child, keyword);
    }

    return matches;
}

MiniOsStatus cmd_search(MiniOsContext *ctx, int argc, char **argv)
{
    if (ctx == NULL || ctx->root == NULL) {
        fprintf(stderr, "search: file system is not ready\n");
        return MINI_OS_ERROR;
    }

    if (argc != 2) {
        fprintf(stderr, "usage: search <name>\n");
        return MINI_OS_ERROR;
    }

    if (argv[1][0] == '\0') {
        fprintf(stderr, "search: empty search keyword\n");
        return MINI_OS_ERROR;
    }

    int matches = search_tree(ctx->root, argv[1]);
    if (matches == 0) {
        printf("search: no matches found for '%s'\n", argv[1]);
    }

    return MINI_OS_SUCCESS;
}
