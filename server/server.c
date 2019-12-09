#include "common.h"
#include <sys/stat.h>
#include <time.h>

void res_free(void ** p) { free(*p); }
void str_free(char ** p) { res_free((void**)p); }
void file_close(int * fd) { close(*fd); }

int main(int argc, char * argv[])
{
    struct sembuf sem_ready[2] = {{0, 0, 0}, {1, 1, 0}};	//init sems
    struct sembuf sem_recv[2] = {{0, -1, 0}, {1, 0, 0}}; // init sems

	key_t key;

	int perms;
	int semd;
    int shmemd;
	char * shmem_addr;
    struct shmid_ds shmem_op;

    char * client_message ON_SCOPE_EXIT(str_free);
    int message_size;
    char * filename;
    int filename_len;

	struct stat st;
	
	int fd ON_SCOPE_EXIT(file_close) = creat(TOKEN_FILENAME, O_RDWR);

	key = ftok(TOKEN_FILENAME, TOKEN_ID);

	perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    semd = semget(key, 2, IPC_CREAT | perms); 

	shmemd = shmget(key, 1024, IPC_CREAT | perms);

	semop(semd, sem_ready, 2);

	semop(semd, sem_recv, 2);	//blocked until client sends message

	shmem_addr = shmat(shmemd, 0, SHM_RND | SHM_RDONLY);

	shmctl(shmemd, IPC_STAT, &shmem_op);

	message_size = shmem_op.shm_segsz;

	client_message = calloc(message_size, sizeof(uint8_t));

	strcpy(client_message, shmem_addr);

	printf("Last detached process PID: %d\n", shmem_op.shm_lpid);
	printf("Time of files creation:\n");

    filename = client_message;
	filename = strtok(filename, "\n");

    do
    {
        filename_len = strlen(filename);
        stat(filename, &st);

		printf("File's %s creation time: %s", filename, asctime(gmtime(&(st.st_ctim.tv_sec))));
    } while ((filename = strtok(filename + filename_len + 1, "\n")) != NULL);

	shmdt(shmem_addr);
	shmctl(shmemd, IPC_RMID, 0);
	semctl(semd, 0, IPC_RMID, 0);

	remove(TOKEN_FILENAME);

    return 0;
}