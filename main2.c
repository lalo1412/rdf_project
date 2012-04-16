#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raptor2.h>

#include <time.h>

#include "rdf_graph.h"
#include "rdf_parser.h"
#include "rdf_lps.h"
#include "rdf_similar.h"

int main(int argc, char **argv)
{
	long inicio;
	long final;

	if(argc != 2)
	{
		printf("usage: %s dataset.rdf\n", argv[0]);
		exit(1);
	}

	// parser file y poblo la bd.
	rdf_database DATABASE = rdf_database_new();
	rdf_database_read_file(DATABASE, argv[1]);

	// creo los terms globales y de cada grafo
	inicio = clock();
	lista terms = database_get_terms(DATABASE);
	lista_proc(terms, DATABASE);
	final = clock();

	// grafo a buscar de prueba
	rdf_graph Gprueba = rdf_graph_new();
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://dbpedia.org/ontology/deathPlace", "http://dbpedia.org/resource/Chalcis");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://dbpedia.org/ontology/birthPlace", "http://dbpedia.org/resource/Stageira");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://purl.org/dc/elements/1.1/description", "Greek philosopher");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://www.w3.org/1999/02/22-rdf-syntax-ns#type", "http://xmlns.com/foaf/0.1/Person");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://xmlns.com/foaf/0.1/name", "Aristotle");

	//inicio = clock();
	//buscar(DATABASE, Gprueba);
	//final = clock();

	printf("Tiempo secuencial: %3.5f\n", (double)(final-inicio)/CLOCKS_PER_SEC);
	printf("%d grafos\n", DATABASE->n);
	printf("%d nodos\n", rdf_database_count_nodes(DATABASE));

	return 0;
}