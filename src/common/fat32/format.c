#include <common/fat32/api.h>
#include <common/fat32/util.h>



bool fat32_filesystem_format(const fat32_filesystem_config_t* config,fat32_filesystem_t* out){
	u32 block_count=config->end_lba-config->start_lba+1;
	if (config->block_size!=FAT32_SECTOR_SIZE||block_count<32){
		return 0;
	}
	u32 sectors_per_fat=(block_count-32)/(FAT32_SECTOR_SIZE/sizeof(u32));
	u8 buffer[512];
	mem_fill(buffer,sizeof(buffer),0);
	buffer[0]=0xeb;
	buffer[1]=88;
	buffer[2]=0x90;
	buffer[3]='F';
	buffer[4]='A';
	buffer[5]='T';
	buffer[6]='3';
	buffer[7]='2';
	buffer[8]=' ';
	buffer[9]=' ';
	buffer[10]=' ';
	buffer[11]=FAT32_SECTOR_SIZE&0xff;
	buffer[12]=FAT32_SECTOR_SIZE>>8;
	buffer[13]=1;
	buffer[14]=32;
	buffer[16]=2;
	buffer[21]=0xf8;
	buffer[24]=1;
	buffer[26]=1;
	buffer[32]=block_count;
	buffer[33]=block_count>>8;
	buffer[34]=block_count>>16;
	buffer[35]=block_count>>24;
	buffer[36]=sectors_per_fat;
	buffer[37]=sectors_per_fat>>8;
	buffer[38]=sectors_per_fat>>16;
	buffer[39]=sectors_per_fat>>24;
	buffer[44]=2;
	buffer[48]=1;
	buffer[66]=0x29;
	buffer[67]=0xaa;
	buffer[68]=0xbb;
	buffer[69]=0xcc;
	buffer[70]=0xdd;
	buffer[71]='N';
	buffer[72]='O';
	buffer[73]=' ';
	buffer[74]='N';
	buffer[75]='A';
	buffer[76]='M';
	buffer[77]='E';
	buffer[78]=' ';
	buffer[79]=' ';
	buffer[80]=' ';
	buffer[81]=' ';
	buffer[82]='F';
	buffer[83]='A';
	buffer[84]='T';
	buffer[85]='3';
	buffer[86]='2';
	buffer[87]=' ';
	buffer[88]=' ';
	buffer[89]=' ';
	buffer[510]=0x55;
	buffer[511]=0xaa;
	if (config->write_callback(config->ctx,config->start_lba,buffer,1)!=1){
		return 0;
	}
	mem_fill(buffer,sizeof(buffer),0);
	buffer[0]='R';
	buffer[1]='R';
	buffer[2]='a';
	buffer[3]='A';
	buffer[484]='r';
	buffer[485]='r';
	buffer[486]='A';
	buffer[487]='a';
	buffer[488]=0xff;
	buffer[489]=0xff;
	buffer[490]=0xff;
	buffer[491]=0xff;
	buffer[492]=0xff;
	buffer[493]=0xff;
	buffer[494]=0xff;
	buffer[495]=0xff;
	buffer[510]=0x55;
	buffer[511]=0xaa;
	if (config->write_callback(config->ctx,config->start_lba+1,buffer,1)!=1){
		return 0;
	}
	mem_fill(buffer,sizeof(buffer),0);
	for (u32 i=0;i<sectors_per_fat*2+1/*also clear root directory cluster*/;i++){
		if (config->write_callback(config->ctx,config->start_lba+32+i,buffer,1)!=1){
			return 0;
		}
	}
	buffer[0]=0xf8;
	buffer[1]=0xff;
	buffer[2]=0xff;
	buffer[3]=0x0f;
	buffer[4]=0xff;
	buffer[5]=0xff;
	buffer[6]=0xff;
	buffer[7]=0x0f;
	buffer[8]=0xff;
	buffer[9]=0xff;
	buffer[10]=0xff;
	buffer[11]=0x0f;
	if (config->write_callback(config->ctx,config->start_lba+32,buffer,1)!=1||config->write_callback(config->ctx,config->start_lba+32+sectors_per_fat,buffer,1)!=1){
		return 0;
	}
	return fat32_filesystem_init(config,out);
}
