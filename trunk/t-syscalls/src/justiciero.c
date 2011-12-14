#include <unistd.h>             // fork(), exec
#include <sys/ptrace.h>         // ptrace
#include <sys/user.h>           // struct user_regs_struct
#include <sys/syscall.h>        // __NR_<syscall>
#include <sys/wait.h>           // wait
#include <stdio.h>              // printf

#define CHECK_ERROR(var, msg) \
	if (var == -1) { \
		perror(msg); \
		goto error; \
	}


int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage %s <cmd> [param]\n", argv[0]);
		goto error;
	}

	pid_t child = fork();
	CHECK_ERROR(child, "fork");

	if (child == 0) {
		long err = ptrace(PTRACE_TRACEME, NULL, NULL, NULL);
		CHECK_ERROR(err, "PTRACE_TRACEME");

		int err = execvp(argv[1], argv + 1);
		CHECK_ERROR(err, "exec");
	}

	// XXX: Si child no es ni 0 ni -1, entonces soy el padre. Notar que como
	// se llama a exec el hijo nunca llega a aca
	// Esperamos a que nuestro hijo termine o llame a kill y lo matamos
	for (;;) {
		int status;
		pid_t r = wait(&status);
		CHECK_ERROR(r, "wait");

		// Me fijo si termino mi hijo
		if (WIFEXITED(status))
			break;

		// Si no murio, me fijo a que syscall llamo mirando
		// orig_eax (x86)
		struct user_regs_struct regs;
		long err = ptrace(PTRACE_GETREGS, child, NULL, &regs);
		CHECK_ERROR(err, "PTRACE_GETREGS");

		if (regs.orig_eax == __NR_kill) {
			printf("Se ha hecho justicia!\n");
			ptrace(PTRACE_KILL, child, NULL, NULL);
			break;
		}

		// Si no era kill, que siga ejecutando nomas
		err = ptrace(PTRACE_SYSCALL, child, NULL, NULL);
		CHECK_ERROR(err, "PTRACE_SYSCALL");
	}


	return 0;

error:
	return -1;
}
