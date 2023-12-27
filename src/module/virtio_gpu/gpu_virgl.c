#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "virtio_gpu_virtgl"



#define VIRGL_CAP_V1_INDEP_BLEND_ENABLE 0x00000001
#define VIRGL_CAP_V1_INDEP_BLEND_FUNC 0x00000002
#define VIRGL_CAP_V1_CUBE_MAP_ARRAY 0x00000004
#define VIRGL_CAP_V1_SHADER_STENCIL_EXPORT 0x00000008
#define VIRGL_CAP_V1_CONDITIONAL_RENDER 0x00000010
#define VIRGL_CAP_V1_START_INSTANCE 0x00000020
#define VIRGL_CAP_V1_PRIMITIVE_RESTART 0x00000040
#define VIRGL_CAP_V1_BLEND_EQ_SEP 0x00000080
#define VIRGL_CAP_V1_INSTANCEID 0x00000100
#define VIRGL_CAP_V1_VERTEX_ELEMENT_INSTANCE_DIVISOR 0x00000200
#define VIRGL_CAP_V1_SEAMLESS_CUBE_MAP 0x00000400
#define VIRGL_CAP_V1_OCCLUSION_QUERY 0x00000800
#define VIRGL_CAP_V1_TIMER_QUERY 0x00001000
#define VIRGL_CAP_V1_STREAMOUT_PAUSE_RESUME 0x00002000
#define VIRGL_CAP_V1_TEXTURE_MULTISAMPLE 0x00004000
#define VIRGL_CAP_V1_FRAGMENT_COORD_CONVENTIONS 0x00008000
#define VIRGL_CAP_V1_DEPTH_CLIP_DISABLE 0x00010000
#define VIRGL_CAP_V1_SEAMLESS_CUBE_MAP_PER_TEXTURE 0x00020000
#define VIRGL_CAP_V1_UBO 0x00040000
#define VIRGL_CAP_V1_COLOR_CLAMPING 0x00080000
#define VIRGL_CAP_V1_POLY_STIPPLE 0x00100000
#define VIRGL_CAP_V1_MIRROR_CLAMP 0x00200000
#define VIRGL_CAP_V1_TEXTURE_QUERY_LOD 0x00400000
#define VIRGL_CAP_V1_HAS_FP64 0x00800000
#define VIRGL_CAP_V1_HAS_TESSELLATION_SHADERS 0x01000000
#define VIRGL_CAP_V1_HAS_INDIRECT_DRAW 0x02000000
#define VIRGL_CAP_V1_HAS_SAMPLE_SHADING 0x04000000
#define VIRGL_CAP_V1_HAS_CULL 0x08000000
#define VIRGL_CAP_V1_CONDITIONAL_RENDER_INVERTED 0x10000000
#define VIRGL_CAP_V1_DERIVATIVE_CONTROL 0x20000000
#define VIRGL_CAP_V1_POLYGON_OFFSET_CLAMP 0x40000000
#define VIRGL_CAP_V1_TRANSFORM_FEEDBACK_OVERFLOW_QUERY 0x80000000

#define VIRGL_CAP_TGSI_INVARIANT 0x00000001
#define VIRGL_CAP_TEXTURE_VIEW 0x00000002
#define VIRGL_CAP_SET_MIN_SAMPLES 0x00000004
#define VIRGL_CAP_COPY_IMAGE 0x00000008
#define VIRGL_CAP_TGSI_PRECISE 0x00000010
#define VIRGL_CAP_TXQS 0x00000020
#define VIRGL_CAP_MEMORY_BARRIER 0x00000040
#define VIRGL_CAP_COMPUTE_SHADER 0x00000080
#define VIRGL_CAP_FB_NO_ATTACH 0x00000100
#define VIRGL_CAP_ROBUST_BUFFER_ACCESS 0x00000200
#define VIRGL_CAP_TGSI_FBFETCH 0x00000400
#define VIRGL_CAP_SHADER_CLOCK 0x00000800
#define VIRGL_CAP_TEXTURE_BARRIER 0x00001000
#define VIRGL_CAP_TGSI_COMPONENTS 0x00002000
#define VIRGL_CAP_GUEST_MAY_INIT_LOG 0x00004000
#define VIRGL_CAP_SRGB_WRITE_CONTROL 0x00008000
#define VIRGL_CAP_QBO 0x00010000
#define VIRGL_CAP_TRANSFER 0x00020000
#define VIRGL_CAP_FBO_MIXED_COLOR_FORMATS 0x00040000
#define VIRGL_CAP_HOST_IS_GLES 0x00080000
#define VIRGL_CAP_BIND_COMMAND_ARGS 0x00100000
#define VIRGL_CAP_MULTI_DRAW_INDIRECT 0x00200000
#define VIRGL_CAP_INDIRECT_PARAMS 0x00400000
#define VIRGL_CAP_TRANSFORM_FEEDBACK3 0x00800000
#define VIRGL_CAP_3D_ASTC 0x01000000
#define VIRGL_CAP_INDIRECT_INPUT_ADDR 0x02000000
#define VIRGL_CAP_COPY_TRANSFER 0x04000000
#define VIRGL_CAP_CLIP_HALFZ 0x08000000
#define VIRGL_CAP_APP_TWEAK_SUPPORT 0x10000000
#define VIRGL_CAP_BGRA_SRGB_IS_EMULATED 0x20000000
#define VIRGL_CAP_CLEAR_TEXTURE 0x40000000
#define VIRGL_CAP_ARB_BUFFER_STORAGE 0x80000000

#define VIRGL_CAP_V2_BLEND_EQUATION 0x00000001
#define VIRGL_CAP_V2_UNTYPED_RESOURCE 0x00000002
#define VIRGL_CAP_V2_VIDEO_MEMORY 0x00000004
#define VIRGL_CAP_V2_MEMINFO 0x00000008
#define VIRGL_CAP_V2_STRING_MARKER 0x00000010
#define VIRGL_CAP_V2_DIFFERENT_GPU 0x00000020
#define VIRGL_CAP_V2_IMPLICIT_MSAA 0x00000040
#define VIRGL_CAP_V2_COPY_TRANSFER_BOTH_DIRECTIONS 0x00000080
#define VIRGL_CAP_V2_SCANOUT_USES_GBM 0x00000100
#define VIRGL_CAP_V2_SSO 0x00000200
#define VIRGL_CAP_V2_TEXTURE_SHADOW_LOD 0x00000400
#define VIRGL_CAP_V2_VS_VERTEX_LAYER 0x00000800
#define VIRGL_CAP_V2_VS_VIEWPORT_INDEX 0x00001000
#define VIRGL_CAP_V2_PIPELINE_STATISTICS_QUERY 0x00002000
#define VIRGL_CAP_V2_DRAW_PARAMETERS 0x00004000
#define VIRGL_CAP_V2_GROUP_VOTE 0x00008000
#define VIRGL_CAP_V2_MIRROR_CLAMP_TO_EDGE 0x00010000



typedef struct _VIRGL_SUPPORTED_FORMAT_MASK{
	u32 bitmask[16];
} virgl_supported_format_mask_t;



typedef struct _VIRGL_VIDEO_CAPS{
	u8 profile;
	u8 entrypoint;
	u8 max_level;
	u8 stacked_frames;
	u16 max_width;
	u16 max_height;
	u16 prefered_format;
	u16 max_macroblocks;
	u32 npot_texture:1;
	u32 supports_progressive:1;
	u32 supports_interlaced:1;
	u32 prefers_interlaced:1;
	u32 max_temporal_layers:8;
	u32 reserved:20;
} virgl_video_caps_t;



typedef struct _VIRGL_CAPS_V2{
	u32 max_version;
	virgl_supported_format_mask_t sampler;
	virgl_supported_format_mask_t render;
	virgl_supported_format_mask_t depthstencil;
	virgl_supported_format_mask_t vertexbuffer;
	u32 capability_bits_v1;
	u32 glsl_level;
	u32 max_texture_array_layers;
	u32 max_streamout_buffers;
	u32 max_dual_source_render_targets;
	u32 max_render_targets;
	u32 max_samples;
	u32 prim_mask;
	u32 max_tbo_size;
	u32 max_uniform_blocks;
	u32 max_viewports;
	u32 max_texture_gather_components;
	float min_aliased_point_size;
	float max_aliased_point_size;
	float min_smooth_point_size;
	float max_smooth_point_size;
	float min_aliased_line_width;
	float max_aliased_line_width;
	float min_smooth_line_width;
	float max_smooth_line_width;
	float max_texture_lod_bias;
	u32 max_geom_output_vertices;
	u32 max_geom_total_output_components;
	u32 max_vertex_outputs;
	u32 max_vertex_attribs;
	u32 max_shader_patch_varyings;
	s32 min_texel_offset;
	s32 max_texel_offset;
	s32 min_texture_gather_offset;
	s32 max_texture_gather_offset;
	u32 texture_buffer_offset_alignment;
	u32 uniform_buffer_offset_alignment;
	u32 shader_buffer_offset_alignment;
	u32 capability_bits;
	u32 sample_locations[8];
	u32 max_vertex_attrib_stride;
	u32 max_shader_buffer_frag_compute;
	u32 max_shader_buffer_other_stages;
	u32 max_shader_image_frag_compute;
	u32 max_shader_image_other_stages;
	u32 max_image_samples;
	u32 max_compute_work_group_invocations;
	u32 max_compute_shared_memory_size;
	u32 max_compute_grid_size[3];
	u32 max_compute_block_size[3];
	u32 max_texture_2d_size;
	u32 max_texture_3d_size;
	u32 max_texture_cube_size;
	u32 max_combined_shader_buffers;
	u32 max_atomic_counters[6];
	u32 max_atomic_counter_buffers[6];
	u32 max_combined_atomic_counters;
	u32 max_combined_atomic_counter_buffers;
	u32 host_feature_check_version;
	virgl_supported_format_mask_t supported_readback_formats;
	virgl_supported_format_mask_t scanout;
	u32 capability_bits_v2;
	u32 max_video_memory;
	char renderer[64];
	float max_anisotropy;
	u32 max_texture_image_units;
	virgl_supported_format_mask_t supported_multisample_formats;
	u32 max_const_buffer_size[6];
	u32 num_video_caps;
	virgl_video_caps_t video_caps[32];
	u32 max_uniform_block_size;
} virgl_caps_v2_t;



void virtio_gpu_virgl_load_opengl_from_capset(_Bool is_v2,const void* data,u32 size){
	LOG("Initializing OpenGL with virgl%s backend...",(is_v2?"2":""));
	if (size<sizeof(virgl_caps_v2_t)){
		WARN("Incomplete virgl caps (partial read?)");
		return;
	}
	const virgl_caps_v2_t* caps=data;
	WARN("Renderer: %s",caps->renderer);
	// panic("AAA");
}
