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

void servidor(int mi_cliente)
{
	MPI_Status status;
	int origen, tag;
	MPI_Comm_rank(MPI_COMM_WORLD, &me);
	MPI_Comm_size(MPI_COMM_WORLD, &n);
	int outstanding_reply = (n/2) - 1;
	int hay_pedido_local = FALSE;
	int listo_para_salir = FALSE;
	
	/* Array para guardar los reply que "postergamos" a los servidores
	 * Como hay n servidores y 2n procesos, la cantidad de servidores es la cantidad de procesos sobre 2
	 */
	int *reply_deferred = malloc(sizeof(int) * (n/2));
	assert(reply_deferred != NULL);
	for (int i = 0; i < n/2; i++)
		reply_deferred[i] = FALSE;

	while (! listo_para_salir) {
		
		int buf;
		MPI_Recv(&buf, 1, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
		origen = status.MPI_SOURCE;
		tag = status.MPI_TAG;

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
			
			//debug("Dándole permiso");
			//MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
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
			// No salimos del while porque sino:
			// los otros servidores esperan nuestra respuesta, pero no va a llegar nunca. Porque si salimos del while, morimos
			// TODO: manejar el caso de que termino
			//listo_para_salir = TRUE;
			
			break;
			
		case SRV_REQ:
			debug("llego SRV_REQ");
			//printf("buf es %d\n", buf);
			int k = buf;
			max_seq_num = (k > max_seq_num) ? k: max_seq_num;

			printf("max_seq es %d\n", max_seq_num);
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
			
			if (outstanding_reply == 0) {
				debug("Dándole permiso");
				MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
			} else {
				printf("faltan %d replies", outstanding_reply);
			}
		
			break;

		default:
			debug("Tag desconocido!");
			assert(0);
		}
	}
	printf("servidor finalizado\n");
	free(reply_deferred);
	
}
