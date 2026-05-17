#include "storage.h"
#include <stdio.h>

static void save_node(FsNode *node, FILE *file)
{
    if (node == NULL)
    {
        return;
    }
    for(int i = 0; i < node->depth; i++)
    { 
        fprintf(file, "   ");
    }
    fprintf (file, "%c|%d|%o|%s|%zu", 
        node->type == FS_DIRECTORY ? 'D' : 'F', node->hidden, 
        node->permissions, node->name, node->content_size);
    if (node->type == FS_FILE)
    {
        fprintf(file, "|%s", node->content);
    }
    fprintf(file, "\n");

    save_node(node->first_child, file);
    save_node(node->next_sibling, file);
}

MiniOsStatus storage_load_tree(MiniOsContext *ctx, const char *path)
{
    (void)ctx;
    (void)path;
    return MINI_OS_SUCCESS;
}

MiniOsStatus storage_save_tree(const MiniOsContext *ctx, const char *path)
{
    FILE *file = fopen(path, "w");
    if (file == NULL) 
    {
        return MINI_OS_ERROR;
    }

    save_node(ctx->root->first_child, file);
    fclose(file);
    return MINI_OS_SUCCESS;
}
