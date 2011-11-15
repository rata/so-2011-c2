#include "srv.h"

static int me;
static int n;
static int our_seq_num = 0;
static int max_seq_num = 0;


void send_broadcast_req()
{
	for (int i = 0; i < n/2; i++) {
		int srv = i * 2;
		
		if (srv == me)
			continue;
		// XXX: esta bien usar el MPI_Send (teniendo en cuenta que puede bloquear hasta que el otro lado haga Recv) ?
		MPI_Send(&our_seq_num, 1, MPI_INT, srv, SRV_REQ, COMM_WORLD);
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
	int outstanding_reply = (n/2) - 1;
	int hay_pedido_local = FALSE;
	int listo_para_salir = FALSE;
	int clients_alive = n/2;
	
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
			outstanding_reply = (n/2) - 1;
			
			/* Pedir tag a los demas servidores */
			debug("Mandando SRV_REQ BROADCAST");
			our_seq_num = max_seq_num + 1;
			send_broadcast_req();
			
			break;
		
		case TAG_LIBERO:
			assert(origen == mi_cliente);
			debug("Mi cliente libera su acceso exclusivo");
			assert(hay_pedido_local == TRUE);
			hay_pedido_local = FALSE;
			outstanding_reply = (n/2) - 1;

			/* Procesar los reply_deferred */
			for (int i = 0; i < n/2; i++) {
				int srv = i * 2;
				if (reply_deferred[i] == TRUE) {
					MPI_Send(NULL, 0, MPI_INT, srv, SRV_REPLY, COMM_WORLD);
					reply_deferred[i] = FALSE;
				}
			}
			
			break;
			
		case TAG_TERMINE:
			assert(origen == mi_cliente);
			debug("Mi cliente avisa que terminó");

			// Si mi cliente termino, hay un cliente menos
			clients_alive--;

			// Le aviso al resto de los servidores que hay un
			// cliente menos, asi actualizan la variable de
			// cantidad de clientes vivos
			send_broadcast_finished();

			// Si mi cliente era el ultimo, estoy listo para salir
			if (clients_alive == 0)
				listo_para_salir = TRUE;
		
			break;
			
		case SRV_REQ:
			debug("llego SRV_REQ");
			int k = buf;
			max_seq_num = (k > max_seq_num) ? k: max_seq_num;

			if (hay_pedido_local && (k > our_seq_num || (k == our_seq_num && origen > me))) {
				int srv = origen/2;
				assert(srv < n/2);
				reply_deferred[srv] = TRUE;
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

		case SRV_FINISHED:

			// Termino el cliente de algun servidor (no el mio). Asique queda un cliente menos
			// PEro si ese cliente era el ultimo, estoy listo para salir.
			clients_alive--;
			if (clients_alive == 0)
				listo_para_salir = TRUE;


		default:
			debug("Tag desconocido!");
			assert(0);
		}
	}

	free(reply_deferred);
}
