#include "common.h"
#include <limits.h>
#include <sys/sysctl.h>
#include <time.h>

void res_free(void ** p) { free(*p); }
void str_free(char ** p) { res_free((void **)p); }
void proc_file_close(FILE ** fp) { pclose(*fp); }

int main(int argc, char * argv[])
{
    struct sembuf sem_block[2] = {{0, 0, 0}, {1, -1, 0}};  // init sems
    struct sembuf sem_send[2] = {{0, 1, 0}, {1, 0, 0}}; // init sems

    key_t key;

    char * cmd_out ON_SCOPE_EXIT(str_free) = NULL;
    size_t out_len = 0;

    char filepath[PATH_MAX];
    size_t cwd_len;

    char * message = NULL;
    size_t mesg_size = 0;

	int semd;
    int shmemd;
    char * shmem_addr;
    struct shmid_ds shmem_op;

	FILE * cmdp ON_SCOPE_EXIT(proc_file_close);

    while (access(TOKEN_FILENAME, F_OK) == -1)
    {
        sleep(1);
    }

    key = ftok(TOKEN_FILENAME, TOKEN_ID);
    
    semd = semget(key, 2, 0);

	semop(semd, sem_block, 2);	//wait for server

    shmemd = shmget(key, 1024, 0);

    cmdp = popen("for filename in ./*; do file $filename; done | grep ELF | awk "
              "-F ':' '{print substr($1,3)}'",
              "r");

    getcwd(filepath, sizeof(filepath));

	strcat(filepath, "/");
	cwd_len = strlen(filepath);

    while (getline(&cmd_out, &out_len, cmdp) != -1)
    {
        strcat(filepath, cmd_out);

        message = realloc(message, mesg_size + strlen(filepath));
        message[mesg_size] = 0;
        mesg_size += strlen(filepath);

		strcat(message, filepath);

		filepath[cwd_len] = 0;
    }

	shmctl(shmemd, IPC_STAT, &shmem_op);

	if (shmem_op.shm_segsz < strlen(message) + 1)
	{
            shmem_op.shm_segsz = strlen(message) + 1;
            shmctl(shmemd, IPC_SET, &shmem_op);
	}

	shmem_addr = shmat(shmemd, 0, SHM_RND);

	strcpy(shmem_addr, message);

	semop(semd, sem_send, 2); // all done

	shmdt(shmem_addr);

    return 0;
}