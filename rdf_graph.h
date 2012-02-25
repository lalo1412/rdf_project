#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilacion: gcc -o rdfprint rdf_database.c `pkg-config raptor2 --cflags --libs`
// BNBrdfdc01.ntriples  contiene 6.773.422 triples.

// valor del nodo
typedef struct {
    char *string;
    int string_len;
} rdf_label;

// nodos
typedef struct node {
    unsigned int arity;
    rdf_label value;
} *rdf_node;

// edges subcontunjo rdf_node x rdf_node
typedef struct edge {
    rdf_node subject;
    rdf_node object;
    rdf_label predicate;
} *rdf_edge;

// edge set
typedef struct edge_set {
    rdf_edge value;
    struct edge_set *next;
} *rdf_edge_set;

// node set
typedef struct node_set {
    rdf_node value;
    struct node_set *next;
} *rdf_node_set;

// graphs
typedef struct graph{
    rdf_node_set V;
    rdf_edge_set E;
    unsigned int u;
} *rdf_graph;

/*****
*** GLOBAL VARIABLE
***/
rdf_graph CURRENT_GRAPH;

/*****
*** Funciones
*****/
rdf_graph rdf_graph_new();
int rdf_graph_isempty(rdf_graph G);
void rdf_graph_print(rdf_graph G);

rdf_node rdf_node_new();
void rdf_node_set_label(rdf_node v, char *label);

rdf_edge rdf_edge_new(rdf_node v1, rdf_node v2);
void rdf_edge_set_label(rdf_edge e, char *label);

rdf_node_set rdf_node_set_new();
void rdf_node_set_add(rdf_node_set V, char *label);
int rdf_node_set_exist(rdf_node_set V, char *label);

rdf_edge_set rdf_edge_set_new();
void rdf_edge_set_add(rdf_edge_set E, rdf_edge e);
rdf_node rdf_edge_set_get_pair(rdf_edge_set E, rdf_node sub);