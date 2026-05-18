#include "storage.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static char *escape_newlines(const char *text)
{
    /* 파일 내용을 한 줄로 저장하기 위해 실제 줄바꿈을 문자 "\n"으로 바꾼다. */
    const char *source = text == NULL ? "" : text;
    size_t size = 0;

    /* '\'와 줄바꿈은 저장할 때 두 글자로 늘어나므로 필요한 크기를 미리 계산한다. */
    for (const char *p = source; *p != '\0'; p++) {
        if (*p == '\\' || *p == '\n') {
            size += 2;
        } else {
            size++;
        }
    }

    char *escaped = (char *)malloc(size + 1);
    if (escaped == NULL) {
        return NULL;
    }

    char *out = escaped;
    for (const char *p = source; *p != '\0'; p++) {
        /* 원래 있던 "\n" 문자열이 실제 줄바꿈으로 오해되지 않도록 '\'도 저장용으로 바꾼다. */
        if (*p == '\\') {
            *out++ = '\\';
            *out++ = '\\';
        } else if (*p == '\n') {
            *out++ = '\\';
            *out++ = 'n';
        } else {
            *out++ = *p;
        }
    }
    *out = '\0';

    return escaped;
}

static char *restore_newlines(const char *text)
{
    /* 저장할 때 바꿔둔 "\n"과 "\\"를 다시 실제 문자로 복원한다. */
    const char *source = text == NULL ? "" : text;
    char *restored = (char *)malloc(strlen(source) + 1);
    if (restored == NULL) {
        return NULL;
    }

    char *out = restored;
    for (const char *p = source; *p != '\0'; p++) {
        /* 아는 escape만 변환하고, 모르는 escape는 원문이 최대한 유지되게 둔다. */
        if (*p == '\\' && p[1] != '\0') {
            p++;
            if (*p == 'n') {
                *out++ = '\n';
            } else if (*p == '\\') {
                *out++ = '\\';
            } else {
                *out++ = '\\';
                *out++ = *p;
            }
        } else {
            *out++ = *p;
        }
    }
    *out = '\0';

    return restored;
}

static void load_node(MiniOsContext *ctx, FILE *file)
{
    char input[4096];
    /* 저장 파일은 납작한 목록이므로 depth별 마지막 노드를 기억해 부모를 찾는다. */
    FsNode *NodeTable[100] = {0};
    NodeTable[0] = ctx->root;

    while (fgets(input, sizeof(input), file) != NULL)
    {
        input[strcspn(input, "\n")] = '\0';

        char *token;

        token = strtok(input, "|");
        if (token == NULL) continue;
        int depth = atoi(token);

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        FsNodeType type = (token[0] == 'D' ? FS_DIRECTORY : FS_FILE);


        token = strtok(NULL, "|");
        if (token == NULL) continue;
        int permissions = strtol(token, NULL, 8);
        
        token = strtok(NULL, "|");
        if (token == NULL) continue;
        char user[MINI_OS_NAME_MAX];
        strncpy(user, token, sizeof(user) - 1);
        user[sizeof(user) - 1] = '\0';
        
        token = strtok(NULL, "|");
        if (token == NULL) continue;
        char group[MINI_OS_NAME_MAX];
        strncpy(group, token, sizeof(group) - 1);
        group[sizeof(group) - 1] = '\0';

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        char name[256];
        strncpy(name, token, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        size_t content_size = 0;
        char *content = NULL;
        if (type == FS_FILE)
        {
            /* content_size 필드는 포맷을 맞추기 위해 먼저 읽고 지나간다. */
            token = strtok(NULL, "|");
            if (token == NULL) continue;
            content_size = (size_t)atoi(token);

            /* content는 마지막 필드라서 '|'가 들어 있어도 줄 끝까지 그대로 읽는다. */
            token = strtok(NULL, "\n");
            content = restore_newlines(token);
            if (content == NULL)
            {
                continue;
            }
            /* 저장된 문자열과 복원된 문자열의 길이가 다를 수 있으므로 다시 계산한다. */
            content_size = strlen(content);
        }

        NodeTable[depth] = fs_create_node(name, type);
        NodeTable[depth]->depth = depth;
        NodeTable[depth]->permissions = permissions;
        strncpy(NodeTable[depth]->user, user, sizeof(NodeTable[depth]->user) - 1);
        NodeTable[depth]->user[sizeof(NodeTable[depth]->user) - 1] = '\0';
        strncpy(NodeTable[depth]->group, group, sizeof(NodeTable[depth]->group) - 1);
        NodeTable[depth]->group[sizeof(NodeTable[depth]->user) - 1] = '\0';
        
        if (type == FS_FILE)
        {
            NodeTable[depth]->content_size = content_size;
            NodeTable[depth]->content = content;
        }

        fs_add_child(NodeTable[depth-1], NodeTable[depth]);
    }
}

static void save_node(FsNode *node, FILE *file)
{
    if (node == NULL)
    {
        return;
    }
    fprintf (file, "%d|%c|%o|%s|%s|%s", node->depth,
        node->type == FS_DIRECTORY ? 'D' : 'F', 
        node->permissions, node->user, node->group, node->name);
    if (node->type == FS_FILE)
    {
        /* fgets()가 노드 하나를 한 줄로 읽을 수 있도록 content의 줄바꿈을 escape한다. */
        char *content = escape_newlines(node->content);
        if (content == NULL) {
            return;
        }
        fprintf(file, "|%zu|%s", node->content_size, content);
        free(content);
    }
    fprintf(file, "\n");

    save_node(node->first_child, file);
    save_node(node->next_sibling, file);
}

MiniOsStatus storage_load_tree(MiniOsContext *ctx, const char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL) 
    {
        return MINI_OS_ERROR;
    }

    load_node(ctx, file);
    fclose(file);
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
