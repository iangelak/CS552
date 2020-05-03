#include "include/mem.h"
#include "include/file_ops.h"
#include "helper.h"
#include "vga.h"



int check_pathname(char *pathname, char *filename);
int exist_inodes(void);
int update_parent(int parent_inode,char *filename, int action, uint32_t type, uint32_t mode);
int allocate_block(void);
int allocate_inode(uint32_t type, uint32_t permissions);
int check_if_exists(char *name,int par_inode);
void show_bitmap(void);
int find_block(int block_num, block_t* cur_block, int inode);
int exist_blocks(void);


/*TODO*/
// Need one function to recurse through the pathnames
// Need one function to recurse through the inodes
// Need one strcmp function to compare string names


/*Initialize the current global fdt pointer*/
//Don't run anywhere else except when the table is
//first created

void init_fdt(void){
	for (int i =0; i<= MAX_FILES; i++){
		glob_fdt_ptr[i].in_use = FREE;
	}
}




// Create a file or a directory
// TODO: Check if the path name exists
// 	 Check if there are available inodes
// 	 Check if there is at least one available block
int rd_creat(char *pathname, mode_t mode){
	
	char buf[16];

	int res = 0;
	char filename[14];
	int parent_inode;
	//Check if we are trying to create the root directory
	if (strcmp(pathname,root->filename) == 0){
		print_s("creat: Root directory already exists!!!");
		return ERROR;
	}
	
	//Check if there are available inodes for our file
	
	res = exist_inodes();
	
	if (res < 0){
		print_s("creat: There are no available inodes...Try later, sorry\n");
		return ERROR;
	}
	
	parent_inode = check_pathname(pathname,filename);
	
	print_s("Filename is ");
	print_s(filename);
	
	itoa(buf,'d',parent_inode);
	print_s("\nParent inode is ");
	print_s(buf);
	print_s("\n");

	if (parent_inode < 0){
		print_s("creat: Invalid path...Aborting\n");
		return ERROR;
	}

	//Ok now we have the correct parent...Allocate resources for the file
	//Also update the parent entry
	
	print_s("Before updating the parent with the new entry\n");
	res = update_parent(parent_inode,filename, CR, FL, mode);
	if (res < 0){
		print_s("creat: Cannot create a new entry for this file...Aborting\n");
		return ERROR;
	}

	print_s("Parent updated\n");

}


int rd_mkdir(char *pathname){
	int res = 0;

	char filename[14];
	int parent_inode;
	char buf[16];
	//Check if we are trying to create the root directory
	if (strcmp(pathname,root->filename) == 0){
		print_s("mkdir: Root directory already exists!!!");
		return ERROR;
	}

	//Check if there are available inodes for our file
	
	res = exist_inodes();
	
	if (res < 0){
		print_s("mkdir: There are no available inodes...Try later, sorry\n");
		return ERROR;
	}
	
	parent_inode = check_pathname(pathname,filename);
	
	itoa(buf,'d',parent_inode);
	print_s("Parent inode is ");
	print_s(buf);
	print_s("\n");

	if (parent_inode < 0){
		print_s("mkdir: Invalid path...Aborting\n");
		return ERROR;
	}

	//Ok now we have the correct parent...Allocate resources for the file
	//Also update the parent entry
	
	print_s("Before updating the parent with the new entry\n");
	res = update_parent(parent_inode,filename, CR, DR, RW);
	if (res < 0){
		print_s("mkdir: Cannot create a new entry for this directory...Aborting\n");
		return ERROR;
	}

	print_s("Parent updated\n");


}

int rd_open(char *pathname, int flags){
	
	int res = 0;

	char filename[14];
	int parent_inode, file_inode;
	char buf[16];

	if (strcmp(pathname,root->filename) == 0){
		print_s("open: Root directory is always open\n");
		return ERROR;
	}
	
	parent_inode = check_pathname(pathname,filename);
	

	
	itoa(buf,'d',parent_inode);
	print_s("\nParent inode is ");
	print_s(buf);
	print_s("\n");

	if (parent_inode < 0){
		print_s("open: Invalid path...Aborting\n");
		return ERROR;
	}

	//Ok now we have the correct parent...Find the inode number of the file
	//Check if the file is a directory or not
	
	print_s("Before updating the parent with the new entry\n");
	
	file_inode = check_if_exists(filename,parent_inode);
	
	if (file_inode < 0 && fs->inode[file_inode].type == DR){
		print_s("open: The file you requested does not exist\n");
		return ERROR;
	}
	
	/* Check the permissions*/

	// The permissions to open the file are not correct
	if (flags > fs->inode[file_inode].perm){
		print_s("open: Incorrect permissions\n");
		return ERROR;
	}
	if(flags == RO && fs->inode[file_inode].perm == WR){
		print_s("open: The file write only\n");
		return ERROR;
	}
	if (flags == RW && fs->inode[file_inode].perm != RW){
		print_s("open: The file cannot be opened as read write\n");
		return ERROR;
	}
	
	/*Assign a valid descriptor to the file*/
	for (int i = 0; i <= MAX_FILES; i++){
		if (glob_fdt_ptr[i].in_use == FREE){
			glob_fdt_ptr[i].in_use = USED;
			glob_fdt_ptr[i].flags = flags;
			glob_fdt_ptr[i].pos_ptr = 0;
			glob_fdt_ptr[i].inode = file_inode;
			
			// Return the index to the fdt
			return i;
		}
	}
	
	print_s("Non available file descriptors for the file\n");
	
	return ERROR;
}

int rd_close(int fd){
	
	if (glob_fdt_ptr[fd].in_use == FREE){
		print_s("The file your are trying to close does not exist\n");
		return ERROR;
	}

	glob_fdt_ptr[fd].in_use = FREE;
	glob_fdt_ptr[fd].flags = JUNK;
	glob_fdt_ptr[fd].pos_ptr = 0;
	glob_fdt_ptr[fd].inode = JUNK;

	return 0;
}

int rd_read(int fd, char *address, int num_bytes){
	if (glob_fdt_ptr[fd].in_use == FREE){
		print_s("The file your are trying to read from does not exist\n");
		return ERROR;
	}
	
	/*Case of a directory*/
	/*if (fs->inode[inode_num].type == DR){*/
		/*print_s("You cannot read from a directory file\n");*/
		/*return ERROR;*/
	/*}*/


}


int rd_write(int fd, char *address, int num_bytes){
	
	int res;
	int num_of_blocks; 				//num of blocks to write
	int extra_bytes; 				//Bytes of the last block
	int inode_num; 					//Inode number for the file
	uint32_t pos_ptr; 				//Position to start in the file
	int flags; 					//File permissions
	int cur_block; 					//Current block to start writing from
	int last_blk_ofst; 				//Offset in the cur block
	block_t *block_ptr;

	/*FD does not exist*/
	if (glob_fdt_ptr[fd].in_use == FREE){
		print_s("write: The file you are trying to write to does not exist\n");
		return ERROR;
	}
	
	/*Read only file*/
	flags = glob_fdt_ptr[fd].flags;

	if (flags == RO ){
		print_s("write: The file is opened as readonly\n");
		return ERROR;
	}
	
	/*Case of a directory*/
	if (fs->inode[inode_num].type == DR){
		print_s("You cannot write to a directory file\n");
		return ERROR;
	}

	inode_num = glob_fdt_ptr[fd].inode;
	pos_ptr = glob_fdt_ptr[fd].pos_ptr;
	
	cur_block = pos_ptr / 256; 		//Number of blocks already written
	last_blk_ofst = pos_ptr % 256; 		//Offset in the last block
	
	
	res = find_block(cur_block,block_ptr,inode_num); //Find the current block;
	
	if (res < 0){
		print_s("write: Cannot find a location pointer for this block\n");
		return ERROR;
	}


	/*Current byte in the block that we are able to write to*/
	char *cur_byte = (char*) (block_ptr) + last_blk_ofst;
	
	int bytes_written = 0;
	while (bytes_written < num_bytes && last_blk_ofst < 256){
		*cur_byte = address[bytes_written];
		cur_byte++;
		bytes_written++;
		last_blk_ofst++;
	}
	

	//We've written everything in the last block
	if (bytes_written == num_bytes)
		return bytes_written;
	//More bytes left to write to the file
	else{
		last_blk_ofst = 0;
		cur_block++;
	}
	
	num_bytes -= bytes_written;

	num_of_blocks = num_bytes / 256;
	extra_bytes  = num_bytes % 256;
	
	/*First fill the last block*/
	
	/*Start writing from the current block*/
	/*We are going to be writing at byte granularity*/

	for (int i = cur_block; i < cur_block + num_of_blocks; i++){
		//Check if there is an available block
		if(!exist_blocks())
			return ERROR;
		
		// Find a free block and set the correct location pointer
		int block_num = allocate_block();

		res = find_block(cur_block,block_ptr,inode_num);
		if (res < 0){
			print_s("write: Cannot find a location pointer for this block\n");
			return ERROR;
		}
		
		block_ptr = &fs->d_blks[block_num];
		char *byte_in_blk = (char*) block_ptr; 	 		//We need to write in byte granularity
	
		//We are in the last block...Write what is left
		if (i == num_of_blocks - 1){
			for (int j=0; j < extra_bytes; j++){
				byte_in_blk[j] = address[bytes_written];
				bytes_written++;
			}
		}
		else{
		 	for(int j=0; j< 256; j++){
				byte_in_blk[j] = address[bytes_written];
				bytes_written++;
			}
		}
	}

	return bytes_written;


}


int rd_lseek(int fd, int offset){
	
	if (glob_fdt_ptr[fd].in_use == FREE){
		print_s("The file your are trying to seek to does not exist\n");
		return ERROR;
	}


}

int rd_unlink(char *pathname){
	
	if (strcmp(pathname,root->filename) == 0){
		print_s("unlink: You cannot remove the root directory\n");
		return ERROR;
	}
}

int rd_chmod(char *pathname, mode_t mode){
	
	int res = 0;

	char filename[14];
	int parent_inode, file_inode;
	char buf[16];

	if (strcmp(pathname,root->filename) == 0){
		print_s("chmod: You cannot change the permissions of the root directory\n");
		return ERROR;
	}
	
	parent_inode = check_pathname(pathname,filename);
	
	itoa(buf,'d',parent_inode);
	print_s("Parent inode is ");
	print_s(buf);
	print_s("\n");

	if (parent_inode < 0){
		print_s("chmod: Invalid path...Aborting\n");
		return ERROR;
	}

	//Ok now we have the correct parent...Find the inode number of the file
	//Check if the file is a directory or not
	
	print_s("Before checking if the file exists\n");
	file_inode = check_if_exists(filename,parent_inode);
	if (file_inode < 0 && fs->inode[file_inode].type == DR){
		print_s("chmod: The file you requested does not exist\n");
		return ERROR;
	}
	
	/*Updating the mode*/
	fs->inode[file_inode].perm = mode;

	print_s("File mode updated\n");

}

int find_block(int block_num, block_t* cur_block, int inode){
	//We reached the blocks limit for a single inode 
	if (block_num == MAX_INODE_BLOCKS){
		print_s("We reached the max number of blocks for this inode...\n");
		return ERROR;
	}
	

	if(block_num <= 7){
		cur_block = (block_t*) fs->inode[inode].location[block_num];
		return 0;
	}
	//First indirection
	else if (block_num <= 71){
		block_t *indirect = (block_t*) fs->inode[inode].location[8];
		cur_block = (block_t*) &indirect[block_num - 8];
		return 0;
	}
	//Second indirection
	else if (block_num <= MAX_INODE_BLOCKS - 1){
		int ind_1 = (block_num - 72) / 64;
		int ind_2 = (block_num - 72) % 64;
		block_t *indirect_1 = fs->inode[inode].location[9];
		block_t *indirect_2 = &indirect_1[ind_1];
		cur_block = (block_t*) &indirect_2[ind_2];
		return 0;
	}
	

}


int next_block(int block_num, block_t *cur_block, int par_inode){
	//We reached the blocks limit for a single inode 
	if (block_num == MAX_INODE_BLOCKS){
		print_s("We reached the max number of blocks for this inode...\n");
		return ERROR;
	}

	block_num++;
	if(block_num <= 7){
		cur_block = (block_t*) fs->inode[par_inode].location[block_num];
		return block_num;
	}
	//First indirection
	else if (block_num <= 71){
		block_t *indirect = (block_t*) fs->inode[par_inode].location[8];
		cur_block = (block_t*) &indirect[block_num - 8];
		return block_num;
	}
	//Second indirection
	else if (block_num <= MAX_INODE_BLOCKS - 1){
		int ind_1 = (block_num - 72) / 64;
		int ind_2 = (block_num - 72) % 64;
		block_t *indirect_1 = fs->inode[par_inode].location[9];
		block_t *indirect_2 = &indirect_1[ind_1];
		cur_block = (block_t*) &indirect_2[ind_2];
		return block_num;
	}
}

// Check if the File/Dir with name exist under parent with inode par_inode
int check_if_exists(char name[],int par_inode){
	
	/*TODO*/
	// Maybe add a check if the parent is full or not
	// Because maybe we cannot add the new entry to parent even if there 
	// is space for it in general (inodes, blocks)

	//Case where the parent inode is a file
	if(fs->inode[par_inode].type == FL){
		print_s("The parent is a regular file...Invalid path!\n");
		return ERROR;
	}
	// The parent is empty so the file/dir definitely does not exist
	if(fs->inode[par_inode].size == 0){
		return ERROR;
	}

	//We have to search in the parent if the file exists or not
	block_t *cur_block;
	cur_block = (block_t*) fs->inode[par_inode].location[0]; 	//Start from parent's first block
	int block_num = JUNK;      					//First block (will help with indirections)

	while (block_num != -1){
		for (int i = 0; i < 16; i++){
			dir_t *entry;  					//Temporary entry struct to extract the inode num
			entry = (dir_t *) (&cur_block + i * 16);
			//The file/dir we are looking for exists in this block...Yay!
			print_s("Exists ");
			print_s(entry->filename);
			print_s("\n");
			if (strcmp(entry->filename, name) == 0){
				print_s("File found!\n");
				return (int) entry->inode_num; 			//Return the inode number
			}
		}
		block_num = next_block(block_num,cur_block,par_inode);
	}

	return ERROR;
}


int check_pathname(char *pathname, char filename[]){
	char buf[16];

	int par_inode = 0; 		// Start from the root inode
	int status = 1;
	int path_len;
	path_len = strlen(pathname);
	itoa(buf,'d',path_len);

	if (pathname[0] != '/'){
		print_s("Pathnames must start with /...Abort!\n");
		return ERROR;
	}
	
	//Only '/' in pathname
	if (path_len == 1){
		print_s("Cannot create nor delete root...Abort!\n");
		return ERROR;
	}

	print_s("After checks 1\n");
	//Get the length of the path to know if we are at the end...

	int name_finish = 1;
	int final = 0 ; 		//Flag to check if we are at the last name or not

	while (final != 1 && status != ERROR){
		/* First get every directory in the path and check if they exist */
		int type = 0; 			//type 0 = regular file  and 1 = directory
		char temp_name[14];
		int i = 0;
		while(pathname[name_finish] != '/' && pathname[name_finish] != '\0'){
			temp_name[i] = pathname[name_finish];
			name_finish++;
			i++;
		}
		
		print_s("Temp name is ");
		print_s(temp_name);
		print_s("\n");
		if (name_finish == path_len){
			print_s("Here1\n");
			final = 1;
			strncpy(filename,temp_name,14);
		}

		//This is a directory name
		if (pathname[name_finish] == '/'){
			print_s("Here2\n");
			type = 1;
			temp_name[i] = '\0'; 		//Null terminator for name
		}
		else
			type = 0; 		//No need for this type was already 0
		
		print_s("Before check if exists\n");
		//Check if the file exists and return its inode if it exists else ERROR
		status = check_if_exists(temp_name,par_inode);
		
		//Case that the name requested does not exist yay
		if (status == ERROR && final == 1){
			print_s("The path is valid and the file does not exist\n");
			return par_inode;
			
		}
		// Case where the parent is a regural file
		if (status != ERROR && final == 0){
			print_s("Traversing the path\n");
			par_inode = status;
			/*continuing*/
		}
		// This is a problem...A directory in the path does not exist
		else if (status == ERROR && final == 0){
			print_s("The path does not exist\n");
			return ERROR;
		}
		// The file/dir that we requested already exists...Return ERROR
		else if (status != ERROR && final == 1) {
			print_s("The file/directory that you requested already exists!\n");
			return ERROR;
		}


	}
	return ERROR;

	
}

int allocate_inode(uint32_t type, uint32_t permissions){
	/*Loop through the inode array to see if there are free inodes*/
	int ret = ERROR;
	int blk_num;
	for (int i = 1; i <= MAX_FILES; i++){
		if (fs->inode[i].in_use == 0){
			ret = i;
			blk_num = allocate_block();

			if (blk_num < 0){
				print_s("There is no block available...Aborting\n");
				return ERROR;
			}
			fs->inode[i].type = type;
			fs->inode[i].size = 0;
			fs->inode[i].location[0] = &fs->d_blks[blk_num];
			fs->inode[i].perm = permissions;
			fs->superblock.free_inodes--; 				//Decrease available inodes
			fs->inode[i].in_use = 1;
			return ret;
		}
	}
	return ret;
}

int allocate_block(void){
	int block_num ;
	uint8_t *bmap_ptr;
	int bit;
	//The first 32 * 8 = 256 blocks are allocated for the superblock inodes
	//It is actually the first 261 blocks, so we need to start from index 32
	//in the bitmap cause there are some unmapped blocks in that byte
	//
	//show_bitmap();

	for (int i = 32; i< BITMAP_SIZE; i++){
		for (int j =0; j< 8; j++){
			bmap_ptr = &fs->bitmap[i];
			bit = (*bmap_ptr << j) & 0x80;
			// Case of a free block in bitmap
			if (bit == 0){
				
				block_num = i * 8 + j - 261;
				*bmap_ptr |= (0x80 >> j);
				fs->superblock.block_num--; 			//Decrease available blocks
				
				return block_num;
			}
		}
	}
	return ERROR;
}

//We got the parent inode and the filename
int update_parent(int parent_inode, char* filename, int action, uint32_t type, uint32_t perm){
	//Depending on the parent's size we can find the correct location pointer
	//No space left in the parent
	if (fs->inode[parent_inode].size == MAX_FILE_SIZE){
		print_s("Parent has no space for a new entry\n");
		return ERROR;
	}
	
	//If we want to create something, the search is smoother
	if (action == CR){
		//First we find how many blocks the parent has, then based on the block
		//number we find which location pointer to use
		int block_num = fs->inode[parent_inode].size / 256; 	//Last used block number
		int offset = fs->inode[parent_inode].size % 256;  	//Offset in the last block number
		// We are in the first direct pointers Yay!!!
		
		int inode_num;
		print_s("Allocating inode\n");
		inode_num = allocate_inode(type, perm);
		
		dir_t *temp_dir; 					//Temp pointer to update smth
		if (block_num <= 7 ){
			temp_dir = (dir_t*) (&(fs->inode[parent_inode].location[block_num]) + offset);
			strncpy(temp_dir->filename,filename,14); 		// Security is everything :)
			temp_dir->inode_num = inode_num;
			fs->inode[parent_inode].size +=16;
			print_s(temp_dir->filename);
			print_s("\n");
		}
		else if (block_num <= 71){
			block_t *indirect = (block_t*) fs->inode[parent_inode].location[8];
			strncpy(temp_dir->filename,filename,14);
			temp_dir = (dir_t*) (&indirect[block_num - 8] + offset);
			temp_dir->inode_num = inode_num;
			fs->inode[parent_inode].size +=16;
		}
		//Second indirection
		else if (block_num <= MAX_INODE_BLOCKS - 1){
			int ind_1 = (block_num - 72) / 64;
			int ind_2 = (block_num - 72) % 64;
			block_t *indirect_1 = fs->inode[parent_inode].location[9];
			block_t *indirect_2 = &indirect_1[ind_1];
			temp_dir = (dir_t*) (&indirect_2[ind_2] + offset);
			strncpy(temp_dir->filename,filename,14);
			temp_dir->inode_num = inode_num;
			fs->inode[parent_inode].size +=16;

		}
	}
}



int exist_blocks(void){
	
	if (fs->superblock.block_num > 0)
		return TRUE;
	else{
		return FALSE;
	}
}

int exist_inodes(void){
	
	if (fs->superblock.free_inodes > 0)
		return TRUE;
	else{
		return FALSE;
	}
	
}

void show_inode_info(int inode){
	char buf[16];
	inode_t *inode_ptr = &fs->inode[inode];
	print_s("\nShowing information for inode\n");
	itoa(buf,'d',inode);
	print_s(buf);
	print_s("\nInode type: ");
	itoa(buf,'d',inode_ptr->type);
	print_s(buf);
	print_s("\nInode size: ");
	itoa(buf,'d',inode_ptr->size);
	print_s(buf);
	print_s("\nInode location pointers: ");
	for (int i =0; i< 10; i++){
		itoa(buf,'16',inode_ptr->location[i]);
		print_s(buf);
		print_s(" ");
	}
	print_s("\nInode permissions: ");
	itoa(buf,'d',inode_ptr->perm);
	print_s(buf);
	print_s("\nInode is used: ");
	itoa(buf,'d',inode_ptr->in_use);
	print_s(buf);
	print_s("\n");

}


void show_bitmap(void){
	char buf[16];

	for (int i =0; i<1024; i++){
		itoa(buf,'x',fs->bitmap[i]);
		print_s(buf);
		print_s(" ");
	}
}

