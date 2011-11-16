#include "srv.h"
#include <stdlib.h>	// abort()
#include <assert.h>	// assert()

static int me;
static int n;
static int our_seq_nr = 0;
static int max_seq_nr = 0;


void send_broadcast_req()
{
	for (int i = 0; i < n/2; i++) {
		int srv = i * 2;
		
		if (srv == me)
			continue;
		// XXX: esta bien usar el MPI_Send (teniendo en cuenta que puede bloquear hasta que el otro lado haga Recv) ?
		MPI_Send(&our_seq_nr, 1, MPI_INT, srv, SRV_REQ, COMM_WORLD);
	}
	
}

void send_broadcast_finished()
{
	for (int i = 0; i < n/2; i++) {
		int srv = i * 2;
		
		if (srv == me)
			continue;
		// XXX: esta bien usar el MPI_Send (teniendo en cuenta que puede bloquear hasta que el otro lado haga Recv) ?
		MPI_Send(NULL, 0, MPI_INT, srv, SRV_FINISHED, COMM_WORLD);
	}
}


void servidor(int mi_cliente)
{
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &me);
	MPI_Comm_size(MPI_COMM_WORLD, &n);
	int listo_para_salir = FALSE;

	// Se corresponde con la variable "requesting_critical_section" del paper
	int hay_pedido_local = FALSE;

	// La cantidad de clientes que todavia estan corriendo
	int clients_alive = n/2;

	// La cantidad de servidores que falta que respondan un SRV_REQ
	int outstanding_reply;
	
	/* Array para guardar los reply que "postergamos" a los servidores
	 * Como hay n servidores y 2n procesos, la cantidad de servidores es la cantidad de procesos sobre 2
	 */
	int *reply_deferred = malloc(sizeof(int) * (n/2));
	assert(reply_deferred != NULL);

	for (int i = 0; i < n/2; i++)
		reply_deferred[i] = FALSE;

	while (!listo_para_salir) {
		
		int buf;
		MPI_Recv(&buf, 1, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
		int origen = status.MPI_SOURCE;
		int tag = status.MPI_TAG;

		switch (tag) {
		case TAG_PEDIDO:
			assert(origen == mi_cliente);
			debug("Mi cliente solicita acceso exclusivo");
			assert(hay_pedido_local == FALSE);
			hay_pedido_local = TRUE;

			// Falta recibir respuesta de todos los servidores menos yo
			outstanding_reply = (n/2) - 1;
			
			/* Pedir tag a los demas servidores */
			debug("Mandando SRV_REQ BROADCAST");
			our_seq_nr = max_seq_nr + 1;
			send_broadcast_req();
			
			break;
		
		case TAG_LIBERO:
			assert(origen == mi_cliente);
			debug("Mi cliente libera su acceso exclusivo");
			assert(hay_pedido_local == TRUE);
			hay_pedido_local = FALSE;

			/* Procesar los reply_deferred */
			for (int i = 0; i < n/2; i++) {
				int srv = i * 2;
				if (reply_deferred[i] == TRUE) {
					MPI_Send(NULL, 0, MPI_INT, srv, SRV_REPLY, COMM_WORLD);
					reply_deferred[i] = FALSE;
				}
			}
			
			break;
			
		case SRV_REQ:
			debug("llego SRV_REQ");
			int k = buf;
			max_seq_nr = (k > max_seq_nr) ? k: max_seq_nr;

			if (hay_pedido_local && (k > our_seq_nr || (k == our_seq_nr && origen > me))) {
				int srv_idx = origen/2;
				assert(srv_idx < n/2);
				reply_deferred[srv_idx] = TRUE;
				break;
			}
			
			MPI_Send(NULL, 0, MPI_INT, origen, SRV_REPLY, COMM_WORLD);
			
			break;
			
		case SRV_REPLY:
			debug("llego SRV_REPLY");
			outstanding_reply -= 1;
			
			// Me llegaron todas las respuestas de los servidores
			if (outstanding_reply == 0) {
				debug("Dándole permiso");
				MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
			}
		
			break;

		case TAG_TERMINE:
			assert(origen == mi_cliente);
			debug("Mi cliente avisa que terminó");

			// Le aviso al resto de los servidores que hay un
			// cliente menos, asi actualizan la variable de
			// cantidad de clientes vivos
			send_broadcast_finished();

			// NO hago el break. Sigo ejecutando el codigo de
			// SRV_FINISHED que es identico a lo que deberia hacer
			// aca

		case SRV_FINISHED:

			// Termino el cliente de algun servidor. Asique queda
			// un cliente menos. Pero si ese cliente era el ultimo,
			// estoy listo para salir.
			clients_alive--;
			if (clients_alive == 0)
				listo_para_salir = TRUE;
			break;

		default:
			debug("Tag desconocido!");
			abort();
		}
	}

	free(reply_deferred);
}
