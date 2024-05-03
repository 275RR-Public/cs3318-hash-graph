/*
*   James Hofer
*   1000199225
*   Compile: /usr/bin/c99 -o app5 app5.c
*
*   Usage1: ./app5              (for typing or pasting input)
*   Usage2: ./app5 < input.dat  (for input redirection)
*
*   This program reads a list of edges and vertices from stdin and
*   prints the results of a double hash table used to store the vertices.
*   It also prints the total number of probes required for all insertions.
*   It then prints the SCCs in topological order.
*
*   Note1: Reduced probe count by over 50% compared to a,b, and c.out   
*   Note2: Eliminated searches with increased hash struct size
*
*   https://ranger.uta.edu/~weems/NOTES3318/LAB/LAB5FALL23/lab5fall23.pdf
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define UNUSED -1
#define MAX_NAME_LEN 25

#define WHITE 0
#define GRAY 1
#define BLACK 2
 
// GLOBALS
struct Item {

    int key;    // -1 (unused) or vertex number
    int name;   // lookup number for vertex name
    int h1;     // hash1
    int h2;     // hash2
    int probe;  // number of probes for insertion
 
};
typedef struct Item hashtable;

int tableSize;
hashtable* hashTableArray;

// GLOBALS for dfsSCC.c
int edges[100];
int edgeCount = 0;

struct edge {
  int tail,head,type; 
};
typedef struct edge edgeType;

int n;  // number of nodes
int e;  // number of edges
int finishIndex;

edgeType *edgeTab;
int *firstEdge;  // Table indicating first in range of edges with a common tail

int *vertexStatus,*secondDFSrestarts;


// **********Start of dfsSCC.c functions**************
int tailThenHead(const void* xin, const void* yin)
// Used in calls to qsort() and bsearch() for read_input_file()
{
    int result;
    edgeType *x,*y;

    x=(edgeType*) xin;
    y=(edgeType*) yin;
    result=x->tail - y->tail;
    if (result!=0)
    return result;
    else
    return x->head - y->head;
}


void read_input_file()
{
    int a,b,i,j;
    edgeType work;
    edgeType *ptr;

    edgeTab=(edgeType*) malloc(e*sizeof(edgeType));
    if (!edgeTab)
    {
        printf("edgeTab malloc failed %d\n",__LINE__);
        exit(0);
    }

    // read edges
    for (i=0; i<e; i++)
    {
        a = edges[i*2];
        b = edges[(i*2)+1];
        if (a<0 || a>=n || b<0 || b>=n)
        {
            printf("Invalid input %d %d at %d\n",a,b,__LINE__);
            exit(0);
        }
        edgeTab[i].tail=a;
        edgeTab[i].head=b;
    }

    // sort edges
    qsort(edgeTab,e,sizeof(edgeType),tailThenHead);

    // Coalesce duplicates into a single edge
    j=0;
    for (i=1; i<e; i++)
    if (edgeTab[j].tail==edgeTab[i].tail
        && edgeTab[j].head==edgeTab[i].head)
        ; 
    else
    {
        j++;
        edgeTab[j].tail=edgeTab[i].tail;
        edgeTab[j].head=edgeTab[i].head;
    }
    e=j+1;

    // For each vertex as a tail, determine first in range of edgeTab entries
    firstEdge=(int*) malloc((n+1)*sizeof(int));
    if (!firstEdge)
    {
        printf("malloc failed %d\n",__LINE__);
        exit(0);
    }
    j=0;
    for (i=0; i<n; i++)
    {
        firstEdge[i]=j;
        for ( ;
            j<e && edgeTab[j].tail==i;
            j++)
            ;
    }
    firstEdge[n]=e;
}


void DFSvisit(int u)
{
    int i,v;

    vertexStatus[u]=GRAY;

    for (i=firstEdge[u];i<firstEdge[u+1];i++)
    {
        v=edgeTab[i].head;
        if (vertexStatus[v]==WHITE)
            DFSvisit(v);
    }
    vertexStatus[u]=BLACK;
    secondDFSrestarts[--finishIndex]=u;
}


void reverseEdges()
{
    int a,b,i,j;
    edgeType work;
    edgeType *ptr;

    for (i=0; i<e; i++)
    {
        a=edgeTab[i].tail;
        b=edgeTab[i].head;
        edgeTab[i].tail=b;
        edgeTab[i].head=a;
    }

    // sort edges
    qsort(edgeTab,e,sizeof(edgeType),tailThenHead);

    // For each vertex as a tail, determine first in range of edgeTab entries
    if (!firstEdge)
    {
        printf("malloc failed %d\n",__LINE__);
        exit(0);
    }
    j=0;
    for (i=0; i<n; i++)
    {
        firstEdge[i]=j;
        for ( ;
            j<e && edgeTab[j].tail==i;
            j++)
            ;
    }
    firstEdge[n]=e;
}


void DFSvisit2(int u, char** vertNamesArray)
{
    int i,v;

    printf("%s\n", vertNamesArray[u]); // Indicate that u is in SCC for this restart
    vertexStatus[u]=GRAY;

    for (i=firstEdge[u];i<firstEdge[u+1];i++)
    {
        v=edgeTab[i].head;
        if (vertexStatus[v]==WHITE)
            DFSvisit2(v, vertNamesArray);
    }
    vertexStatus[u]=BLACK;
}


void dfsSCC(char** vertNamesArray) {
    int u,i,j,k,nextDFS;
    int SCCcount=0;

    read_input_file();

    vertexStatus=(int*) malloc(n*sizeof(int));
    secondDFSrestarts=(int*) malloc(n*sizeof(int));
    if (!vertexStatus || !secondDFSrestarts)
    {
        printf("malloc failed\n");
        exit(0);
    }
    // DFS code
    for (u=0;u<n;u++)
        vertexStatus[u]=WHITE;
    finishIndex=n;
    for (u=0;u<n;u++)
        if (vertexStatus[u]==WHITE)
            DFSvisit(u);

    reverseEdges();

    // DFS code
    for (u=0;u<n;u++)
        vertexStatus[u]=WHITE;
    for (i=0;i<n;i++)
        if (vertexStatus[secondDFSrestarts[i]]==WHITE)
        {
            SCCcount++;
            printf("SCC %d\n",SCCcount);
            DFSvisit2(secondDFSrestarts[i], vertNamesArray);
        }

    free(edgeTab);
    free(firstEdge);
    free(vertexStatus);
    free(secondDFSrestarts);
}
// **********End of dfsSCC.c functions**************


/**
 * @brief finds the closet but larger prime number to given number
 *        ( min(prime) > num )
 *
 * @param num - the given number
 * @return int
*/
int findPrime(int num) {
    int i, j;
    bool isPrime;

    for(i = num + 1; ; i++) {
        isPrime = true;
        for(j = 2; j < i; j++) {
            if(i % j == 0) {
                isPrime = false;
                break;
            }
        }
        if(isPrime)
            return i;
    }
}


/**
 * @brief prints the results of the vertex to names lookup table,
 *        double hash table, and total probes
 * 
 * @param totalVerts - total number of vertices
 * @param vertNamesArray - array of vertex names
*/
void print(int totalVerts, char** vertNamesArray) {
    
    printf("Double hash table size is %d\n", tableSize);
    printf("Names in order of first appearance:\n");
    for(int i = 0; i < totalVerts; i++) {
        if(vertNamesArray[i])
            printf("%d %s\n", i, vertNamesArray[i]);
        else {
            printf("Disparaging Message!\n");
            exit(EXIT_FAILURE);
        }
    }
    printf("Double hash table used while processing input:\n");
    int probeSum = 0;
    for(int i = 0; i < tableSize; i++) {
        if(hashTableArray[i].key != UNUSED) {
            int key = hashTableArray[i].key;
            int name = hashTableArray[i].name;
            char* vertName = vertNamesArray[name];
            int h1 = hashTableArray[i].h1;
            int h2 = hashTableArray[i].h2;
            int probes = hashTableArray[i].probe;
            printf("%d %d (%s) %d %d: total probes %d\n", key, name, vertName, h1, h2, probes);
            probeSum += probes;
        }
        else {
            printf("%d %d\n", i, UNUSED);
        }
    }
    printf("Total probes: %d\n", probeSum);
}


/**
 * @brief inserts a vertex into the name lookup table,
 *        double hash table, and edges array for dfsSCC.c
 * 
 * @param key - vertex name
 * @param totalVerts - total number of vertices
 * @param vertNamesArray - array of vertex names
*/
void insert(char* key, int totalVerts, char** vertNamesArray) {
    
    if(key == NULL) {
        printf("Insert Error %d\n",__LINE__);
        exit(EXIT_FAILURE);
    }
    
    // add vertex name to lookup array
    int namesIndex = UNUSED;
    for(int i = 0; i < totalVerts; i++) {
        if(vertNamesArray[i] != NULL && strcmp(vertNamesArray[i], key) == 0) {
            //printf("Dup: %s = %s\n", key, vertNamesArray[i]);
            edges[edgeCount] = i;
            edgeCount++;
            return;
        }
        if(strcmp(vertNamesArray[i], "unused") == 0) {
            strcpy(vertNamesArray[i], key);
            namesIndex = i;
            //printf("%d %s: ", namesIndex, vertNamesArray[i]);
            edges[edgeCount] = i;
            edgeCount++;
            break;
        }
    }
    
    // calculate hash1
    int hash1 = 0;
    for(int i = 0; key[i] != '\0'; i++)
        hash1 = (hash1*10 + key[i]) % tableSize;

    // calculate hash2
    int hash2 = 0;
    for(int i = 0; key[i] != '\0'; i++)
        hash2 = (1 + (hash2*10 + key[i])) % (tableSize - 1);

    // insert vertex into hashtable
    int hashIndex = hash1;
    int probe = 0;
    while(hashTableArray[hashIndex].key != UNUSED) {
        hashIndex = (hash1 + probe*hash2) % tableSize;
        probe++;
    }
    hashTableArray[hashIndex].key = hashIndex;
    hashTableArray[hashIndex].name = namesIndex;
    hashTableArray[hashIndex].h1 = hash1;
    hashTableArray[hashIndex].h2 = hash2;
    hashTableArray[hashIndex].probe = probe;
}


/**
 * @brief Main
 * 
 * @return int 
*/
int main() {
    
    // check if redirected input
    if(isatty(0))
        printf("Type input by line or Paste entire input.\n");

    // read first line of input
    int totalVerts, totalEdges;
    scanf("%d %d", &totalVerts, &totalEdges);
    n = totalVerts;
    e = totalEdges;
    
    // find load factor < 50%
    tableSize = findPrime(totalVerts*2);

    // create lookup array and hashtable array
    char* vertNamesArray[totalVerts];
    hashTableArray = (hashtable*) malloc(tableSize * sizeof(hashtable));
    if(!hashTableArray) {
        printf("Malloc Error %d\n",__LINE__);
        exit(EXIT_FAILURE);
    }

    // initialize both arrays
    for(int i = 0; i < totalVerts; i++) {
        vertNamesArray[i] = (char*) malloc(MAX_NAME_LEN * sizeof(char));
        if(!vertNamesArray[i]) {
            printf("Malloc Error %d\n",__LINE__);
            exit(EXIT_FAILURE);
        }
        strcpy(vertNamesArray[i], "unused");
    }
    for(int i = 0; i < tableSize; i++) {
        hashTableArray[i].key = UNUSED;
        hashTableArray[i].name = UNUSED;
        hashTableArray[i].h1 = UNUSED;
        hashTableArray[i].h2 = UNUSED;
        hashTableArray[i].probe = UNUSED;
    }

    // read remaining lines of input
    char vert[MAX_NAME_LEN];
    char edge[MAX_NAME_LEN];
    char temp1[MAX_NAME_LEN];
    char temp2[MAX_NAME_LEN];
    while(1) {

        scanf("%s %s", vert, edge);
        if(strcmp(vert, temp1) == 0 && strcmp(edge, temp2) == 0)
            break;
        //printf("print: %s %s\n", vert, edge);
        insert(vert, totalVerts, vertNamesArray);
        insert(edge, totalVerts, vertNamesArray);
        strcpy(temp1, vert);
        strcpy(temp2, edge);
    }

    // print results
    print(totalVerts, vertNamesArray);

    // print SCCs
    dfsSCC(vertNamesArray);

    // clean up
    free(hashTableArray);
    return EXIT_SUCCESS;
}