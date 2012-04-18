#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raptor2.h>

#include <time.h>

#include "rdf_graph.h"
#include "rdf_parser.h"
#include "rdf_similar.h"
#include "rdf_lps.h"

int compare(const void* left, const void* right);  // for qsort()

int main(int argc, char **argv)
{
	double inicio;
	double final;
	double iniciolps;
	double finallps;
	int i;

	if(argc != 2)
	{
		printf("usage: %s dataset.rdf\n", argv[0]);
		exit(1);
	}

	// parser file y poblo la bd.
	rdf_database DATABASE = rdf_database_new();
	rdf_database_read_file(DATABASE, argv[1]);

	// creo los terms globales y de cada grafo y obtengo los indices
	//inicio = clock();
	lista terms = database_get_terms(DATABASE);
	lista_proc(terms, DATABASE);
	//final = clock();

	// grafo a buscar de prueba
	rdf_graph Gprueba = rdf_graph_new();
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://dbpedia.org/ontology/deathPlace", "http://dbpedia.org/resource/Chalcis");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://dbpedia.org/ontology/birthPlace", "http://dbpedia.org/resource/Stageira");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://purl.org/dc/elements/1.1/description", "Greek philosopher");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://www.w3.org/1999/02/22-rdf-syntax-ns#type", "http://xmlns.com/foaf/0.1/Person");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://xmlns.com/foaf/0.1/name", "Aristotle");

	iniciolps = clock();
	buscar(DATABASE, Gprueba);
	finallps = clock();

	inicio = clock();
	query_result resultado = database_query_graph(DATABASE, Gprueba, terms);
	qsort(resultado, DATABASE->n, sizeof *resultado, compare);
	final = clock();

	//for(i = 0; i < DATABASE->n; i++)
	//	printf("Grafo(%d) = %3.5f\n", resultado[i].index, resultado[i].idf);

	printf("Tiempo secuencial Similar: %3.5f\n", (double)(final-inicio)/CLOCKS_PER_SEC);
	printf("Tiempo secuencial LPS: %3.5f\n", (double)(finallps-iniciolps)/CLOCKS_PER_SEC);
	printf("%d grafos\n", DATABASE->n);
	printf("%d nodos\n", rdf_database_count_nodes(DATABASE));

	return 0;
}

// compare
int compare(const void* left, const void* right)
{
	query_result lt = (query_result) left;
	query_result rt = (query_result) right;

	double diff = lt->idf - rt->idf;
	
	if ( diff < 0 ) return +1;
	if ( diff > 0 ) return -1;
	return 0;
}