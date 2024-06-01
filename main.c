#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 256

// лекция 12 на сжатие данных

// сжимать надо по таблице а разжимать по дереву

typedef struct TreeNode {
    char symbol;
    struct TreeNode *left;
    struct TreeNode *right;
    double F; // частота встречаемости
} TreeNode;

typedef struct Heap {
    TreeNode **arr;
    int size;
    int cap;
} Heap;

typedef struct HuffmanCode {
    char *code;
    int length;
} HuffmanCode;

Heap* EmptyHeap(int capacity); // создание пустой кучи
void swap(TreeNode **a, TreeNode **b);
void minHeapify(Heap *heap, int ind); // функция для восстановления свойств кучи 
void Insert(Heap *heap, TreeNode *node); // вставка узла в кучу
void InsertHelper(Heap *heap, int ind);
TreeNode* ExtractMin(Heap* heap); // извлечение минимального узла
TreeNode* NewTreeFromSymbol(char sym, double F); // создание нового узла дерева
TreeNode* NewTreeFromTwoTrees(TreeNode* tree0, TreeNode* tree1); // создание нового узла для объединения поддеревьев
TreeNode* BuildHuffmanTree(int cap, int *f); // создание дерева хаффмана
void Compression(char *inputPath, char *outputPath); // функция для сжатия
void Decompression(char *inputPath, char *outputPath); // функция для разжатия
void BuildCodeTable(TreeNode* root, char* code, int depth, HuffmanCode** table);
void WriteTree(TreeNode* root, FILE* f_out); // сохранение структуры дерева в файл
TreeNode* ReadTree(FILE* f_in); // восстановление дерева из файла:

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Ошибка\n");
        return 0;
    }

    if (strcmp(argv[1], "c") == 0)
    {
        Compression(argv[2], argv[3]);
    } else if (strcmp(argv[1], "d") == 0)
    {
        Decompression(argv[2], argv[3]);
    } else
    {
        printf("Ошибка, введите 'c', если хотите сжать файл или 'd' для разжатия\n");
        return 0;
    }

    return 0;
}


void swap(TreeNode **a, TreeNode **b)
{
    TreeNode *temp = *a;
    *a = *b;
    *b = temp;
}

Heap* EmptyHeap(int capacity)
{
    Heap *heap = (Heap*) calloc(1, sizeof(Heap));
    heap->size = 0;
    heap->cap = capacity;
    heap->arr = (TreeNode**) calloc(capacity, sizeof(TreeNode*));
    return heap;
}

void minHeapify(Heap *heap, int ind)
{
    int smallest = ind;
    int left = 2 * ind + 1;
    int right = 2 * ind + 2;

    if (left < heap->size && heap->arr[left]->F < heap->arr[smallest]->F)
    {
        smallest = left;
    }

    if (right < heap->size && heap->arr[right]->F < heap->arr[smallest]->F)
    {
        smallest = right;
    }

    if (smallest != ind)
    {
        swap(&heap->arr[smallest], &heap->arr[ind]);
        minHeapify(heap, smallest);
    }
}

void Insert(Heap *heap, TreeNode *node)
{
    if (heap->size < heap->cap)
    {
        heap->arr[heap->size] = node;
        InsertHelper(heap, heap->size);
        heap->size++;
    }
}

void InsertHelper(Heap *heap, int ind)
{
    int parent = (ind - 1) / 2;
    if (heap->arr[parent]->F > heap->arr[ind]->F)
    {
        swap(&heap->arr[parent], &heap->arr[ind]);
        InsertHelper(heap, parent);
    }
}

TreeNode* ExtractMin(Heap* heap)
{
    TreeNode* minNode = heap->arr[0]; // вытаскиваем первый - минимальный
    heap->arr[0] = heap->arr[heap->size - 1]; // меняем первый на последний
    heap->size--; // убираем последний
    minHeapify(heap, 0);
    return minNode;
}

TreeNode* NewTreeFromSymbol(char sym, double F)
{
    TreeNode* res = (TreeNode*) malloc(sizeof(TreeNode));
    res->F = F;
    res->symbol = sym;
    res->left = res->right = NULL;
    return res;
}

TreeNode* NewTreeFromTwoTrees(TreeNode* tree0, TreeNode* tree1)
{
    TreeNode* res = (TreeNode*) malloc(sizeof(TreeNode));
    res->F = tree0->F + tree1->F;
    res->left = tree0;
    res->right = tree1;
    return res;
}

TreeNode* BuildHuffmanTree(int cap, int *f)
{
    Heap *heap = EmptyHeap(cap);
    TreeNode *tree0;
    TreeNode *tree1;
    for (int i = 0; i < cap; i++)
    {
        if (f[i] != 0)
        {
            Insert(heap, NewTreeFromSymbol((char)i, f[i]));
        }
    }
    while (heap->size > 1) //   for - seg fault
    {
        tree0 = ExtractMin(heap);
        tree1 = ExtractMin(heap);
        Insert(heap, NewTreeFromTwoTrees(tree0, tree1));
    }
    return ExtractMin(heap);
}


/**/

void BuildCodeTable(TreeNode* root, char* code, int depth, HuffmanCode** table)
{
    if (root->left == NULL && root->right == NULL)
    {
        code[depth] = '\0';
        table[(unsigned char)root->symbol]->code = strdup(code);
        table[(unsigned char)root->symbol]->length = depth;
    }
    if (root->left)
    {
        code[depth] = '0';
        BuildCodeTable(root->left, code, depth + 1, table);
    } 
    if (root->right)
    {
        code[depth] = '1';
        BuildCodeTable(root->right, code, depth + 1, table);
    }
}

void WriteTree(TreeNode* root, FILE* f_out)
{
    if (!root->left && !root->right)
    {
        fputc('L', f_out);  // лист
        fputc(root->symbol, f_out);
    } else
    {
        fputc('I', f_out);  // внутренний узел 
        WriteTree(root->left, f_out);
        WriteTree(root->right, f_out);
    }
}

TreeNode* ReadTree(FILE* f_in)
{
    char nodeType = fgetc(f_in);
    if (nodeType == 'L')
    {
        char symbol = fgetc(f_in);
        return NewTreeFromSymbol(symbol, 0);
    } else
    {
        TreeNode* left = ReadTree(f_in);
        TreeNode* right = ReadTree(f_in);
        return NewTreeFromTwoTrees(left, right);
    }
}

void Compression(char *inputPath, char *outputPath)
{
    FILE *f_in = fopen(inputPath, "rb");
    FILE *f_out = fopen(outputPath, "wb+");

    if (!f_in || !f_out)
    {
        printf("Ошибка при открытии файлов\n");
        return;
    }

    int *frequency = (int*) calloc(SIZE, sizeof(int));
    char ch;    
    while ((ch = fgetc(f_in)) != EOF) 
    {
        frequency[(unsigned char)ch]++;
    }

    TreeNode *root = BuildHuffmanTree(SIZE, frequency);
    if (!root)
    {
        printf("Ошибка построения дерева Хаффмана\n");
        fclose(f_in);
        fclose(f_out);
        free(frequency);
        return;
    }

    WriteTree(root, f_out);

    HuffmanCode** table = (HuffmanCode**) calloc(SIZE, sizeof(HuffmanCode*));
    for (int i = 0; i < SIZE; i++)
    {
        table[i] = (HuffmanCode*) calloc(1, sizeof(HuffmanCode));
    }
    
    char code[SIZE];
    BuildCodeTable(root, code, 0, table);
    
    fseek(f_in, 0, SEEK_SET); // возвращаемся в начало файла
    
    unsigned char byte = 0; // байт, в который мы собираем биты
    int length = 0, bitCount = 0; // cчётчик, который отслеживает, сколько битов мы сейчас добавили в байт
    char* codeStr;
    while ((ch = fgetc(f_in)) != EOF)
    {
        codeStr = table[(unsigned char)ch]->code;
        length = table[(unsigned char)ch]->length;
        
        for (int i = 0; i < length; i++)
        {
            byte = byte << 1 | (codeStr[i] - '0');
            bitCount++;
            if (bitCount == 8)
            {
                fwrite(&byte, sizeof(unsigned char), 1, f_out);
                byte = 0;
                bitCount = 0;
            }
        }
    }

    // остались биты, которые не вошли в полный байт
    if (bitCount > 0)
    {
        byte = byte << (8 - bitCount);
        fwrite(&byte, sizeof(unsigned char), 1, f_out);
    }


    for (int i = 0; i < SIZE; i++)
    {
        free(table[i]);
    }
    free(table);
    free(frequency);
    fclose(f_in);
    fclose(f_out);
}

void Decompression(char *inputPath, char *outputPath)
{
    FILE *f_in = fopen(inputPath, "rb");
    FILE *f_out = fopen(outputPath, "wb+");
    
    if (!f_in || !f_out)
    {
        printf("Ошибка при открытии файлов\n");
        return;
    }
    
    // Восстановим дерево из файла
    TreeNode *root = ReadTree(f_in);
    if (!root)
    {
        printf("Ошибка восстановления дерева Хаффмана\n");
        fclose(f_in);
        fclose(f_out);
        return;
    }

    TreeNode *current = root;
    unsigned char byte;
    int bitPos;
    
    while (fread(&byte, sizeof(unsigned char), 1, f_in))
    {
        for (bitPos = 7; bitPos >= 0; bitPos--)
        {
            // Идём по дереву Хаффмана
            if (byte & (1 << bitPos))
                current = current->right;
            else
                current = current->left;

            // Считываем символ, если достигли листа
            if (!current->left && !current->right)
            {
                fputc(current->symbol, f_out);
                current = root;  // Возвращаемся к корню
            }
        }
    }

    fclose(f_in);
    fclose(f_out);
}
