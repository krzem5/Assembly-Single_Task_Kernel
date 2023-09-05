#include <user/aml.h>
#include <user/io.h>



static void _print_node(const aml_node_t* node,u32 indent,_Bool inside_package){
	for (u32 i=0;i<indent;i+=4){
		printf("    ");
	}
	if (!inside_package){
		printf("%s:",node->name);
	}
	switch (node->type){
		case AML_NODE_TYPE_UNDEFINED:
			printf("<undefined>");
			goto _end;
		case AML_NODE_TYPE_BUFFER:
			printf("buffer<size=%u>",node->data.buffer.length);
			if (node->data.buffer.length>16){
				goto _end;
			}
			printf("{",node->data.buffer.length);
			for (u8 i=0;i<node->data.buffer.length;i++){
				if (i){
					printf(" ");
				}
				printf("%x",node->data.buffer.data[i]);
			}
			printf("}");
			goto _end;
		case AML_NODE_TYPE_BUFFER_FIELD:
			printf("buffer_field<...>");
			break;
		case AML_NODE_TYPE_DEBUG:
			printf("debug<...>");
			break;
		case AML_NODE_TYPE_DEVICE:
			printf("device");
			break;
		case AML_NODE_TYPE_EVENT:
			printf("event<...>");
			goto _end;
		case AML_NODE_TYPE_FIELD_UNIT:
			printf("field_unit<address=%p, size=%u>",node->data.field_unit.address,node->data.field_unit.size);
			goto _end;
		case AML_NODE_TYPE_INTEGER:
			printf("0x%lx",node->data.integer);
			goto _end;
		case AML_NODE_TYPE_METHOD:
			printf("method<arg_count=%u>",node->data.method.flags&7);
			goto _end;
		case AML_NODE_TYPE_MUTEX:
			printf("mutex<...>");
			goto _end;
		case AML_NODE_TYPE_REFERENCE:
			printf("reference<...>");
			goto _end;
		case AML_NODE_TYPE_REGION:
			printf("region<type=");
			switch (node->data.region.type){
				case 0:
					printf("SystemMemory");
					break;
				case 1:
					printf("SystemIO");
					break;
				case 2:
					printf("PCI_Config");
					break;
				case 3:
					printf("EmbeddedControl");
					break;
				case 4:
					printf("SMBus");
					break;
				case 5:
					printf("SystemCMOS");
					break;
				case 6:
					printf("PciBarTarget");
					break;
				case 7:
					printf("IPMI");
					break;
				case 8:
					printf("GeneralPurposeIO");
					break;
				case 9:
					printf("GenericSerialBus");
					break;
				case 10:
					printf("PCC");
					break;
				default:
					printf("Unknown");
					break;
			}
			printf(", offset=%u, length=%u>",node->data.region.offset,node->data.region.length);
			goto _end;
		case AML_NODE_TYPE_POWER_RESOURCE:
			printf("power_resource<...>");
			break;
		case AML_NODE_TYPE_PROCESSOR:
			printf("processor<id=%u>",node->data.processor.id);
			break;
		case AML_NODE_TYPE_STRING:
			printf("'%s'",node->data.string.data);
			goto _end;
		case AML_NODE_TYPE_THERMAL_ZONE:
			printf("thermal_zone<...>");
			break;
	}
	if ((node->type==AML_NODE_TYPE_PACKAGE?!node->data.package.length:!node->child)){
		printf("{}");
		goto _end;
	}
	printf("{\n");
	if (node->type==AML_NODE_TYPE_PACKAGE){
		for (u8 i=0;i<node->data.package.length;i++){
			_print_node(node->data.package.elements+i,indent+4,1);
			if (i+1<node->data.package.length){
				printf(",\n");
			}
			else{
				printf("\n");
			}
		}
	}
	else{
		for (const aml_node_t* child=node->child;child;child=child->next){
			_print_node(child,indent+4,0);
			if (child->next){
				printf(",\n");
			}
			else{
				printf("\n");
			}
		}
	}
	for (u32 i=0;i<indent;i+=4){
		printf("    ");
	}
	printf("}");
_end:
	if (!indent){
		printf("\n");
	}
}



void aml_print_node(const aml_node_t* node){
	_print_node(node,0,0);
}
