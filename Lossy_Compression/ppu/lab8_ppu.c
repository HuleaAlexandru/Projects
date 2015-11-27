#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <libspe2.h>
#include <pthread.h>
#include <sys/time.h>
#include "../cmp.h"

extern spe_program_handle_t lab8_spu;

#define MAX_SPU_THREADS   	16

void *ppu_pthread_function(void *thread_arg) {

	spe_context_ptr_t ctx;
	struct package_t *arg = (struct package_t *) thread_arg;

	/* Create SPE context */
	if ((ctx = spe_context_create (0, NULL)) == NULL) {
		perror ("Failed creating context");
		exit (1);
	}

	/* Load SPE program into context */
	if (spe_program_load (ctx, &lab8_spu)) {
		perror ("Failed loading program");
		exit (1);
	}

	/* Run SPE context */
	unsigned int entry = SPE_DEFAULT_ENTRY;

	/* transferul adresei structurii initiale */
	if (spe_context_run(ctx, &entry, 0, (void *)arg, (void *)sizeof(struct package_t), NULL) < 0) {  
		perror ("Failed running context");
		exit (1);
	}  

	/* Destroy context */
	if (spe_context_destroy (ctx) != 0) {
		perror("Failed destroying context");
		exit (1);
	}

	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    
	if (argc != 8) {
		printf("Usage: ./tema3 mod_vect mod_dma num_spus in.pgm out.cmp out.pgm results.txt");
		return -1;
    }
	int mod_vect = atoi(argv[1]);
	int mod_dma = atoi(argv[2]);
	int num_spus = atoi(argv[3]);
	char *inpgm = argv[4];
	char *outcmp = argv[5];
	char *outpgm = argv[6];
	char *results = argv[7];
	int i;

	struct img initial_image, decompressed_image;
	struct c_img compressed_image;

	struct timeval start_total, end_total, start_op, end_op;
	double total_time = 0, op_time = 0;

	gettimeofday(&start_total, NULL);

	// citeste imaginea initiala 
	read_pgm(inpgm, &initial_image);

	gettimeofday(&start_op, NULL);
	
	compressed_image.width = initial_image.width;
	compressed_image.height = initial_image.height;
	int nr_cmp_blocks = (1LL * initial_image.width * initial_image.height) / (BLOCK_SIZE * BLOCK_SIZE);
	compressed_image.blocks = (struct block *)malloc_align(nr_cmp_blocks * sizeof(struct block), 7);

	pthread_t *compress_threads = (pthread_t*)malloc_align(num_spus * sizeof(pthread_t), 7);
	struct package_t *cthread_arg = (struct package_t *)malloc_align(num_spus * sizeof(struct package_t), 7);
	
	int nr_of_blocks = (initial_image.width * initial_image.height) / (BLOCK_SIZE * BLOCK_SIZE);
	int average_blocks = nr_of_blocks / num_spus;
	int rest_blocks = nr_of_blocks % num_spus;
	int offset = 0;

	for(i = 0; i < num_spus; i++) { 

		/* completeaza structura package_t de trimis la spu pentru fiecare spu*/	
		cthread_arg[i].action_type = 0;
		cthread_arg[i].mod_vect = mod_vect;
		cthread_arg[i].mod_dma = mod_dma;
		cthread_arg[i].num_spus = num_spus;
		cthread_arg[i].nr_blocks = average_blocks;
		cthread_arg[i].index_block = offset;
			
		cthread_arg[i].img_pgm.width = initial_image.width;
		cthread_arg[i].img_pgm.height = initial_image.height;
		cthread_arg[i].img_pgm.pixels = initial_image.pixels + ((offset / (initial_image.width / BLOCK_SIZE)) * BLOCK_SIZE * initial_image.width + (offset % (initial_image.width / BLOCK_SIZE)) * BLOCK_SIZE);
				
		cthread_arg[i].img_cmp.width = compressed_image.width;
		cthread_arg[i].img_cmp.height = compressed_image.height;
		cthread_arg[i].img_cmp.blocks = compressed_image.blocks + ((offset / (initial_image.width / BLOCK_SIZE)) * (initial_image.width / BLOCK_SIZE) + (offset % (initial_image.width / BLOCK_SIZE)));

		offset += average_blocks;
		nr_of_blocks -= average_blocks;
		if (rest_blocks != 0 && i != num_spus - 1) {
			average_blocks = nr_of_blocks / (num_spus - 1 - i);
			rest_blocks = nr_of_blocks % (num_spus - 1 - i);
		}

		/* Create thread for each SPE context */
		if (pthread_create (&compress_threads[i], NULL, &ppu_pthread_function, &cthread_arg[i]))  {
			perror ("Failed creating thread");
			exit (1);
		}
	}

	/* Wait for SPU-thread to complete execution.  */
  	for (i = 0; i < num_spus; i++) {
		if (pthread_join (compress_threads[i], NULL)) {
			perror("Failed pthread_join");
			exit (1);
		}
	}

  	free_align(compress_threads);
	free_align(cthread_arg);
 
	decompressed_image.width = initial_image.width;
	decompressed_image.height = initial_image.height;
	int nr_dec_blocks = (1LL * initial_image.width * initial_image.height) / (BLOCK_SIZE * BLOCK_SIZE);
	decompressed_image.pixels = (unsigned char *)malloc_align(initial_image.height * initial_image.width * sizeof(unsigned char), 7);

	pthread_t *decompress_threads = (pthread_t*)malloc_align(num_spus * sizeof(pthread_t), 7);
	struct package_t *dthread_arg = (struct package_t *)malloc_align(num_spus * sizeof(struct package_t), 7);
	
	int dec_average_blocks = nr_dec_blocks / num_spus;
	int dec_rest_blocks = nr_dec_blocks % num_spus;
	int dec_offset = 0;

	for(i = 0; i < num_spus; i++) { 

		/* completeaza structura package_t de trimis la spu pentru fiecare spu*/	
		dthread_arg[i].action_type = 1;
		dthread_arg[i].mod_vect = mod_vect;
		dthread_arg[i].mod_dma = mod_dma;
		dthread_arg[i].num_spus = num_spus;
		dthread_arg[i].nr_blocks = dec_average_blocks;
		dthread_arg[i].index_block = dec_offset;
			
		dthread_arg[i].img_pgm.width = initial_image.width;
		dthread_arg[i].img_pgm.height = initial_image.height;
		dthread_arg[i].img_pgm.pixels = decompressed_image.pixels + ((dec_offset / (initial_image.width / BLOCK_SIZE)) * BLOCK_SIZE * initial_image.width + (dec_offset % (initial_image.width / BLOCK_SIZE)) * BLOCK_SIZE);
				
		dthread_arg[i].img_cmp.width = compressed_image.width;
		dthread_arg[i].img_cmp.height = compressed_image.height;
		dthread_arg[i].img_cmp.blocks = compressed_image.blocks + ((dec_offset / (initial_image.width / BLOCK_SIZE)) * (initial_image.width / BLOCK_SIZE) + (dec_offset % (initial_image.width / BLOCK_SIZE)));

		dec_offset += dec_average_blocks;
		nr_dec_blocks -= dec_average_blocks;
		if (dec_rest_blocks != 0 && i != num_spus - 1) {
			dec_average_blocks = nr_dec_blocks / (num_spus - 1 - i);
			dec_rest_blocks = nr_dec_blocks % (num_spus - 1 - i);
		}

		/* Create thread for each SPE context */
		if (pthread_create (&decompress_threads[i], NULL, &ppu_pthread_function, &dthread_arg[i]))  {
			perror ("Failed creating thread");
			exit (1);
		}
	}

	/* Wait for SPU-thread to complete execution.  */
  	for (i = 0; i < num_spus; i++) {
		if (pthread_join (decompress_threads[i], NULL)) {
			perror("Failed pthread_join");
			exit (1);
		}
	}
	gettimeofday(&end_op, NULL);

	write_cmp(outcmp, &compressed_image);
	write_pgm(outpgm, &decompressed_image);
	
	free_align(compressed_image.blocks);
	free_align(decompressed_image.pixels);
	free_align(decompress_threads);
	free_align(dthread_arg);

	gettimeofday(&end_total, NULL);
	
	total_time += GET_TIME_DELTA(start_total, end_total);
	op_time += GET_TIME_DELTA(start_op, end_op);

	freopen(results, "a+", stdout);
	printf("%i %lf %lf\n", num_spus, op_time, total_time);
	fclose(stdout);

	return 0;
}
