#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <ui/display.h>
#define KERNEL_LOG_NAME "ui_display_info"



typedef struct _EIA_RESOLUTION_AND_TIMING{
	u32 width;
	u32 height;
	u32 freq;
} eia_resolution_and_timing_t;



static const eia_resolution_and_timing_t _eia_timings[256]={
	[1]={640,480,60},
	[2]={720,480,60},
	[3]={720,480,60},
	[4]={1280,720,60},
	[5]={1920,540,60},
	[6]={1440,240,60},
	[7]={1440,240,60},
	[8]={1440,240,60},
	[9]={1440,240,60},
	[10]={2880,240,60},
	[11]={2880,240,60},
	[12]={2880,240,60},
	[13]={2880,240,60},
	[14]={1440,480,60},
	[15]={1440,480,60},
	[16]={1920,1080,60},
	[17]={720,576,50},
	[18]={720,576,50},
	[19]={1280,720,50},
	[20]={1920,540,50},
	[21]={1440,288,50},
	[22]={1440,288,50},
	[23]={1440,288,50},
	[24]={1440,288,50},
	[25]={2880,288,50},
	[26]={2880,288,50},
	[27]={2880,288,50},
	[28]={2880,288,50},
	[29]={1440,576,50},
	[30]={1440,576,50},
	[31]={1920,1080,50},
	[35]={2880,240,60},
	[36]={2880,240,60},
	[37]={2880,576,50},
	[38]={2880,576,50},
	[39]={1920,540,50},
	[40]={1920,540,100},
	[41]={1280,720,100},
	[42]={720,576,100},
	[43]={720,576,100},
	[44]={1440,576,100},
	[45]={1440,576,100},
	[46]={1920,540,120},
	[47]={1280,720,120},
	[48]={720,480,120},
	[49]={720,480,120},
	[50]={1440,480,120},
	[51]={1440,480,120},
	[52]={720,576,200},
	[53]={720,576,200},
	[54]={1440,288,200},
	[55]={1440,288,200},
	[56]={720,480,240},
	[57]={720,480,240},
	[58]={1440,240,240},
	[59]={1440,240,240},
	[63]={1920,1080,120},
	[64]={1920,1080,100},
	[68]={1280,720,50},
	[69]={1650,750,60},
	[70]={1280,720,100},
	[71]={1280,720,120},
	[75]={1920,1080,50},
	[76]={1920,1080,60},
	[77]={1920,1080,100},
	[78]={1920,1080,120},
	[82]={1680,720,50},
	[83]={1680,720,60},
	[84]={1680,720,100},
	[85]={1680,720,120},
	[89]={2560,1080,50},
	[90]={2560,1080,60},
	[91]={2560,1080,100},
	[92]={2560,1080,120},
	[96]={3840,2160,50},
	[97]={3840,2160,60},
	[101]={4096,2160,50},
	[102]={4096,2160,60},
	[106]={3840,2160,50},
	[107]={3840,2160,60},
	[117]={3840,2160,100},
	[118]={3840,2160,120},
	[119]={3840,2160,100},
	[120]={3840,2160,120},
	[125]={5120,2160,50},
	[126]={5120,2160,60},
	[127]={5120,2160,100},
	[193]={5120,2160,120},
	[198]={7680,4320,50},
	[199]={7680,4320,60},
	[200]={7680,4320,100},
	[201]={7680,4320,120},
	[206]={7680,4320,50},
	[207]={7680,4320,60},
	[208]={7680,4320,100},
	[209]={7680,4320,120},
	[214]={10240,4320,50},
	[215]={10240,4320,60},
	[216]={10240,4320,100},
	[217]={10240,4320,120},
	[218]={4096,2160,100},
	[219]={4096,2160,120}
};

static omm_allocator_t* KERNEL_INIT_WRITE _ui_display_info_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _ui_display_info_mode_allocator=NULL;



static void _edid_parse_descriptor(const u8* data,ui_display_info_t* out){
	if (data[3]==0xfc){
		u32 j=0;
		for (;j<13&&data[j+5]!='\n';j++){
			out->name[j]=data[j+5];
		}
		out->name[j]=0;
	}
}



static void _add_mode(u32 width,u32 height,u32 freq,ui_display_info_t* out){
	ui_display_info_mode_t* mode=omm_alloc(_ui_display_info_mode_allocator);
	mode->next=out->modes;
	mode->width=width;
	mode->height=height;
	mode->freq=freq;
	out->modes=mode;
	INFO("Detected display mode: %u x %u @ %u Hz",width,height,freq);
}



MODULE_INIT(){
	LOG("Initializing UI display information...");
	_ui_display_info_allocator=omm_init("ui_display_info",sizeof(ui_display_info_t),8,2);
	rwlock_init(&(_ui_display_info_allocator->lock));
	_ui_display_info_mode_allocator=omm_init("ui_display_info_mode",sizeof(ui_display_info_mode_t),8,4);
	rwlock_init(&(_ui_display_info_mode_allocator->lock));
}



KERNEL_PUBLIC ui_display_info_t* ui_display_info_parse_edid(const u8* edid,u32 edid_length){
	LOG("Parsing display EDID...");
	if (edid_length<128||edid_length<((edid[126]+1)<<7)){
		WARN("Invalid EDID size");
		return NULL;
	}
	for (u32 i=0;i<=(edid[126]<<7);i+=128){
		u8 sum=0;
		for (u32 j=i;j<i+128;j++){
			sum+=edid[j];
		}
		if (sum){
			WARN("Invalid EDID checksum in block #%u",i>>7);
			return NULL;
		}
	}
	if ((edid[0]|edid[7])||(edid[1]&edid[2]&edid[3]&edid[4]&edid[5]&edid[6])!=0xff){
		WARN("Invalid EDID header");
		return NULL;
	}
	if (edid[18]!=1||edid[19]!=4){
		WARN("Wrong EDID version");
		return NULL;
	}
	ui_display_info_t* out=omm_alloc(_ui_display_info_allocator);
	out->edid=amm_alloc(edid_length);
	mem_copy(out->edid,edid,edid_length);
	out->edid_length=edid_length;
	u16 manufacturer_id=__builtin_bswap16(*((const u16*)(edid+8)));
	out->manufacturer[0]=((manufacturer_id>>10)&0x1f)+64;
	out->manufacturer[1]=((manufacturer_id>>5)&0x1f)+64;
	out->manufacturer[2]=(manufacturer_id&0x1f)+64;
	out->manufacturer[3]=0;
	out->manufacturer_product_code=*((const u16*)(edid+10));
	out->serial_number=*((const u16*)(edid+12));
	out->video_interface=((edid[20]>>7)?edid[20]&15:UI_DISPLAY_INFO_VIDEO_INTERFACE_ANALOG);
	if (!edid[21]||!edid[22]){
		out->screen_height_cm=0;
		out->screen_height_cm=0;
	}
	else{
		out->screen_width_cm=edid[21];
		out->screen_height_cm=edid[22];
	}
	out->modes=NULL;
	if (edid[35]&0x01){
		_add_mode(800,600,60,out);
	}
	if (edid[35]&0x02){
		_add_mode(800,600,56,out);
	}
	if (edid[35]&0x04){
		_add_mode(640,480,75,out);
	}
	if (edid[35]&0x08){
		_add_mode(640,480,72,out);
	}
	if (edid[35]&0x10){
		_add_mode(640,480,67,out);
	}
	if (edid[35]&0x20){
		_add_mode(640,480,60,out);
	}
	if (edid[35]&0x40){
		_add_mode(720,400,88,out);
	}
	if (edid[35]&0x80){
		_add_mode(720,400,70,out);
	}
	if (edid[36]&0x01){
		_add_mode(1280,1024,75,out);
	}
	if (edid[36]&0x02){
		_add_mode(1024,768,75,out);
	}
	if (edid[36]&0x04){
		_add_mode(1024,768,70,out);
	}
	if (edid[36]&0x08){
		_add_mode(1024,768,60,out);
	}
	if (edid[36]&0x10){
		_add_mode(1024,768,87,out);
	}
	if (edid[36]&0x20){
		_add_mode(832,624,75,out);
	}
	if (edid[36]&0x40){
		_add_mode(800,600,75,out);
	}
	if (edid[36]&0x80){
		_add_mode(800,600,72,out);
	}
	if (edid[37]&0x80){
		_add_mode(1152,870,75,out);
	}
	for (u8 i=38;i<54;i+=2){
		if (edid[i]==1&&edid[i+1]==1){
			continue;
		}
		u32 x=(edid[i]+31)<<3;
		u32 y=0;
		switch (edid[i+1]>>6){
			case 0:
				y=x*10/16;
				break;
			case 1:
				y=x*3/4;
				break;
			case 2:
				y=x*4/5;
				break;
			case 3:
				y=x*9/16;
				break;
		}
		_add_mode(x,y,(edid[i+1]&0x3f)+60,out);
	}
	out->name[0]=0;
	for (u32 i=54;i<126;i+=18){
		if (edid[i]||edid[i+1]){
			continue;
		}
		_edid_parse_descriptor(edid+i,out);
	}
	for (u32 i=128;i<=(edid[126]<<7);i+=128){
		if (edid[i]!=0x02||edid[i+1]!=0x03){
			continue;
		}
		if (edid[i+2]){
			for (u32 j=edid[i+2];j<110;j+=18){
				if (!edid[i+j]&&!edid[i+j+1]){
					break;
				}
				_edid_parse_descriptor(edid+i+j,out);
			}
		}
		for (u32 j=4;j<edid[i+2];j+=edid[j]&0x1f){
			u8 type=edid[i+j]>>5;
			bool is_extended=(type==7);
			const u8* data=edid+i+j+is_extended+1;
			if (is_extended){
				type=edid[i+j+1];
			}
			if (!is_extended&&type==0x02){
				u8 vic=data[0]-(data[0]>127&&data[0]<193?128:0);
				if (_eia_timings[vic].width){
					_add_mode(_eia_timings[vic].width,_eia_timings[vic].height,_eia_timings[vic].freq,out);
				}
			}
		}
	}
	return out;
}



KERNEL_PUBLIC const ui_display_info_mode_t* ui_display_info_find_mode(const ui_display_info_t* display_info,u32 width,u32 height,u32 freq){
	for (const ui_display_info_mode_t* mode=display_info->modes;mode;mode=mode->next){
		if ((!width||mode->width==width)&&(!height||mode->height==height)&&(!freq||mode->freq==freq)){
			return mode;
		}
	}
	return NULL;
}
