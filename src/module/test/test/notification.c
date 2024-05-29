#include <kernel/log/log.h>
#include <kernel/notification/notification.h>
#include <kernel/types.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



static u64 _test_notification_object=NULL;
static u64 _test_notification_object2=NULL;
static u32 _test_notification_type=0;
static u32 _test_notification_type2=0;



static void _callback(u64 object,u32 type){
	_test_notification_object=object;
	_test_notification_type=type;
}



static void _callback2(u64 object,u32 type){
	_test_notification_object2=object;
	_test_notification_type2=type;
}



void test_notification(void){
	TEST_MODULE("notification");
	TEST_FUNC("notification_dispatcher_dispatch");
	TEST_GROUP("add");
	notification_dispatcher_t dispatcher;
	notification_dispatcher_init(&dispatcher);
	notification_dispatcher_add_listener(&dispatcher,_callback);
	notification_dispatcher_add_listener(&dispatcher,_callback2);
	_test_notification_object=NULL;
	_test_notification_object2=NULL;
	_test_notification_type=0;
	_test_notification_type2=0;
	notification_dispatcher_dispatch(&dispatcher,1234,5678);
	TEST_ASSERT(_test_notification_object==1234);
	TEST_ASSERT(_test_notification_object2==1234);
	TEST_ASSERT(_test_notification_type==5678);
	TEST_ASSERT(_test_notification_type2==5678);
	TEST_GROUP("remove");
	notification_dispatcher_remove_listener(&dispatcher,_callback2);
	_test_notification_object=NULL;
	_test_notification_object2=NULL;
	_test_notification_type=0;
	_test_notification_type2=0;
	notification_dispatcher_dispatch(&dispatcher,0xabcd,0x1234);
	TEST_ASSERT(_test_notification_object==0xabcd);
	TEST_ASSERT(!_test_notification_object2);
	TEST_ASSERT(_test_notification_type==0x1234);
	TEST_ASSERT(!_test_notification_type2);
	notification_dispatcher_deinit(&dispatcher);
}
