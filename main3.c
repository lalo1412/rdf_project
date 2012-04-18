#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raptor2.h>

#include <time.h>

#include "rdf_graph.h"
#include "rdf_parser.h"
#include "rdf_lps.h"

#define  INIT  1        // Message dando el tamano
#define  DATA  2        // mensaje que comunica datos
#define  ANSW  3        // mensaje retornando el vector y ordenado

int compare(const void* left, const void* right);  // for qsort()

int parallel(int size, int loc, rdf_database db, rdf_graph G);

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("usage: %s dataset.rdf\n", argv[0]);
		exit(1);
	}

	int myRank, nProc;
	int rc;
   	int size;          // tamano de la bd a leer
   	int loc;			// locacion en la bd
   	double start;
   	double finish;

	rc = MPI_Init(&argc, &argv);

	if ( rc < 0 )
	{
		puts ("Fallo MPI.  Abort!");
		exit(-1);
	}

	rc = MPI_Comm_rank (MPI_COMM_WORLD, &myRank);
	rc = MPI_Comm_size (MPI_COMM_WORLD, &nProc);

	// grafo a buscar de prueba
	rdf_graph Gprueba = rdf_graph_new();
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://dbpedia.org/ontology/deathPlace", "http://dbpedia.org/resource/Chalcis");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://dbpedia.org/ontology/birthPlace", "http://dbpedia.org/resource/Stageira");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://purl.org/dc/elements/1.1/description", "Greek philosopher");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://www.w3.org/1999/02/22-rdf-syntax-ns#type", "http://xmlns.com/foaf/0.1/Person");
	rdf_graph_add_triple(Gprueba, "http://dbpedia.org/resource/Aristotle", "http://xmlns.com/foaf/0.1/name", "Aristotle");

	// levanto la BD
	rdf_database DATABASE;
	DATABASE = rdf_database_new();
	rdf_database_read_file(DATABASE, argv[1]);

	// proceso la base de datos y obtengo los términos
	//lista terms = database_get_terms(DATABASE);
	//lista_proc(terms, DATABASE);

	// todos poseen la BD lista
	MPI_Barrier(MPI_COMM_WORLD);


	// *****
	// *****
	// EMPIEZA EL PARALELO!
	// *****
	// *****
	if(myRank == 0) // proceso principal Head0
	{
		int encuentro; // si encuentra el grafo o no
		int i;

		start = MPI_Wtime();
		encuentro = parallel(DATABASE->n, 0, DATABASE, Gprueba);
		finish = MPI_Wtime();

		//for(i = 0; i < DATABASE->n; i++)
		//	printf("Grafo(%d) = %3.5f\n", vector[i].index, vector[i].idf);

		printf("Tiempo paralelo LPS: %3.5f\n", (finish-start));
		printf("Encontre el grafo?: %d\n", encuentro);
		printf("%d grafos\n", DATABASE->n);
		printf("%d nodos\n", rdf_database_count_nodes(DATABASE));
	}
	else
	{
		int parent = (myRank+1) / 2;
		MPI_Status status;

		rc = MPI_Recv(&size, 1, MPI_INT, MPI_ANY_SOURCE, INIT, MPI_COMM_WORLD, &status);
		rc = MPI_Recv(&loc, 1, MPI_INT, MPI_ANY_SOURCE, INIT, MPI_COMM_WORLD, &status);

		parallel(size, loc, DATABASE, Gprueba);

		MPI_Finalize();
		return 0;
	}

	// solo myran == 0 ejecuta aqui
	// tiempo
	//printf("tiempo paralelo: %3.5f\n", time_total);
	//printf("%d nodos\n", rdf_database_count_nodes(DATABASE));

	MPI_Abort(MPI_COMM_WORLD, 0);
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

int parallel(int size, int loc, rdf_database db, rdf_graph G)
{
	int parent;
	int myRank, nProc;
	int rc, ltChild, rtChild;
	int flag;
	int rtEncuentra;
	int ltEncuentra;

	rc = MPI_Comm_rank (MPI_COMM_WORLD, &myRank);
	rc = MPI_Comm_size (MPI_COMM_WORLD, &nProc);

	parent = (myRank-1)/2;
	ltChild = (myRank << 1) + 1;
	rtChild = ltChild + 1;

	if(ltChild < nProc)
	{
		int left_size  = size / 2;
		int right_size = size - left_size;

		int left_loc = loc;
		int right_loc = loc + left_size;

		MPI_Status status;               // estado de MPI

		rc = MPI_Send(&left_size, 1, MPI_INT, ltChild, INIT, MPI_COMM_WORLD);
		rc = MPI_Send(&left_loc, 1, MPI_INT, ltChild, INIT, MPI_COMM_WORLD);
		//printf("yo %d enviando dato a ltchild %d size: %d y loc: %d\n", myRank, ltChild, left_size, left_loc); fflush(stdout);

		if(rtChild < nProc)
		{
			rc = MPI_Send(&right_size, 1, MPI_INT, rtChild, INIT, MPI_COMM_WORLD);
			rc = MPI_Send(&right_loc, 1, MPI_INT, rtChild, INIT, MPI_COMM_WORLD);
			//printf("yo %d enviando dato a ltchild %d size: %d y loc: %d\n", myRank, rtChild, right_size, right_loc); fflush(stdout);

			rc = MPI_Recv(&rtEncuentra, 1, MPI_INT, rtChild, ANSW, MPI_COMM_WORLD, &status );
			//printf("yo %d recibo dato de rtchild %d\n", myRank, rtChild); fflush(stdout);
		}
		else
		{
			// CONSULTANDO EN LPS
			flag = buscarn(db, G, right_size, right_loc);
			printf("Yo %d encontre el grafo: %d\n", myRank, flag);
		}

		rc = MPI_Recv(&ltEncuentra, 1, MPI_INT, ltChild, ANSW, MPI_COMM_WORLD, &status );
		//printf("yo %d recibo dato de ltchild %d\n", myRank, ltChild); fflush(stdout);

		// Merge the two results back into vector

		if(ltEncuentra == 1 || rtEncuentra == 1)
			flag = 1;

		if(myRank == 0)
			return flag;
	} 
	else
	{
		// CONSULTANDO EN LPS
		flag = buscarn(db, G, size, loc);
		printf("Yo %d encontre el grafo: %d\n", myRank, flag);
	}
/**
 * Note:  if not the root node, send the result to the parent.
 **/
 	if ( myRank != 0 )
	{
		MPI_Send(&flag, 1, MPI_INT, parent, ANSW, MPI_COMM_WORLD );
		printf("Yo %d envio mi respuesta: %d\n", myRank, flag);
	}
}