#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "aml"


#define OP_ZERO
#define OP_ONE
#define OP_ALIAS
#define OP_NAME
#define OP_BYTE_PREF
#define OP_WORD_PREF
#define OP_D_WORD_PREF
#define OP_STRING_PREF
#define OP_Q_WORD_PREF
#define OP_SCOPE
#define OP_BUFFER
#define OP_PACKAGE
#define OP_VAR_PACKAGE
#define OP_METHOD
#define OP_DUAL_NAME_PREF
#define OP_MULTI_NAME_PREF
#define OP_ROOT_CH
#define OP_PARENT_PREFIX_CH
#define OP_NAME_CH
#define OP_LOCAL0
#define OP_LOCAL1
#define OP_LOCAL2
#define OP_LOCAL3
#define OP_LOCAL4
#define OP_LOCAL5
#define OP_LOCAL6
#define OP_LOCAL7
#define OP_ARG0
#define OP_ARG1
#define OP_ARG2
#define OP_ARG3
#define OP_ARG4
#define OP_ARG5
#define OP_ARG6
#define OP_STORE
#define OP_REF_OF
#define OP_ADD
#define OP_CONCAT
#define OP_SUBTRACT
#define OP_INCREMENT
#define OP_DECREMENT
#define OP_MULTIPLY
#define OP_DIVIDE
#define OP_SHIFT_LEFT
#define OP_SHIFT_RIGHT
#define OP_AND
#define OP_NAND
#define OP_OR
#define OP_NOR
#define OP_XOR
#define OP_NOT
#define OP_FIND_SET_LEFT_BIT
#define OP_FIND_SET_RIGHT_BIT
#define OP_DEREF_OF
#define OP_CONCAT_RES
#define OP_MOD
#define OP_NOTIFY
#define OP_SIZE_OF
#define OP_INDEX
#define OP_MATCH
#define OP_CREATE_D_WORD_FIELD
#define OP_CREATE_WORD_FIELD
#define OP_CREATE_BYTE_FIELD
#define OP_CREATE_BIT_FIELD
#define OP_TYPE
#define OP_CREATE_Q_WORD_FIELD
#define OP_LAND
#define OP_LOR
#define OP_L_NOT_EQUAL
#define OP_L_NOT_EQUAL
#define OP_L_NOT_EQUAL
#define OP_LNOT
#define OP_L_EQUAL
#define OP_L_GREATER
#define OP_L_LESS
#define OP_TO_BUFFER
#define OP_TO_DECIMAL_STRING
#define OP_TO_HEX_STRING
#define OP_TO_INTEGER
#define OP_TO_STRING
#define OP_COPY_OBJECT
#define OP_MID
#define OP_CONTINUE
#define OP_IF
#define OP_ELSE
#define OP_WHILE
#define OP_NOOP
#define OP_RETURN
#define OP_BREAK
#define OP_BREAK_POINT
#define OP_ONES

#define OP_EXT_MUTEX
#define OP_EXT_EVENT
#define OP_EXT_COND_REF_OF
#define OP_EXT_CREATE_FIELD
#define OP_EXT_LOAD_TABLE
#define OP_EXT_LOAD
#define OP_EXT_STALL
#define OP_EXT_SLEEP
#define OP_EXT_ACQUIRE
#define OP_EXT_SIGNAL
#define OP_EXT_WAIT
#define OP_EXT_RESET
#define OP_EXT_RELEASE
#define OP_EXT_FROM_BCD
#define OP_EXT_TO_B
#define OP_EXT_UNLOAD
#define OP_EXT_REVISION
#define OP_EXT_DEBUG
#define OP_EXT_FATAL
#define OP_EXT_TIMER
#define OP_EXT_OP_REGION
#define OP_EXT_FIELD
#define OP_EXT_DEVICE
#define OP_EXT_PROCESSOR
#define OP_EXT_POWER_RES
#define OP_EXT_THERMAL_ZONE
#define OP_EXT_INDEX_FIELD
#define OP_EXT_BANK_FIELD
#define OP_EXT_DATA_REGION



void aml_load(const u8* data,u32 length){
	LOG("Loading AML...");
	if (length<36){
		return;
	}
	u32 offset=36;
	while (offset<length){
		if (data[offset]!=0x5b){
			switch (data[offset]){
				case 0x00:
					ERROR("ZeroOp => OP_ZERO");return;
					break;
				case 0x01:
					ERROR("OneOp => OP_ONE");return;
					break;
				case 0x06:
					ERROR("AliasOp => OP_ALIAS");return;
					break;
				case 0x08:
					ERROR("NameOp => OP_NAME");return;
					break;
				case 0x0a:
					ERROR("BytePrefix => OP_BYTE_PREF");return;
					break;
				case 0x0b:
					ERROR("WordPrefix => OP_WORD_PREF");return;
					break;
				case 0x0c:
					ERROR("DWordPrefix => OP_D_WORD_PREF");return;
					break;
				case 0x0d:
					ERROR("StringPrefix => OP_STRING_PREF");return;
					break;
				case 0x0e:
					ERROR("QWordPrefix => OP_Q_WORD_PREF");return;
					break;
				case 0x10:
					ERROR("ScopeOp => OP_SCOPE");return;
					break;
				case 0x11:
					ERROR("BufferOp => OP_BUFFER");return;
					break;
				case 0x12:
					ERROR("PackageOp => OP_PACKAGE");return;
					break;
				case 0x13:
					ERROR("VarPackageOp => OP_VAR_PACKAGE");return;
					break;
				case 0x14:
					ERROR("MethodOp => OP_METHOD");return;
					break;
				case 0x2e:
					ERROR("DualNamePrefix => OP_DUAL_NAME_PREF");return;
					break;
				case 0x2f:
					ERROR("MultiNamePrefix => OP_MULTI_NAME_PREF");return;
					break;
				case 0x5c:
					ERROR("RootChar => OP_ROOT_CH");return;
					break;
				case 0x5e:
					ERROR("ParentPrefixChar => OP_PARENT_PREFIX_CH");return;
					break;
				case 0x5f:
					ERROR("NameChar => OP_NAME_CH");return;
					break;
				case 0x60:
					ERROR("Local0Op => OP_LOCAL0");return;
					break;
				case 0x61:
					ERROR("Local1Op => OP_LOCAL1");return;
					break;
				case 0x62:
					ERROR("Local2Op => OP_LOCAL2");return;
					break;
				case 0x63:
					ERROR("Local3Op => OP_LOCAL3");return;
					break;
				case 0x64:
					ERROR("Local4Op => OP_LOCAL4");return;
					break;
				case 0x65:
					ERROR("Local5Op => OP_LOCAL5");return;
					break;
				case 0x66:
					ERROR("Local6Op => OP_LOCAL6");return;
					break;
				case 0x67:
					ERROR("Local7Op => OP_LOCAL7");return;
					break;
				case 0x68:
					ERROR("Arg0Op => OP_ARG0");return;
					break;
				case 0x69:
					ERROR("Arg1Op => OP_ARG1");return;
					break;
				case 0x6a:
					ERROR("Arg2Op => OP_ARG2");return;
					break;
				case 0x6b:
					ERROR("Arg3Op => OP_ARG3");return;
					break;
				case 0x6c:
					ERROR("Arg4Op => OP_ARG4");return;
					break;
				case 0x6d:
					ERROR("Arg5Op => OP_ARG5");return;
					break;
				case 0x6e:
					ERROR("Arg6Op => OP_ARG6");return;
					break;
				case 0x70:
					ERROR("StoreOp => OP_STORE");return;
					break;
				case 0x71:
					ERROR("RefOfOp => OP_REF_OF");return;
					break;
				case 0x72:
					ERROR("AddOp => OP_ADD");return;
					break;
				case 0x73:
					ERROR("ConcatOp => OP_CONCAT");return;
					break;
				case 0x74:
					ERROR("SubtractOp => OP_SUBTRACT");return;
					break;
				case 0x75:
					ERROR("IncrementOp => OP_INCREMENT");return;
					break;
				case 0x76:
					ERROR("DecrementOp => OP_DECREMENT");return;
					break;
				case 0x77:
					ERROR("MultiplyOp => OP_MULTIPLY");return;
					break;
				case 0x78:
					ERROR("DivideOp => OP_DIVIDE");return;
					break;
				case 0x79:
					ERROR("ShiftLeftOp => OP_SHIFT_LEFT");return;
					break;
				case 0x7a:
					ERROR("ShiftRightOp => OP_SHIFT_RIGHT");return;
					break;
				case 0x7b:
					ERROR("AndOp => OP_AND");return;
					break;
				case 0x7c:
					ERROR("NandOp => OP_NAND");return;
					break;
				case 0x7d:
					ERROR("OrOp => OP_OR");return;
					break;
				case 0x7e:
					ERROR("NorOp => OP_NOR");return;
					break;
				case 0x7f:
					ERROR("XorOp => OP_XOR");return;
					break;
				case 0x80:
					ERROR("NotOp => OP_NOT");return;
					break;
				case 0x81:
					ERROR("FindSetLeftBitOp => OP_FIND_SET_LEFT_BIT");return;
					break;
				case 0x82:
					ERROR("FindSetRightBitOp => OP_FIND_SET_RIGHT_BIT");return;
					break;
				case 0x83:
					ERROR("DerefOfOp => OP_DEREF_OF");return;
					break;
				case 0x84:
					ERROR("ConcatResOp => OP_CONCAT_RES");return;
					break;
				case 0x85:
					ERROR("ModOp => OP_MOD");return;
					break;
				case 0x86:
					ERROR("NotifyOp => OP_NOTIFY");return;
					break;
				case 0x87:
					ERROR("SizeOfOp => OP_SIZE_OF");return;
					break;
				case 0x88:
					ERROR("IndexOp => OP_INDEX");return;
					break;
				case 0x89:
					ERROR("MatchOp => OP_MATCH");return;
					break;
				case 0x8a:
					ERROR("CreateDWordFieldOp => OP_CREATE_D_WORD_FIELD");return;
					break;
				case 0x8b:
					ERROR("CreateWordFieldOp => OP_CREATE_WORD_FIELD");return;
					break;
				case 0x8c:
					ERROR("CreateByteFieldOp => OP_CREATE_BYTE_FIELD");return;
					break;
				case 0x8d:
					ERROR("CreateBitFieldOp => OP_CREATE_BIT_FIELD");return;
					break;
				case 0x8e:
					ERROR("TypeOp => OP_TYPE");return;
					break;
				case 0x8f:
					ERROR("CreateQWordFieldOp => OP_CREATE_Q_WORD_FIELD");return;
					break;
				case 0x90:
					ERROR("LandOp => OP_LAND");return;
					break;
				case 0x91:
					ERROR("LorOp => OP_LOR");return;
					break;
				case 0x92:
					if (data[offset+1]==0x93){
						offset++;
						ERROR("LNotEqualOp => OP_L_NOT_EQUAL");return;
						break;
					}
					if (data[offset+1]==0x94){
						offset++;
						ERROR("LNotEqualOp => OP_L_NOT_EQUAL");return;
						break;
					}
					if (data[offset+1]==0x95){
						offset++;
						ERROR("LNotEqualOp => OP_L_NOT_EQUAL");return;
						break;
					}
					ERROR("LnotOp => OP_LNOT");return;
					break;
				case 0x93:
					ERROR("LEqualOp => OP_L_EQUAL");return;
					break;
				case 0x94:
					ERROR("LGreaterOp => OP_L_GREATER");return;
					break;
				case 0x95:
					ERROR("LLessOp => OP_L_LESS");return;
					break;
				case 0x96:
					ERROR("ToBufferOp => OP_TO_BUFFER");return;
					break;
				case 0x97:
					ERROR("ToDecimalStringOp => OP_TO_DECIMAL_STRING");return;
					break;
				case 0x98:
					ERROR("ToHexStringOp => OP_TO_HEX_STRING");return;
					break;
				case 0x99:
					ERROR("ToIntegerOp => OP_TO_INTEGER");return;
					break;
				case 0x9c:
					ERROR("ToStringOp => OP_TO_STRING");return;
					break;
				case 0x9d:
					ERROR("CopyObjectOp => OP_COPY_OBJECT");return;
					break;
				case 0x9e:
					ERROR("MidOp => OP_MID");return;
					break;
				case 0x9f:
					ERROR("ContinueOp => OP_CONTINUE");return;
					break;
				case 0xa0:
					ERROR("IfOp => OP_IF");return;
					break;
				case 0xa1:
					ERROR("ElseOp => OP_ELSE");return;
					break;
				case 0xa2:
					ERROR("WhileOp => OP_WHILE");return;
					break;
				case 0xa3:
					ERROR("NoopOp => OP_NOOP");return;
					break;
				case 0xa4:
					ERROR("ReturnOp => OP_RETURN");return;
					break;
				case 0xa5:
					ERROR("BreakOp => OP_BREAK");return;
					break;
				case 0xcc:
					ERROR("BreakPointOp => OP_BREAK_POINT");return;
					break;
				case 0xff:
					ERROR("OnesOp => OP_ONES");return;
					break;
			}
		}
		// 0x30-0x39 ('0'-'9')	DigitChar
		// 0x41-0x5A (‘A’-‘Z’)	NameChar
		else{
			offset++;
			switch (data[offset]){
				case 0x01:
					ERROR("MutexOp => OP_EXT_MUTEX");return;
					break;
				case 0x02:
					ERROR("EventOp => OP_EXT_EVENT");return;
					break;
				case 0x12:
					ERROR("CondRefOfOp => OP_EXT_COND_REF_OF");return;
					break;
				case 0x13:
					ERROR("CreateFieldOp => OP_EXT_CREATE_FIELD");return;
					break;
				case 0x1f:
					ERROR("LoadTableOp => OP_EXT_LOAD_TABLE");return;
					break;
				case 0x20:
					ERROR("LoadOp => OP_EXT_LOAD");return;
					break;
				case 0x21:
					ERROR("StallOp => OP_EXT_STALL");return;
					break;
				case 0x22:
					ERROR("SleepOp => OP_EXT_SLEEP");return;
					break;
				case 0x23:
					ERROR("AcquireOp => OP_EXT_ACQUIRE");return;
					break;
				case 0x24:
					ERROR("SignalOp => OP_EXT_SIGNAL");return;
					break;
				case 0x25:
					ERROR("WaitOp => OP_EXT_WAIT");return;
					break;
				case 0x26:
					ERROR("ResetOp => OP_EXT_RESET");return;
					break;
				case 0x27:
					ERROR("ReleaseOp => OP_EXT_RELEASE");return;
					break;
				case 0x28:
					ERROR("FromBCDOp => OP_EXT_FROM_BCD");return;
					break;
				case 0x29:
					ERROR("ToBCD => OP_EXT_TO_B");return;
					break;
				case 0x2a:
					ERROR("UnloadOp => OP_EXT_UNLOAD");return;
					break;
				case 0x30:
					ERROR("RevisionOp => OP_EXT_REVISION");return;
					break;
				case 0x31:
					ERROR("DebugOp => OP_EXT_DEBUG");return;
					break;
				case 0x32:
					ERROR("FatalOp => OP_EXT_FATAL");return;
					break;
				case 0x33:
					ERROR("TimerOp => OP_EXT_TIMER");return;
					break;
				case 0x80:
					ERROR("OpRegionOp => OP_EXT_OP_REGION");return;
					break;
				case 0x81:
					ERROR("FieldOp => OP_EXT_FIELD");return;
					break;
				case 0x82:
					ERROR("DeviceOp => OP_EXT_DEVICE");return;
					break;
				case 0x83:
					ERROR("ProcessorOp => OP_EXT_PROCESSOR");return;
					break;
				case 0x84:
					ERROR("PowerResOp => OP_EXT_POWER_RES");return;
					break;
				case 0x85:
					ERROR("ThermalZoneOp => OP_EXT_THERMAL_ZONE");return;
					break;
				case 0x86:
					ERROR("IndexFieldOp => OP_EXT_INDEX_FIELD");return;
					break;
				case 0x87:
					ERROR("BankFieldOp => OP_EXT_BANK_FIELD");return;
					break;
				case 0x88:
					ERROR("DataRegionOp => OP_EXT_DATA_REGION");return;
					break;
			}
		}
		offset++;
	}
}
