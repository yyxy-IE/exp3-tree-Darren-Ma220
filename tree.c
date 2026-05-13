/**
 * 实验：目录树查看器（仿 Linux tree 命令）
 * 学号：2504020305  姓名：马超群
 * 说明：请补全所有标记为 TODO 的函数体，不要修改其他代码。
 * 目录树查看器（仿 Linux tree 命令）
 * 完整实现版本（C语言，左孩子右兄弟二叉树）
 * 编译：gcc -o tree tree.c -std=c99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// ================== 二叉树结点定义 ==================
typedef struct FileNode {
    char *name;                  // 文件/目录名
    int isDir;                   // 1:目录 0:文件
    struct FileNode *firstChild; // 左孩子：第一个子项
    struct FileNode *nextSibling;// 右兄弟：下一个同层项
} FileNode;

// ================== 函数声明 ==================
FileNode* createNode(const char *name, int isDir);
int cmpNode(const void *a, const void *b);
FileNode* buildTree(const char *path);
void printTree(FileNode *node, const char *prefix, int isLast);
int countNodes(FileNode *root);
int countLeaves(FileNode *root);
int treeHeight(FileNode *root);
void countDirFile(FileNode *root, int *dirs, int *files);
void freeTree(FileNode *root);
char* getBaseName(void);

// ================== 需要补全的函数 ==================

// 创建新结点（分配内存、复制字符串、初始化指针）
FileNode* createNode(const char *name, int isDir) {
    FileNode *node = (FileNode *)malloc(sizeof(FileNode));
    if (node == NULL)
        return NULL;

    node->name = (char *)malloc(strlen(name) + 1);
    strcpy(node->name, name);
    node->isDir = isDir;
    node->firstChild = NULL;
    node->nextSibling = NULL;
    return node;
}

// 比较函数，用于 qsort 对子项按名称排序
int cmpNode(const void *a, const void *b) {
    FileNode *na = *(FileNode **)a;
    FileNode *nb = *(FileNode **)b;
    return strcmp(na->name, nb->name);
}

// 递归构建目录树（核心难点）
FileNode* buildTree(const char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL)
        return NULL;

    // 提取当前目录名
    char *lastSlash = strrchr(path, '/');
    char curName[256];
    if (lastSlash == NULL)
        strcpy(curName, path);
    else
        strcpy(curName, lastSlash + 1);

    FileNode *cur = createNode(curName, 1);
    FileNode **childArr = NULL;
    int childNum = 0;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        // 跳过 . 和 ..
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        // 拼接完整路径
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, ent->d_name);

        struct stat st;
        if (stat(fullPath, &st) == -1)
            continue;

        FileNode *child;
        if (S_ISDIR(st.st_mode)) {
            child = buildTree(fullPath);
        } else {
            child = createNode(ent->d_name, 0);
        }

        // 加入临时数组
        childArr = (FileNode **)realloc(childArr, (childNum + 1) * sizeof(FileNode *));
        childArr[childNum++] = child;
    }
    closedir(dir);

    // 排序
    qsort(childArr, childNum, sizeof(FileNode *), cmpNode);

    // 链接成兄弟链表
    if (childNum > 0) {
        cur->firstChild = childArr[0];
        for (int i = 0; i < childNum - 1; i++) {
            childArr[i]->nextSibling = childArr[i + 1];
        }
    }

    free(childArr);
    return cur;
}

// 树形输出（仿 tree 命令）
void printTree(FileNode *node, const char *prefix, int isLast) {
    if (node == NULL)
        return;

    printf("%s", prefix);
    printf(isLast ? "`-- " : "|-- ");
    printf("%s", node->name);
    if (node->isDir)
        printf("/");
    printf("\n");

    if (node->firstChild == NULL)
        return;

    // 统计孩子总数
    int total = 0;
    FileNode *p = node->firstChild;
    while (p) {
        total++;
        p = p->nextSibling;
    }

    // 递归打印每个孩子
    p = node->firstChild;
    int idx = 0;
    while (p) {
        char newPrefix[1024];
        snprintf(newPrefix, sizeof(newPrefix), "%s%s", prefix, isLast ? "    " : "|   ");
        int last = (++idx == total);
        printTree(p, newPrefix, last);
        p = p->nextSibling;
    }
}

// 统计二叉树结点总数
int countNodes(FileNode *root) {
    if (root == NULL)
        return 0;
    int cnt = 1;
    // 遍历所有孩子
    FileNode *p = root->firstChild;
    while (p) {
        cnt += countNodes(p);
        p = p->nextSibling;
    }
    return cnt;
}

// 统计叶子结点数（firstChild == NULL 的结点）
int countLeaves(FileNode *root) {
    if (root == NULL)
        return 0;
    // 无孩子就是叶子
    if (root->firstChild == NULL)
        return 1;

    int cnt = 0;
    FileNode *p = root->firstChild;
    while (p) {
        cnt += countLeaves(p);
        p = p->nextSibling;
    }
    return cnt;
}

// 计算二叉树高度（根深度为1，空树高度为0）
int treeHeight(FileNode *root) {
    if (root == NULL)
        return 0;

    int maxH = 0;
    FileNode *p = root->firstChild;
    while (p) {
        int h = treeHeight(p);
        if (h > maxH)
            maxH = h;
        p = p->nextSibling;
    }
    return maxH + 1;
}

// 统计目录数和文件数（遍历整棵树）
void countDirFile(FileNode *root, int *dirs, int *files) {
    if (root == NULL)
        return;

    if (root->isDir)
        (*dirs)++;
    else
        (*files)++;

    FileNode *p = root->firstChild;
    while (p) {
        countDirFile(p, dirs, files);
        p = p->nextSibling;
    }
}

// 释放整棵树的内存
void freeTree(FileNode *root) {
    if (root == NULL)
        return;
   //递归释放所有孩子 
   FileNode *p = root->firstChild;
     while (p) {
         FileNode *next = p->nextSibling;
         freeTree(p);
         p = next;
     }
     free(root->name);
     free(root);
 }
 // 获取当前工作目录的“基本名称”（用于显示根结点名）
 char* getBaseName(void) {
     char *path = getcwd(NULL, 0);
     if (path == NULL)
         return NULL;
     char *p = strrchr(path, '/');
     char *name;
     if (p == NULL)
         name = strdup(path);
     else
         name = strdup(p + 1);
     free(path);
     return name;
 }
 int main(int argc, char *argv[]) {
     char targetPath[1024];
     if (argc >= 2) {
         strncpy(targetPath, argv[1], sizeof(targetPath)-1);
         targetPath[sizeof(targetPath)-1] = '\0';
     } else {
         if (getcwd(targetPath, sizeof(targetPath)) == NULL) {
            perror("getcwd");
             return 1;
         }
     }
     int len = strlen(targetPath);
     if (len > 0 && targetPath[len-1] == '/')
         targetPath[len-1] = '\0';
     struct stat st;
     if (stat(targetPath, &st) != 0) {
         perror("stat");
         return 1;
     }
     if (!S_ISDIR(st.st_mode)) {
         fprintf(stderr, "错误: %s 不是目录\n", targetPath);
         return 1;
     }
     FileNode *root = buildTree(targetPath);
     if (!root) {
         fprintf(stderr, "无法构建目录树\n");
         return 1;
     }
     // 输出根目录名
     char *displayName = NULL;
     if (argc >= 2) {
         displayName = root->name;
     } else {
         displayName = getBaseName();
     }
     printf("%s/\n", displayName);
     if (argc < 2) free(displayName);
     FileNode *child = root->firstChild;
     int childCount = 0;
     FileNode *tmp = child;
     while (tmp) { childCount++; tmp = tmp->nextSibling; }
     int idx = 0;
     while (child) {
         int isLast = (++idx == childCount);
         child = child->nextSibling;
     }
     int dirs = 0, files = 0;
     countDirFile(root, &dirs, &files);
     printf("\n%d 个目录, %d 个文件\n", dirs, files);
     printf("二叉树结点总数: %d\n", countNodes(root));
     printf("叶子结点数: %d\n", countLeaves(root));
     printf("树的高度: %d\n", treeHeight(root));
     freeTree(root);
     return 0;
 }