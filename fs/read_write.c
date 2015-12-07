#include "testfs.h"
#include "list.h"
#include "super.h"
#include "block.h"
#include "inode.h"

/* given logical block number, read the corresponding physical block into block.
 * return physical block number.
 * returns 0 if physical block does not exist.
 * returns negative value on other errors. */
static int
testfs_read_block(struct inode *in, int log_block_nr, char *block)
{
	int phy_block_nr = 0;

	assert(log_block_nr >= 0);
	if (log_block_nr < NR_DIRECT_BLOCKS) {
		phy_block_nr = (int)in->in.i_block_nr[log_block_nr];
	} else {
		log_block_nr -= NR_DIRECT_BLOCKS;
	
		if (log_block_nr >= NR_INDIRECT_BLOCKS){
			/****************************************/
			log_block_nr-= NR_INDIRECT_BLOCKS;
			int quotient=log_block_nr/NR_INDIRECT_BLOCKS;
			int remain=log_block_nr%NR_INDIRECT_BLOCKS;
			if (in->in.i_dindirect > 0){
				/* start at block number start and read nr blocks */
		/*void read_blocks(struct super_block *sb, char *blocks, off_t start, size_t nr)*/
				read_blocks(in->sb, block, in->in.i_dindirect, 1);
				int second = ((int *)block)[quotient];
				if(second==0)
					return 0;
				read_blocks(in->sb, block, second, 1);
				phy_block_nr = ((int *)block)[remain];
			}

		}
			
		if (in->in.i_indirect > 0) {
			read_blocks(in->sb, block, in->in.i_indirect, 1);
			phy_block_nr = ((int *)block)[log_block_nr];
		}
	}
	if (phy_block_nr > 0) {
		read_blocks(in->sb, block, phy_block_nr, 1);
	} else {
		/* we support sparse files by zeroing out a block that is not
		 * allocated on disk. */
		bzero(block, BLOCK_SIZE);
	}
	return phy_block_nr;
}

int
testfs_read_data(struct inode *in, char *buf, off_t start, size_t size)
{
	char block[BLOCK_SIZE];
	long block_nr = start / BLOCK_SIZE;
	long block_ix = start % BLOCK_SIZE;
	int ret;

	assert(buf);
	if (start + (off_t) size > in->in.i_size) {
		size = in->in.i_size - start;
	}
	if (block_ix + size > BLOCK_SIZE) {
	/**************************************************/
		if ((ret = testfs_read_block(in, block_nr, block)) < 0)
			return ret;
		memcpy(buf, block + block_ix, BLOCK_SIZE-block_ix);
		unsigned int current=BLOCK_SIZE-block_ix;
		block_nr++;
		while(1){
			if(current==size)	return size;
			if(size-current>=BLOCK_SIZE){
				if ((ret = testfs_read_block(in, block_nr, block)) < 0)
					return  EFBIG;
				memcpy(buf+current, block, BLOCK_SIZE);
				block_nr++;
				current+=BLOCK_SIZE;
			}
			else{
				if ((ret = testfs_read_block(in, block_nr, block)) < 0)
					return ret;
				memcpy(buf+current, block, size-current);
				return size;
			}
		}
	}
	if ((ret = testfs_read_block(in, block_nr, block)) < 0)
		return ret;
	memcpy(buf, block + block_ix, size);
	/* return the number of bytes read or any error */
	return size;
}

/* given logical block number, allocate a new physical block, if it does not
 * exist already, and return the physical block number that is allocated.
 * returns negative value on error. */
static int
testfs_allocate_block(struct inode *in, int log_block_nr, char *block)
{
	int phy_block_nr;
	char indirect[BLOCK_SIZE];
	int indirect_allocated = 0;

	assert(log_block_nr >= 0);
	phy_block_nr = testfs_read_block(in, log_block_nr, block);

	/* phy_block_nr > 0: block exists, so we don't need to allocate it, 
	   phy_block_nr < 0: some error */
	if (phy_block_nr != 0)
		return phy_block_nr;

	/* allocate a direct block */
	if (log_block_nr < NR_DIRECT_BLOCKS) {
		assert(in->in.i_block_nr[log_block_nr] == 0);
		phy_block_nr = testfs_alloc_block_for_inode(in);
		if (phy_block_nr >= 0) {
			in->in.i_block_nr[log_block_nr] = phy_block_nr;
		}
		return phy_block_nr;
	}

	log_block_nr -= NR_DIRECT_BLOCKS;
	if (log_block_nr >= NR_INDIRECT_BLOCKS){
	/*************************************************************/
		log_block_nr-=NR_INDIRECT_BLOCKS;
		char dindirect[BLOCK_SIZE];
		char indir[BLOCK_SIZE];
		int dindirect_allocated = 0;
		int indir_allocated=0;
		if(in->in.i_dindirect == 0){
			bzero(dindirect, BLOCK_SIZE);
			phy_block_nr = testfs_alloc_block_for_inode(in);
			if (phy_block_nr < 0)
				return phy_block_nr;
			dindirect_allocated = 1;
			in->in.i_dindirect = phy_block_nr;
		}else {	/* read indirect block */
			read_blocks(in->sb, dindirect, in->in.i_dindirect, 1);
		}

		int index=log_block_nr/(BLOCK_SIZE/4);
		int offset=log_block_nr%(BLOCK_SIZE/4);
		if(((int *)dindirect)[index]==0){
			bzero(indir, BLOCK_SIZE);
			phy_block_nr = testfs_alloc_block_for_inode(in);
			if (phy_block_nr < 0){
				if(dindirect_allocated==1)
					testfs_free_block_from_inode(in, in->in.i_dindirect);
				return phy_block_nr;
			}
			((int*)dindirect)[index]=phy_block_nr;
			//indir_allocated=1;
			write_blocks(in->sb, dindirect, in->in.i_dindirect, 1);
		}
			read_blocks(in->sb, indir, ((int *)dindirect)[index], 1);
			assert(((int *)indir)[offset]==0);
			phy_block_nr = testfs_alloc_block_for_inode(in);
			if (phy_block_nr < 0){
				if(indir_allocated==1)
					testfs_free_block_from_inode(in, ((int *)dindirect)[index]);
				return phy_block_nr;
			}
			((int *)indir)[offset]=phy_block_nr;
			write_blocks(in->sb,indir, ((int *)dindirect)[index], 1);
			write_blocks(in->sb,dindirect, in->in.i_dindirect, 1);
			return phy_block_nr;

	}

	if (in->in.i_indirect == 0) {	/* allocate an indirect block */
		bzero(indirect, BLOCK_SIZE);
		phy_block_nr = testfs_alloc_block_for_inode(in);
		if (phy_block_nr < 0)
			return phy_block_nr;
		indirect_allocated = 1;
		in->in.i_indirect = phy_block_nr;
	} else {	/* read indirect block */
		read_blocks(in->sb, indirect, in->in.i_indirect, 1);
	}

	/* allocate direct block */
	assert(((int *)indirect)[log_block_nr] == 0);	
	phy_block_nr = testfs_alloc_block_for_inode(in);

	if (phy_block_nr >= 0) {
		/* update indirect block */
		((int *)indirect)[log_block_nr] = phy_block_nr;
		write_blocks(in->sb, indirect, in->in.i_indirect, 1);
	} else if (indirect_allocated) {
		/* free the indirect block that was allocated */
		testfs_free_block_from_inode(in, in->in.i_indirect);
	}
	return phy_block_nr;
}

int
testfs_write_data(struct inode *in, const char *buf, off_t start, size_t size)
{
	char block[BLOCK_SIZE];
	long block_nr = start / BLOCK_SIZE;
	long block_ix = start % BLOCK_SIZE;
	int ret;

	if (block_ix + size > BLOCK_SIZE) {
		/************************************************/
		unsigned int current;
		ret = testfs_allocate_block(in, block_nr, block);
		if (ret < 0)
			return ret;
		memcpy(block + block_ix, buf, BLOCK_SIZE-block_ix);
		write_blocks(in->sb, block, ret, 1);
		current=BLOCK_SIZE-block_ix;
		block_nr++;
		while(1){
			if(current==size)	break;
			if(size-current>=BLOCK_SIZE){
				ret = testfs_allocate_block(in, block_nr, block);
				if (ret < 0)
					return  EFBIG;
				memcpy(block,buf+current, BLOCK_SIZE);
				write_blocks(in->sb, block, ret, 1);
				current+=BLOCK_SIZE;
				block_nr++;
			}
			else{
				ret = testfs_allocate_block(in, block_nr, block);
				if (ret < 0)
					return  EFBIG;
				memcpy(block, buf+current, size-current);
				write_blocks(in->sb, block, ret, 1);
				current=size;
			}
		}
		if (size > 0)
		in->in.i_size = MAX(in->in.i_size, start + (off_t) size);
		in->i_flags |= I_FLAGS_DIRTY;
		/* return the number of bytes written or any error */
		return size;
	}

	/* ret is the newly allocated physical block number */
	ret = testfs_allocate_block(in, block_nr, block);
	if (ret < 0)
		return ret;
	memcpy(block + block_ix, buf, size);
	write_blocks(in->sb, block, ret, 1);
	/* increment i_size by the number of bytes written. */
	if (size > 0)
		in->in.i_size = MAX(in->in.i_size, start + (off_t) size);
	in->i_flags |= I_FLAGS_DIRTY;
	/* return the number of bytes written or any error */
	return size;
}

int
testfs_free_blocks(struct inode *in)
{
	int i;
	int e_block_nr;

	/* last block number */
	e_block_nr = DIVROUNDUP(in->in.i_size, BLOCK_SIZE);

	/* remove direct blocks */
	for (i = 0; i < e_block_nr && i < NR_DIRECT_BLOCKS; i++) {
		if (in->in.i_block_nr[i] == 0)
			continue;
		testfs_free_block_from_inode(in, in->in.i_block_nr[i]);
		in->in.i_block_nr[i] = 0;
	}
	e_block_nr -= NR_DIRECT_BLOCKS;

	/* remove indirect blocks */
	if (in->in.i_indirect > 0) {
		char block[BLOCK_SIZE];
		read_blocks(in->sb, block, in->in.i_indirect, 1);
		for (i = 0; i < e_block_nr && i < NR_INDIRECT_BLOCKS; i++) {
			testfs_free_block_from_inode(in, ((int *)block)[i]);
			((int *)block)[i] = 0;
		}
		testfs_free_block_from_inode(in, in->in.i_indirect);
		in->in.i_indirect = 0;
	}

	e_block_nr -= NR_INDIRECT_BLOCKS;
	if (e_block_nr >= 0) {
	/**********************************************************/
			/* start at block number start and read nr blocks */
		/*void read_blocks(struct super_block *sb, char *blocks, off_t start, size_t nr)*/
		char block[BLOCK_SIZE];
		int i,j,k;
		int quotient=e_block_nr /NR_INDIRECT_BLOCKS;
		int remain=e_block_nr %NR_INDIRECT_BLOCKS;
		read_blocks(in->sb, block, in->in.i_dindirect, 1);
		for(i=0;i<quotient;i++){	
			int second = ((int *)block)[i];	
			char second_block[BLOCK_SIZE];
			read_blocks(in->sb, second_block, second, 1);
			for(j=0;j<NR_INDIRECT_BLOCKS; j++){
				testfs_free_block_from_inode(in, second_block[j]);
				((int *)second_block)[j] = 0;
			}
			testfs_free_block_from_inode(in, second);
		}
		read_blocks(in->sb, block, in->in.i_dindirect, 1);
		int second = ((int *)block)[quotient];
		char second_block[BLOCK_SIZE];
		read_blocks(in->sb, second_block, second, 1);
		for(k=0;k<remain;k++){
			testfs_free_block_from_inode(in, ((int *)second_block)[k]);
			((int *)second_block)[k] = 0;
		}
		testfs_free_block_from_inode(in, ((int *)block)[quotient]);
		testfs_free_block_from_inode(in, in->in.i_dindirect);
	}

	in->in.i_size = 0;
	in->i_flags |= I_FLAGS_DIRTY;
	return 0;
}