#include <stdio.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include "../cmp.h"

#define wait_tag(t) mfc_write_tag_mask(1<<t); mfc_read_tag_status_all();

// cast de la array de char la array de float prin copiere element cu element si cast
void charToFloatScalar(unsigned char *src, float *dst, unsigned int size) {
	unsigned int i;
	for (i = 0; i < size; i++) {
		dst[i] = (float)src[i];
	}
}

// cast de la array de float la array de char prin copiere element cu element si cast
void floatToCharScalar(float *src, unsigned char *dst, unsigned int size) {
	unsigned int i;
	for (i = 0; i < size; i++) {
		dst[i] = (unsigned char)src[i];
	}
}

// cast de la vector de char la vector de float prin operatii intrinsec
// realizez o trecere intermediara prin vector de int: char->int->float
void charToFloatVector(vector unsigned char *src, vector float *dst) {
	unsigned int j;
	vector unsigned int zero = spu_splats((unsigned int)0);
	vector unsigned char pattern = (vector unsigned char) {16, 16, 16, 0, 16, 16, 16, 1, 16, 16, 16, 2, 16, 16, 16, 3}; 
	vector unsigned int *clone_int = (vector unsigned int *)malloc_align(BUF_SIZE / (16 / sizeof(unsigned int)) * sizeof(vector unsigned int), 7);

	for (j = 0; j < BUF_SIZE / (16 / sizeof(float)); j++) {
		vector unsigned char four = spu_splats((unsigned char)4);
		vector unsigned char fourj = spu_splats((unsigned char)(j % 4));
		vector unsigned char mul = four * fourj;
		vector unsigned char actual_pattern = pattern + mul;
		clone_int[j] = spu_shuffle((vector unsigned int)src[j / 4], zero, actual_pattern);
		dst[j] = spu_convtf(clone_int[j], 0);
	}
	free_align(clone_int);
}

// cast de la vector de float la vector de char prin operatii intrinsec
// realizez o trecere intermediara prin vector de int: float->int->char
void floatToCharVector(vector float *src, vector unsigned char *dst) {
		unsigned int j;
		vector unsigned int *vaux_int =  (vector unsigned int *)malloc_align(BUF_SIZE / (16 / sizeof(unsigned int)) * sizeof(vector unsigned int), 7);
		vector unsigned int zero = spu_splats((unsigned int)0);
		for (j = 0; j < BUF_SIZE / (16 / (sizeof(unsigned char))) ; j++) {
			// vectorii de int 0 1 2 3 formeaza vectorul de char 0
			vector unsigned char pattern1 = (vector unsigned char) {3, 7, 11, 15, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31}; 
			vector unsigned char pattern2 = (vector unsigned char) {16, 17, 18, 19, 3, 7, 11, 15, 31, 31, 31, 31, 31, 31, 31, 31};
			vector unsigned char pattern3 = (vector unsigned char) {16, 17, 18, 19, 20, 21, 22, 23, 3, 7, 11, 15, 31, 31, 31, 31}; 
			vector unsigned char pattern4 = (vector unsigned char) {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 3, 7, 11, 15};
			unsigned int mult = sizeof(float) / sizeof(unsigned char);
			vaux_int[j * mult] = spu_convtu(src[j * mult], 0);
			vaux_int[j * mult + 1] = spu_convtu(src[j * mult + 1], 0);
			vaux_int[j * mult + 2] = spu_convtu(src[j * mult + 2], 0);
			vaux_int[j * mult + 3] = spu_convtu(src[j * mult + 3], 0);

			dst[j] = (vector unsigned char)spu_shuffle(vaux_int[j * mult], zero, pattern1);
			dst[j] = (vector unsigned char)spu_shuffle(vaux_int[j * mult + 1], (vector unsigned int)dst[j], pattern2);
			dst[j] = (vector unsigned char)spu_shuffle(vaux_int[j * mult + 2], (vector unsigned int)dst[j], pattern3);
			dst[j] = (vector unsigned char)spu_shuffle(vaux_int[j * mult + 3], (vector unsigned int)dst[j], pattern4);
		}
		free_align(vaux_int);
}

// operatiile de compresie realizate cu operatii vectoriale
void intrinsecCompress(vector float *vaux, float pas, unsigned char min) {
	unsigned int i;
	vector float vmin = spu_splats((float)min);	
	vector float vpas = spu_splats(pas);
	vector float rot = spu_splats(0.5f);

	for(i = 0; i < BUF_SIZE / (16 / sizeof(float)); i++) {
		vaux[i] = (vaux[i] - vmin) / vpas + rot;
	}
}

// operatiile de decompresie realizate cu operatii vectoriale
void intrinsecDecompress(vector float *vaux, float pas, unsigned char min) {
	unsigned int i;
	vector float vmin = spu_splats((float)min);	
	vector float vpas = spu_splats(pas);
	vector float rot = spu_splats(0.5f);

	for (i = 0; i < BUF_SIZE / (16 / sizeof(float)); i++) { 
		vaux[i] = vaux[i] * vpas + vmin + rot;
	}
}

// mod_vect == 0 - casturi element cu element si calcule - compresie/decompresie realizate pe array-uri
void compress0(unsigned char *A, struct block *curr_block) {
	unsigned int j;	
	float aux[BUF_SIZE] __attribute__ ((aligned(16)));
	unsigned char min = 255;
	unsigned char max = 0;
	for(j = 0; j < BLOCK_SIZE * BLOCK_SIZE; j++) {
		if (A[j] < min)
			min = A[j];
		if (A[j] > max)
			max = A[j];	
	}
	charToFloatScalar(A, aux, BUF_SIZE);	

	curr_block->min = min;
	curr_block->max = max;
	float pas = (max - min) / (float)(NUM_COLORS_PALETTE - 1);
	if (pas != 0) {
		for(j = 0; j < BUF_SIZE; j++) {
			aux[j] = (aux[j] - min) / pas + 0.5;
		}
		floatToCharScalar(aux, curr_block->index_matrix, BUF_SIZE);
	}
	else {
		memset(curr_block->index_matrix, 0, BUF_SIZE * sizeof(unsigned char));
	}
}

void decompress0(unsigned char *A, struct block *curr_block) {
	unsigned int j;
	float aux[BUF_SIZE] __attribute__ ((aligned(16)));
	unsigned char min = curr_block->min;
	unsigned char max = curr_block->max;
	float pas = (max - min) / (float)(NUM_COLORS_PALETTE - 1);
	
	charToFloatScalar(curr_block->index_matrix, aux, BUF_SIZE);
	for(j = 0; j < BUF_SIZE; j++) {
		aux[j] = aux[j] * pas + min + 0.5;
	}
	floatToCharScalar(aux, A, BUF_SIZE);
}

// mod_vect == 1 - casturi element cu element si calcule - compresie/decompresie realizate cu operatii vectoriale
void compress1(unsigned char *A, struct block *curr_block) {
	unsigned int j;	
	float aux[BUF_SIZE] __attribute__ ((aligned(16)));
	unsigned char min = 255;
	unsigned char max = 0;
	for(j = 0; j < BUF_SIZE; j++) {
		if (A[j] < min)
			min = A[j];
		if (A[j] > max)
			max = A[j];	
	}
	charToFloatScalar(A, aux, BUF_SIZE);

	curr_block->min = min;
	curr_block->max = max;
	float pas =  (max - min) / (float)(NUM_COLORS_PALETTE - 1);
	
	if (pas != 0) {
		vector float *vaux = (vector float *) aux;
		intrinsecCompress(vaux, pas, min);
		floatToCharScalar(aux, curr_block->index_matrix, BUF_SIZE);	
	}
	else {
		vector unsigned char *vblock = (vector unsigned char *)curr_block->index_matrix;
		for(j = 0; j < BUF_SIZE / ( 16 / sizeof(unsigned char)); j++) {
			vblock[j] = spu_splats((unsigned char)0);	
		}
	}
}

void decompress1(unsigned char *A, struct block *curr_block) {
	float aux[BUF_SIZE] __attribute__ ((aligned(16)));
	unsigned char min = curr_block->min;
	unsigned char max = curr_block->max;
	float pas = (max - min) / (float)(NUM_COLORS_PALETTE - 1);
	
	charToFloatScalar(curr_block->index_matrix, aux, BUF_SIZE);	
	vector float *vaux = (vector float *) aux;
	intrinsecDecompress(vaux, pas, min);
	floatToCharScalar(aux, A, BUF_SIZE);
}

// mod_vect == 2 - casturi cu operatii intrinsec si calcule - compresie/decompresie realizate cu operatii vectoriale
void compress2(unsigned char *A, struct block *curr_block) {
	unsigned int j;	
	unsigned char min = 255;
	unsigned char max = 0;
	
	for(j = 0; j < BLOCK_SIZE * BLOCK_SIZE; j++) {
		if (A[j] < min)
			min = A[j];
		if (A[j] > max)
			max = A[j];	
	}	
	curr_block->min = min;
	curr_block->max = max;
	float pas =  (max - min) / (float)(NUM_COLORS_PALETTE - 1);
	
	vector unsigned char *cloneA = (vector unsigned char *)A;
	vector float *vaux = (vector float *)malloc_align(BUF_SIZE / (16 / sizeof(float))* sizeof(vector float), 7);
	charToFloatVector(cloneA, vaux);

	if (pas != 0) {
		intrinsecCompress(vaux, pas, min);		
		vector unsigned char *last =  (vector unsigned char *)(curr_block->index_matrix);
		floatToCharVector(vaux, last);		
	}
	else {
		vector unsigned char *aux = (vector unsigned char *)curr_block->index_matrix;
		for(j = 0; j < BUF_SIZE / ( 16 / sizeof(unsigned char)); j++) {
			aux[j] = spu_splats((unsigned char)0);	
		}
	}
	free_align(vaux);
}

void decompress2(unsigned char *A, struct block *curr_block) {
	unsigned char min = curr_block->min;
	unsigned char max = curr_block->max;
	float pas = (max - min) / (float)(NUM_COLORS_PALETTE - 1);
	
	vector unsigned char *clone = (vector unsigned char *)(curr_block->index_matrix);
	vector float *vaux = (vector float *)malloc_align(BUF_SIZE / (16 / sizeof(float))* sizeof(vector float), 7);
	
	charToFloatVector(clone, vaux);
	intrinsecDecompress(vaux, pas, min);
	vector unsigned char *last =  (vector unsigned char *)(A);
	floatToCharVector(vaux, last);
	free_align(vaux);
}

int main(unsigned long long speid, unsigned long long argp, unsigned long long envp)
{
	unsigned int i, j;		
	struct package_t p __attribute__ ((aligned(16)));

	uint32_t tag_id = mfc_tag_reserve();
	if (tag_id==MFC_TAG_INVALID){
		printf("SPU: ERROR can't allocate tag ID\n"); 
		return -1;
	}

	/* transferul DMA al structurii */ 
	mfc_get((void *)&p, argp, (int)envp, tag_id, 0, 0);
	wait_tag(tag_id);

	if (p.mod_dma == 0) {	
		if (p.action_type == 0) {
			int offset = 0;
			for (i = 0; i < p.nr_blocks; i++) {
				struct block curr_block;	
				unsigned char A[BUF_SIZE] __attribute__ ((aligned(16)));

				for (j = 0; j < BLOCK_SIZE; j++) {
					mfc_get((void*)(A + j * BLOCK_SIZE), (uint32_t)(p.img_pgm.pixels) + (j * p.img_pgm.width + offset) * sizeof(unsigned char), BLOCK_SIZE * sizeof(unsigned char), tag_id, 0, 0);
					wait_tag(tag_id);
				}
			
				/* compresia blocului */
				if (p.mod_vect == 0) {
					compress0(A, &curr_block);
				}				
				else if (p.mod_vect == 1) {
					compress1(A, &curr_block);
				}			
				else {
					compress2(A, &curr_block);	
				}

				/* scriere in main storage */
				mfc_put((void *)&curr_block, (uint32_t) (p.img_cmp.blocks) + i * sizeof(struct block), sizeof(struct block), tag_id, 0, 0);
				wait_tag(tag_id);

				int column_blocks = p.img_pgm.width / BLOCK_SIZE;
				int actual_column = p.index_block % column_blocks;
				if (actual_column + 1 == column_blocks) {
					offset += (BLOCK_SIZE + (BLOCK_SIZE - 1) * p.img_pgm.width); 
				}
				else {
					offset += BLOCK_SIZE;	
				}
				p.index_block++;
			}
		}
		else {
			int offset = 0;
			for (i = 0; i < p.nr_blocks; i++) {
				struct block curr_block;	
				unsigned char A[BUF_SIZE] __attribute__ ((aligned(16)));

				mfc_get((void*)(&curr_block), (uint32_t)(p.img_cmp.blocks) + i * sizeof(struct block), sizeof(struct block), tag_id, 0, 0);
				wait_tag(tag_id);
			
				/* decompresia blocului */
				if (p.mod_vect == 0) {
					decompress0(A, &curr_block);	
				}
				else if (p.mod_vect == 1) {
					decompress1(A, &curr_block);	
				}
				else {
					decompress2(A, &curr_block);
				}
	
				/* scriere in main storage */
				for (j = 0; j < BLOCK_SIZE; j++) {
					mfc_put((void*)(A + j * BLOCK_SIZE), (uint32_t)(p.img_pgm.pixels) + (j * p.img_pgm.width + offset) * sizeof(unsigned char), BLOCK_SIZE * sizeof(unsigned char), tag_id, 0, 0);
					wait_tag(tag_id);
				}
	
				int column_blocks = p.img_pgm.width / BLOCK_SIZE;
				int actual_column = p.index_block % column_blocks;
				if (actual_column + 1 == column_blocks) {
					offset += (BLOCK_SIZE + (BLOCK_SIZE - 1) * p.img_pgm.width); 
				}
				else {
					offset += BLOCK_SIZE;	
				}
				p.index_block++;
			}
		}
	}
	else {
		// double buffering
		uint32_t tag_id_buf = mfc_tag_reserve();
		if (tag_id_buf==MFC_TAG_INVALID){
			printf("SPU: ERROR can't allocate tag ID\n"); 
			return -1;
		}
		uint32_t tag_id_vec = mfc_tag_reserve();
		if (tag_id_vec==MFC_TAG_INVALID){
			printf("SPU: ERROR can't allocate tag ID\n"); 
			return -1;
		}


		if (p.action_type == 0) {
			int offset = 0;
			int turn = 0;
			struct block curr_block;	
			unsigned char A[BUF_SIZE] __attribute__ ((aligned(16)));
			unsigned char A_buf[BUF_SIZE] __attribute__ ((aligned(16)));

			// iau primul set de date 
			for (j = 0; j < BLOCK_SIZE; j++) {
				mfc_get((void*)(A + j * BLOCK_SIZE), (uint32_t)(p.img_pgm.pixels) + (j * p.img_pgm.width + offset) * sizeof(unsigned char), BLOCK_SIZE * sizeof(unsigned char), tag_id_buf, 0, 0);
				if (j != BLOCK_SIZE - 1) {
					wait_tag(tag_id_buf);
				}
			}
	
			int column_blocks = p.img_pgm.width / BLOCK_SIZE;
			int actual_column = p.index_block % column_blocks;
			if (actual_column + 1 == column_blocks) {
				offset += (BLOCK_SIZE + (BLOCK_SIZE - 1) * p.img_pgm.width); 
			}
			else {
				offset += BLOCK_SIZE;	
			}
			p.index_block++;

			for (i = 0; i <= p.nr_blocks; i++) {
				if (turn == 0) {
					for (j = 0; j < BLOCK_SIZE; j++) {
						mfc_get((void*)(A_buf + j * BLOCK_SIZE), (uint32_t)(p.img_pgm.pixels) + (j * p.img_pgm.width + offset) * sizeof(unsigned char), BLOCK_SIZE * sizeof(unsigned char), tag_id_vec, 0, 0);
						if (j != BLOCK_SIZE - 1) {
							wait_tag(tag_id_vec);
						}
					}
					wait_tag(tag_id_buf);
			
					/* compresia blocului */
					if (p.mod_vect == 0) {
						compress0(A, &curr_block);
					}				
					else if (p.mod_vect == 1) {
						compress1(A, &curr_block);
					}			
					else {
						compress2(A, &curr_block);	
					}
				}
				else {
					for (j = 0; j < BLOCK_SIZE; j++) {
						mfc_get((void*)(A + j * BLOCK_SIZE), (uint32_t)(p.img_pgm.pixels) + (j * p.img_pgm.width + offset) * sizeof(unsigned char), BLOCK_SIZE * sizeof(unsigned char), tag_id_buf, 0, 0);
						if (j != BLOCK_SIZE - 1) {
							wait_tag(tag_id_buf);
						}
					}	
					wait_tag(tag_id_vec);
				
					/* compresia blocului */
					if (p.mod_vect == 0) {
						compress0(A_buf, &curr_block);
					}				
					else if (p.mod_vect == 1) {
						compress1(A_buf, &curr_block);
					}			
					else {
						compress2(A_buf, &curr_block);	
					}
				}
				/* scriere in main storage */
				mfc_put((void *)&curr_block, (uint32_t) (p.img_cmp.blocks) + i * sizeof(struct block), sizeof(struct block), tag_id, 0, 0);
				wait_tag(tag_id);

				turn ^= 1;

				column_blocks = p.img_pgm.width / BLOCK_SIZE;
				actual_column = p.index_block % column_blocks;
				if (actual_column + 1 == column_blocks) {
					offset += (BLOCK_SIZE + (BLOCK_SIZE - 1) * p.img_pgm.width); 
				}
				else {
					offset += BLOCK_SIZE;	
				}
				p.index_block++;
			}
			
			if (turn == 0) {	
				wait_tag(tag_id_buf);
				
				/* compresia blocului */
				if (p.mod_vect == 0) {
					compress0(A, &curr_block);
				}				
				else if (p.mod_vect == 1) {
					compress1(A, &curr_block);
				}			
				else {
					compress2(A, &curr_block);	
				}
			}
			else {
				wait_tag(tag_id_vec);	

				/* compresia blocului */
				if (p.mod_vect == 0) {
					compress0(A_buf, &curr_block);
				}				
				else if (p.mod_vect == 1) {
					compress1(A_buf, &curr_block);
				}			
				else {
					compress2(A_buf, &curr_block);	
				}
			}

			/* scriere in main storage */
			mfc_put((void *)&curr_block, (uint32_t) (p.img_cmp.blocks) + i * sizeof(struct block), sizeof(struct block), tag_id, 0, 0);
			wait_tag(tag_id);

		}
		else {
			int offset = 0, offset_buf = 0;
			int turn = 0;
			struct block curr_block;
			struct block curr_block_buf;	
			unsigned char A[BUF_SIZE] __attribute__ ((aligned(16)));
			i = 0;

			mfc_get((void*)(&curr_block), (uint32_t)(p.img_cmp.blocks) + i * sizeof(struct block), sizeof(struct block), tag_id_buf, 0, 0);
			i++;

			offset_buf = offset;
			int column_blocks = p.img_pgm.width / BLOCK_SIZE;
			int actual_column = p.index_block % column_blocks;
			if (actual_column + 1 == column_blocks) {
				offset += (BLOCK_SIZE + (BLOCK_SIZE - 1) * p.img_pgm.width); 
			}
			else {
				offset += BLOCK_SIZE;	
			}
			p.index_block++;

			for (; i < p.nr_blocks; i++) {
				if (turn == 0) {
					mfc_get((void*)(&curr_block_buf), (uint32_t)(p.img_cmp.blocks) + i * sizeof(struct block), sizeof(struct block), tag_id_vec, 0, 0);
					wait_tag(tag_id_buf);
			
					/* decompresia blocului */
					if (p.mod_vect == 0) {
						decompress0(A, &curr_block);	
					}
					else if (p.mod_vect == 1) {
						decompress1(A, &curr_block);	
					}
					else {
						decompress2(A, &curr_block);
					}
				}
				else {
					mfc_get((void*)(&curr_block), (uint32_t)(p.img_cmp.blocks) + i * sizeof(struct block), sizeof(struct block), tag_id_buf, 0, 0);
					wait_tag(tag_id_vec);
			
					/* decompresia blocului */
					if (p.mod_vect == 0) {
						decompress0(A, &curr_block_buf);	
					}
					else if (p.mod_vect == 1) {
						decompress1(A, &curr_block_buf);	
					}
					else {
						decompress2(A, &curr_block_buf);
					}
				}
	
				/* scriere in main storage */
				for (j = 0; j < BLOCK_SIZE; j++) {
					mfc_put((void*)(A + j * BLOCK_SIZE), (uint32_t)(p.img_pgm.pixels) + (j * p.img_pgm.width + offset_buf) * sizeof(unsigned char), BLOCK_SIZE * sizeof(unsigned char), tag_id, 0, 0);
					wait_tag(tag_id);
				}
	
				turn ^= 1;
				offset_buf = offset;
				column_blocks = p.img_pgm.width / BLOCK_SIZE;
				actual_column = p.index_block % column_blocks;
				if (actual_column + 1 == column_blocks) {
					offset += (BLOCK_SIZE + (BLOCK_SIZE - 1) * p.img_pgm.width); 
				}
				else {
					offset += BLOCK_SIZE;	
				}
				p.index_block++;
			}
		
			if (turn == 0) {
				wait_tag(tag_id_buf);
				/* decompresia blocului */
				if (p.mod_vect == 0) {
					decompress0(A, &curr_block);	
				}
				else if (p.mod_vect == 1) {
					decompress1(A, &curr_block);	
				}
				else {
					decompress2(A, &curr_block);
				}
			}
			else {
				wait_tag(tag_id_vec);
				/* decompresia blocului */
				if (p.mod_vect == 0) {
					decompress0(A, &curr_block_buf);	
				}
				else if (p.mod_vect == 1) {
					decompress1(A, &curr_block_buf);	
				}
				else {
					decompress2(A, &curr_block_buf);
				}
			}

			/* scriere in main storage */
			for (j = 0; j < BLOCK_SIZE; j++) {
				mfc_put((void*)(A + j * BLOCK_SIZE), (uint32_t)(p.img_pgm.pixels) + (j * p.img_pgm.width + offset_buf) * sizeof(unsigned char), BLOCK_SIZE * sizeof(unsigned char), tag_id, 0, 0);
				wait_tag(tag_id);
			}
		}
		
		/* eliberam tag id-urile suplimentare pentru double buffering */
		mfc_tag_release(tag_id_buf);
		mfc_tag_release(tag_id_vec);
	
	}

	/* eliberam tag id-ul */
	mfc_tag_release(tag_id);
	
	return 0;
}

