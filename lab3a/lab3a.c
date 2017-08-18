/* Name: Jeffrey Xu
 * Email: jeffreyhxu@gmail.com
 * ID: 404768745
 */
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "ext2_fs.h"

int main(int argc, char **argv){
  if(argc != 2){
    fprintf(stderr, "Usage: lab3a imagefile\n");
    exit(1);
  }
  int imfd = open(argv[1], O_RDONLY);
  if(imfd == -1){
    fprintf(stderr, "Error using open to open image file: %s\n", strerror(errno));
    exit(2);
  }
  struct ext2_super_block sb;
  if(pread(imfd, &sb, sizeof(sb), 1024) == -1){
    fprintf(stderr, "Error using pread to read superblock: %s\n", strerror(errno));
    exit(2);
  }
  __u32 block_size = EXT2_MIN_BLOCK_SIZE << sb.s_log_block_size;
  /* SUPERBLOCK,# of blocks,# of inodes,block size (B),i-node size,blocks/group,
     i-nodes/group,1st non-reserved i-node */
  printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", sb.s_blocks_count, sb.s_inodes_count, block_size, sb.s_inode_size, sb.s_blocks_per_group, sb.s_inodes_per_group, sb.s_first_ino);
  
  __u32 bgdt = 0;
  while(bgdt < 2048)
    bgdt += block_size;
  if((sb.s_blocks_count - 1) / sb.s_blocks_per_group != (sb.s_inodes_count - 1) / sb.s_inodes_per_group)
    fprintf(stderr, "Inconsistent number of groups uhoh\n");
  for(int g = 0; g < (sb.s_blocks_count - 1) / sb.s_blocks_per_group + 1; g++){
    struct ext2_group_desc group;
    if(pread(imfd, &group, sizeof(group), bgdt + g * sizeof(group)) == -1){
      fprintf(stderr, "Error using pread to read block group descriptor %d: %s\n", g, strerror(errno));
      exit(2);
    }
    __u32 blockshere = sb.s_blocks_per_group;
    __u32 inodeshere = sb.s_inodes_per_group;
    if(g == (sb.s_blocks_count - 1) / sb.s_blocks_per_group){
      blockshere = sb.s_blocks_count % sb.s_blocks_per_group;
      if(blockshere == 0)
	blockshere = sb.s_blocks_per_group;
      inodeshere = sb.s_inodes_count % sb.s_inodes_per_group;
      if(inodeshere == 0)
	inodeshere = sb.s_inodes_per_group;
    }
    /* GROUP,group number (from zero),blocks in this group,i-nodes,# of free
       blocks,# of free i-nodes,block number of free block bitmap,of free i-node
       bitmap,of first block of i-nodes */
    printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", g, blockshere, inodeshere, group.bg_free_blocks_count, group.bg_free_inodes_count, group.bg_block_bitmap, group.bg_inode_bitmap, group.bg_inode_table);
    
    unsigned char *fbb = malloc(blockshere / 8);
    if(pread(imfd, fbb, blockshere / 8, group.bg_block_bitmap * block_size) == -1){
      fprintf(stderr, "Error using pread to read free block bitmap of group %d: %s\n", g, strerror(errno));
      exit(2);
    }
    for(int b = 0; b < blockshere; b++){
      if(!((fbb[b / 8] >> (b % 8)) & 1))
	/* BFREE,number of the free block */
	printf("BFREE,%d\n", b + g * sb.s_blocks_per_group + bgdt / block_size - 1);
    }
    free(fbb);
    
    unsigned char *fib = malloc(inodeshere / 8);
    if(pread(imfd, fib, inodeshere / 8, group.bg_inode_bitmap * block_size) == -1){
      fprintf(stderr, "Error using pread to read free inode bitmap of group %d: %s\n", g, strerror(errno));
      exit(2);
    }
    for(int i = 0; i < inodeshere; i++){
      if(!((fib[i / 8] >> (i % 8)) & 1))
	/* IFREE,number of the free i-node */
	printf("IFREE,%d\n", i + g * sb.s_inodes_per_group + 1);
      
      struct ext2_inode inode;
      if(pread(imfd, &inode, sizeof(inode), group.bg_inode_table * block_size + i * sizeof(inode)) == -1){
	fprintf(stderr, "Error using pread to read inode %d: %s\n", i + g * sb.s_inodes_per_group + 1, strerror(errno));
	exit(2);
      }
      if(inode.i_mode != 0 && inode.i_links_count != 0){
	char filetype = '?';
	switch(inode.i_mode >> 12){
	case 0xA:
	  filetype = 's';
	  break;
	case 0x8:
	  filetype = 'f';
	  break;
	case 0x4:
	  filetype = 'd';
	  break;
	}
	time_t timet = inode.i_mtime;
	char *modtime = malloc(32);
	if(!strftime(modtime, 31, "%D %T", gmtime(&timet))){
	  fprintf(stderr, "Error using strftime to get modification time: %s\n", strerror(errno));
	  exit(2);
	}
	timet = inode.i_atime;
	char *acctime = malloc(32);
	if(!strftime(acctime, 31, "%D %T", gmtime(&timet))){
	  fprintf(stderr, "Error using strftime to get access time: %s\n", strerror(errno));
	  exit(2);
        }
	/* INODE,inode number,file type,mode,owner,group,link count,time of last
	   inode change,modification time,time of last access,file size,number
	   of blocks,block addresses (15) */
	printf("INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d", i + g * sb.s_inodes_per_group + 1, filetype, inode.i_mode & 0xFFF, inode.i_uid, inode.i_gid, inode.i_links_count, inode.i_mtime > inode.i_atime ? modtime : acctime, modtime, acctime, inode.i_size, inode.i_blocks);
	for(int block = 0; block < EXT2_N_BLOCKS; block++){
	  printf(",%d", inode.i_block[block]);
	  if(filetype == 's')
	    break;
	}
	printf("\n");
	free(modtime);
	free(acctime);
	
	if(filetype == 'd'){
	  struct ext2_dir_entry dirent;
	  int deloc;
	  deloc = inode.i_block[0] * block_size;
	  //else if( indirect block dirent hmm... also how to go past invalid dirents onto the indirect blocks
	  int validblock = 1;
	  while(validblock){
	    if(pread(imfd, &dirent, sizeof(dirent), deloc) == -1){
	      fprintf(stderr, "Error using pread to read directory entry: %s\n", strerror(errno));
	      exit(2);
	    }
	    if(dirent.inode)
	      /* DIRENT,parent inode #,byte offset,inode of file,entry length,
		 name length,name */
	      printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n", i + g * sb.s_inodes_per_group + 1, deloc - inode.i_block[0] * block_size, dirent.inode, dirent.rec_len, dirent.name_len, dirent.name);
	    deloc += (int) dirent.rec_len;
	    // this kludge of massive code is to check if the block pointed to
	    // by this dirent is still part of this inode. It sucks and I hate.
	    int delblock = deloc / block_size;
	    validblock = 0;
	    for(int b = 0; b < EXT2_NDIR_BLOCKS; b++){
	      if(inode.i_block[b] == delblock){
		validblock = 1;
		break;
	      }
	    }
	    if(!validblock && inode.i_block[EXT2_IND_BLOCK]){
	      for(int ib = 0; ib < block_size / 4; ib++){
		__u32 ipb;
		if(pread(imfd, &ipb, 4, inode.i_block[EXT2_IND_BLOCK] * block_size + ib * 4) == -1){
		  fprintf(stderr, "Error using pread to examine indirect block to find block of directory entry: %s\n", strerror(errno));
		  exit(2);
		}
		if(ipb == delblock){
		  validblock = 1;
		  break;
		}
	      }
	    }
	    if(!validblock && inode.i_block[EXT2_DIND_BLOCK]){
	      for(int ib = 0; ib < block_size / 4; ib++){
		__u32 dipb;
		if(pread(imfd, &dipb, 4, inode.i_block[EXT2_DIND_BLOCK] * block_size + ib * 4) == -1){
		  fprintf(stderr, "Error using pread to examine doubly indirect block to find block of directory entry: %s\n", strerror(errno));
		  exit(2);
		}
		if(dipb){
		  for(int dib = 0; dib < block_size / 4; dib ++){
		    __u32 ipb;
		    if(pread(imfd, &ipb, 4, dipb * block_size + dib * 4) == -1){
		      fprintf(stderr, "Error using pread to examine indirect block to find block of directory entry: %s\n", strerror(errno));
		      exit(2);
		    }
		    if(ipb == delblock){
		      validblock = 1;
		      break;
		    }
		  }
		}
		if(validblock)
		  break;
	      }
	    }
	    if(!validblock && inode.i_block[EXT2_TIND_BLOCK]){
	      for(int ib = 0; ib < block_size / 4; ib++){
		__u32 tipb;
		if(pread(imfd, &tipb, 4, inode.i_block[EXT2_TIND_BLOCK] * block_size + ib * 4) == -1){
		  fprintf(stderr, "Error using pread to examine triply indirect block to find block of directory entry: %s\n", strerror(errno));
		  exit(2);
		}
		if(tipb){
		  for(int dib = 0; dib < block_size / 4; dib ++){
		    __u32 dipb;
		    if(pread(imfd, &dipb, 4, tipb * block_size + dib * 4) == -1){
		      fprintf(stderr, "Error using pread to examine doubly indirect block to find block of directory entry: %s\n", strerror(errno));
		      exit(2);
		    }
		    if(dipb){
		      for(int tib = 0; tib < block_size / 4; tib++){
			__u32 ipb;
			if(pread(imfd, &ipb, 4, dipb * block_size + tib * 4) == -1){
			  fprintf(stderr, "Error using pread to examine direct block to find block of directory entry: %s\n", strerror(errno));
			  exit(2);
			}
			if(ipb == delblock){
			  validblock = 1;
			  break;
			}
		      }
		    }
		    if(validblock)
		      break;
		  }
		}
		if(validblock)
		  break;
	      }
	    }
	  }
	}
	
	if(filetype == 'd' || filetype == 'f'){
	  // I'm reusing code from traversing indirect blocks to find the block
	  // of the directory entry. A better solution would be able to do it
	  // once for both. Oh well. At least I can fix the crazy var names.
	  /* INDIRECT,owning inode,level of indirection,logical block offset,
	     containing block,referenced block */
	  if(inode.i_block[EXT2_IND_BLOCK]){
	    for(int ib = 0; ib < block_size / 4; ib++){
	      __u32 bl;
	      if(pread(imfd, &bl, 4, inode.i_block[EXT2_IND_BLOCK] * block_size + ib * 4) == -1){
		fprintf(stderr, "Error using pread to scan indirect block: %s\n", strerror(errno));
		exit(2);
	      }
	      if(bl)
		printf("INDIRECT,%d,%d,%d,%d,%d\n", i + g * sb.s_inodes_per_group + 1, 1, 12 + ib, inode.i_block[EXT2_IND_BLOCK], bl);
	    }
	  }
	  if(inode.i_block[EXT2_DIND_BLOCK]){
	    for(int dib = 0; dib < block_size / 4; dib++){
	      __u32 ibl;
	      if(pread(imfd, &ibl, 4, inode.i_block[EXT2_DIND_BLOCK] * block_size + dib * 4) == -1){
		fprintf(stderr, "Error using pread to scan doubly indirect block: %s\n", strerror(errno));
		exit(2);
	      }
	      if(ibl){
		printf("INDIRECT,%d,%d,%d,%d,%d\n", i + g * sb.s_inodes_per_group + 1, 2, 12 + (block_size / 4) * (1 + dib), inode.i_block[EXT2_DIND_BLOCK], ibl);
		for(int ib = 0; ib < block_size / 4; ib++){
		  __u32 bl;
		  if(pread(imfd, &bl, 4, ibl * block_size + ib * 4) == -1){
		    fprintf(stderr, "Error using pread to scan indirect block: %s\n", strerror(errno));
		    exit(2);
		  }
		  if(bl)
		    printf("INDIRECT,%d,%d,%d,%d,%d\n", i + g * sb.s_inodes_per_group + 1, 1, 12 + (block_size / 4) * (1 + dib) + ib, ibl, bl);
		}
	      }
	    }
	  }
	  if(inode.i_block[EXT2_TIND_BLOCK]){
	    for(int tib = 0; tib < block_size / 4; tib++){
	      __u32 dibl;
	      if(pread(imfd, &dibl, 4, inode.i_block[EXT2_TIND_BLOCK] * block_size + tib * 4) == -1){
		fprintf(stderr, "Error using pread to scan triply indirect block: %s\n", strerror(errno));
		exit(2);
	      }
	      if(dibl){
		printf("INDIRECT,%d,%d,%d,%d,%d\n", i + g * sb.s_inodes_per_group + 1, 3, 12 + (block_size / 4) * (1 + block_size / 4) + (block_size / 4) * (block_size / 4) * tib, inode.i_block[EXT2_TIND_BLOCK], dibl);
		for(int dib = 0; dib < block_size / 4; dib++){
		  __u32 ibl;
		  if(pread(imfd, &ibl, 4, dibl * block_size + dib * 4) == -1){
		    fprintf(stderr, "Error using pread to scan doubly indirect block: %s\n", strerror(errno));
		    exit(2);
		  }
		  if(ibl){
		    printf("INDIRECT,%d,%d,%d,%d,%d\n", i + g * sb.s_inodes_per_group + 1, 2, 12 + (block_size / 4) * (1 + block_size / 4 + dib) + (block_size / 4) * (block_size / 4) * tib, dibl, ibl);
		    for(int ib = 0; ib < block_size / 4; ib++){
		      __u32 bl;
		      if(pread(imfd, &bl, 4, ibl * block_size + ib * 4) == -1){
			fprintf(stderr, "Error using pread to scan indirect block: %s\n", strerror(errno));
			exit(2);
		      }
		      if(bl)
			printf("INDIRECT,%d,%d,%d,%d,%d\n", i + g * sb.s_inodes_per_group + 1, 1, 12 + (block_size / 4) * (1 + block_size / 4 + dib) + (block_size / 4) * (block_size / 4) * tib + ib, ibl, bl);
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
    free(fib);
  }
}
