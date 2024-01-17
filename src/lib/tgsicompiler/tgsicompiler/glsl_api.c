#include <glsl/backend.h>
#include <sys/types.h>



static const glsl_backend_descriptor_t _tgsi_glsl_backend_descriptor={
	"tgsi"
};



SYS_PUBLIC const glsl_backend_descriptor_t* _glsl_backend_query_descriptor(void){
	return &_tgsi_glsl_backend_descriptor;
}
