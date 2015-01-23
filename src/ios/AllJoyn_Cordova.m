#define AJ_MODULE BASIC_CLIENT

#import "AllJoyn_Cordova.h"
#include <stdio.h>
#include <stdlib.h>
#include "aj_debug.h"
#include "alljoyn.h"
#include "aj_disco.h"
#include "aj_config.h"


// Used to enable handling the msg responce for a method invocation
typedef bool (^MethodReplyHandler)(AJ_Message*);

#define MSG_TIMEOUT 1000 /* 1 sec */
#define METHOD_CALL_TIMEOUT 1000 /* 1 sec */

uint8_t dbgBASIC_CLIENT = 1;

@interface AllJoyn_Cordova  ()
// Dictionary of handlers keyed off of message id
// If a message handler exists for a given message id it will be
// invoked with a pointer to the message when it is being unmarshalled
@property NSMutableDictionary* methodReplyHandlers;
//Holds callback id used to communicate with success/error
// functions used to start the plugin (initialize call)
@property NSString* initializeCallbackId;
// Used for thread to handle msg loop and other AllJoyn communication
@property dispatch_queue_t dispatchQueue;
// used for triggering background thread activity
@property dispatch_source_t dispatchSource;
// Indicates if the app is connected to the bus or not
@property Boolean connectedToBus;

// Property to hold dynamically registered object lists
@property AJ_Object* proxyObjects;
@end

@implementation AllJoyn_Cordova

//TODO: Can this be a private property?
AJ_BusAttachment _bus;

-(void)connect:(CDVInvokedUrlCommand*)command {
    [self.commandDelegate runInBackground:^{
        if(![self connectedToBus]) {
            printf("Starting ...\n");
            AJ_Status status = AJ_OK;
            AJ_Initialize();

            printf("Starting the dispatcher\n");
            // Start the background task
            dispatch_resume([self dispatchSource]);

            printf("Connecting to the bus...\n");
            status = [self internalConnectBus:&_bus];
            if(status == AJ_OK) {
                [self setConnectedToBus:true];
                [self sendProgressMessage:@"Connected" toCallback:[command callbackId] withKeepCallback:false];
            } else {
                [self sendErrorMessage:@"Failed to connect" toCallback:[command callbackId] withKeepCallback:false];
            }
            printf("\n\nStarted!\n");
        }
    }];
}

-(void)registerObjects:(CDVInvokedUrlCommand*)command {
    [self.commandDelegate runInBackground:^{
        AJ_Initialize();
        NSArray* appObjects = [command argumentAtIndex:0];
        NSArray* proxyObjects = [command argumentAtIndex:1];

        AJ_Object* objectList = AJ_ObjectsCreate();
        for (NSDictionary* object in proxyObjects) {
            if([object isKindOfClass:[NSDictionary class]]) {
                AJ_Object newObject = {0};

                //setup object
                //newObject.flags = [[object objectForKey:@"flags"] unsignedIntValue];
                newObject.path = [[object objectForKey:@"path"] UTF8String];
                AJ_InterfaceDescription* interfaces = AJ_InterfacesCreate();

                for (NSArray* interface in [object objectForKey:@"interfaces"]) {
                    if([interface isKindOfClass:[NSArray class]]) {
                        char** ifaceMethods = NULL;
                        for(NSString* member in interface) {
                            if([member isKindOfClass:[NSString class]]) {

                                if(!ifaceMethods) {
                                    printf("InterfaceName: %s\n", [member UTF8String]);
                                    ifaceMethods = AJ_InterfaceDescriptionCreate([member UTF8String]);
                                } else {
                                    if([member length] > 0) {
                                        printf("Member: %s\n", [member UTF8String]);
                                        ifaceMethods = AJ_InterfaceDescriptionAdd(ifaceMethods, [member UTF8String]);
                                    }
                                }
                            }
                        }

                        interfaces = AJ_InterfacesAdd(interfaces, ifaceMethods);
                    }

                }
                newObject.interfaces = interfaces;
                objectList = AJ_ObjectsAdd(objectList, newObject);
            }
        }

        AJ_RegisterObjects(NULL, objectList);
        AJ_PrintXML(objectList);
        [self setProxyObjects:objectList];
        [self sendProgressMessage:@"Registered" toCallback:[command callbackId] withKeepCallback:false];
    }];
}

-(void)addAdvertisedNameListener:(CDVInvokedUrlCommand*)command {
    [self.commandDelegate runInBackground:^{
        NSString* name = [command argumentAtIndex:0];

        AJ_Status status = [self findService:&_bus withName:name];
        if(status != AJ_OK) {
            [self sendErrorMessage:@"Failure starting find" toCallback:[command callbackId] withKeepCallback:false];
            return;
        } else {
            NSNumber* methodKey = [NSNumber numberWithInt:AJ_SIGNAL_FOUND_ADV_NAME];
            MethodReplyHandler adjustVolumeHandler = ^bool(AJ_Message* pMsg) {

                AJ_Arg arg;
                AJ_UnmarshalArg(pMsg, &arg);
                AJ_InfoPrintf(("FoundAdvertisedName(%s)\n", arg.val.v_string));

                NSMutableDictionary* responseDictionary = [NSMutableDictionary new];

                [responseDictionary setObject:[NSString stringWithUTF8String:arg.val.v_string] forKey:@"name"];
                [responseDictionary setObject:[NSString stringWithUTF8String:pMsg->sender] forKey:@"sender"];
                [self sendProgressDictionary:responseDictionary toCallback:[command callbackId] withKeepCallback:false];
                [[self methodReplyHandlers] removeObjectForKey:methodKey];
                return true;
            };

            [[self methodReplyHandlers] setObject:adjustVolumeHandler forKey:methodKey];
        }
    }];
}

-(void)addInterfacesListener:(CDVInvokedUrlCommand*)command {
    [self.commandDelegate runInBackground:^{
        NSArray* interfaces = [command argumentAtIndex:0];
        AJ_Status status = [self askForAboutAnnouncements:&_bus forObjectsImplementing:interfaces];
        if(status != AJ_OK) {
            [self sendErrorMessage:@"Failure starting find" toCallback:[command callbackId] withKeepCallback:false];
        } else {


            NSNumber* methodKey = [NSNumber numberWithInt:AJ_SIGNAL_ABOUT_ANNOUNCE];
            MethodReplyHandler adjustVolumeHandler = ^bool(AJ_Message* pMsg) {
                uint16_t aboutVersion, aboutPort;
                AJ_UnmarshalArgs(pMsg, "qq", &aboutVersion, &aboutPort);
                AJ_InfoPrintf((" -- AboutVersion: %d, AboutPort: %d\n", aboutVersion, aboutPort));
                //TODO: Get more about info and convert to proper callback
                NSMutableDictionary* responseDictionary = [NSMutableDictionary new];
                [responseDictionary setObject:[NSNumber numberWithUnsignedInt:aboutVersion] forKey:@"version"];
                [responseDictionary setObject:[NSNumber numberWithUnsignedInt:aboutPort] forKey:@"port"];
                [responseDictionary setObject:[NSString stringWithUTF8String:pMsg->sender] forKey:@"name"];
                [self sendProgressDictionary:responseDictionary toCallback:[command callbackId] withKeepCallback:false];
                [[self methodReplyHandlers] removeObjectForKey:methodKey];
                return true;
            };

            [[self methodReplyHandlers] setObject:adjustVolumeHandler forKey:methodKey];
        }
    }];
}

-(void)joinSession:(CDVInvokedUrlCommand*)command {
    [self.commandDelegate runInBackground:^{
        printf("+joinSessionAsyc\n");
        AJ_Status status = AJ_OK;
        NSDictionary* server = [command argumentAtIndex:0];
        if(![server isKindOfClass:[NSDictionary class]]) {
            [self sendErrorMessage:@"JoinSession: Invalid Argument" toCallback:[command callbackId] withKeepCallback:false];
            return;
        }

        NSNumber* port = [server objectForKey:@"port"];
        NSString* name = [server objectForKey:@"name"];

        status = AJ_BusJoinSession(&_bus, [name UTF8String], [port intValue], NULL);
        if(status == AJ_OK) {
            NSNumber* methodKey = [NSNumber numberWithInt:AJ_REPLY_ID(AJ_METHOD_JOIN_SESSION)];
            MethodReplyHandler adjustVolumeHandler = ^bool(AJ_Message* pMsg) {

                AJ_InfoPrintf((" -- Got reply to JoinSession ---\n"));
                AJ_InfoPrintf(("MsgType: %d 0x%x\n", (*pMsg).hdr->msgType, (*pMsg).hdr->msgType));
                uint32_t replyCode;
                uint32_t sessionId;

                if ((*pMsg).hdr->msgType == AJ_MSG_ERROR) {
                    [self sendErrorMessage:@"Failure joining session MSG ERROR" toCallback:[command callbackId] withKeepCallback:false];
                } else {
                    AJ_UnmarshalArgs(pMsg, "uu", &replyCode, &sessionId);
                    if (replyCode == AJ_JOINSESSION_REPLY_SUCCESS) {
                        NSMutableArray* responseArray = [NSMutableArray new];
                        [responseArray addObject:[NSNumber numberWithUnsignedInt:sessionId]];
                        [responseArray addObject:name];
                        [self sendProgressArray:responseArray toCallback:[command callbackId] withKeepCallback:false];
                    } else {
                        [self sendErrorMessage:[NSString stringWithFormat:@"Failure joining session replyCode = 0x%x %d", replyCode, replyCode] toCallback:[command callbackId] withKeepCallback:false];
                    }
                }
                [[self methodReplyHandlers] removeObjectForKey:methodKey];
                return true;

            };

            [[self methodReplyHandlers] setObject:adjustVolumeHandler forKey:methodKey];

        } else {
            [self sendErrorMessage:[NSString stringWithFormat:@"Failed to iniitate join session: %x %d %s", status, status, AJ_StatusText(status)] toCallback:[command callbackId] withKeepCallback:false];
        }
    }];
}
-(void)invokeMember:(CDVInvokedUrlCommand*) command {
    //    exec(callMethodSuccess, callMethodError, "AllJoyn", "invokeMember", [sessionId, destination, signature, path, indexList, inParameterType, parameters, outParameterType]);
    [self.commandDelegate runInBackground:^{
        NSNumber* sessionId = [command argumentAtIndex:0];
        NSString* destination = [command argumentAtIndex:1];
        NSString* signature = [command argumentAtIndex:2];
        NSString* path = [command argumentAtIndex:3];
        NSArray* indexList = [command argumentAtIndex:4];
        NSString* parameterTypes = [command argumentAtIndex:5];
        NSArray* parameters = [command argumentAtIndex:6 withDefault:[NSArray new]];
        NSString* outParameterSignature = [command argumentAtIndex:7];

        AJ_Status status = AJ_OK;

        if( ![signature isKindOfClass:[NSString class]] ||
           ![path isKindOfClass:[NSString class]] ||
           ![indexList isKindOfClass:[NSArray class]]) {

            [self sendErrorMessage:@"inokeMember: Invalid Argument" toCallback:[command callbackId] withKeepCallback:false];
            return;
        }

        if([indexList count] < 4) {
            [self sendErrorMessage:@"invokeMember: Expected 3 indices in indexList" toCallback:[command callbackId] withKeepCallback:false];
            return;
        }
        NSNumber* listIndex = [indexList objectAtIndex:0];
        NSNumber* objectIndex = [indexList objectAtIndex:1];
        NSNumber* interfaceIndex = [indexList objectAtIndex:2];
        NSNumber* memberIndex = [indexList objectAtIndex:3];

        if( ![listIndex isKindOfClass:[NSNumber class]] ||
           ![objectIndex isKindOfClass:[NSNumber class]] ||
           ![interfaceIndex isKindOfClass:[NSNumber class]] ||
           ![memberIndex isKindOfClass:[NSNumber class]]) {
            [self sendErrorMessage:@"invokeMember: non-number index encountered" toCallback:[command callbackId] withKeepCallback:false];
        }
        //TODO: save app and prx objects and use listIndex to pick the right one
        AJ_Object* object = [self proxyObjects];
        uint counter = [objectIndex unsignedIntValue];
        while(counter) {
            if(!++object){
                [self sendErrorMessage:@"invokeMember: invalid object index" toCallback:[command callbackId] withKeepCallback:false];
                return;
            }
            counter--;
        }

        counter = [interfaceIndex unsignedIntValue];
        const AJ_InterfaceDescription* interface = object->interfaces;
        while(counter) {
            if(!++interface) {
                [self sendErrorMessage:@"invokeMember: invalid interface index" toCallback:[command callbackId] withKeepCallback:false];
                return;
            }
            counter--;
        }
        counter = [memberIndex unsignedIntValue];
        AJ_InterfaceDescription member = *(interface);
        member +=1;
        while(counter) {
            if(!++member) {
                [self sendErrorMessage:@"invokeMember: invalid member index" toCallback:[command callbackId] withKeepCallback:false];
                return;
            }
            counter--;
        }

        printf("Invoke Member %s\n", *member);
        uint32_t msgId = AJ_ENCODE_MESSAGE_ID(
                                              [listIndex unsignedIntValue],
                                              [objectIndex unsignedIntValue],
                                              [interfaceIndex unsignedIntValue],
                                              [memberIndex unsignedIntValue]);

        printf("Message id: %u\n", msgId);

        const char* memberSignature = NULL;//[signature UTF8String];
        uint8_t isSecure = 0;
        //        if(!memberSignature) {
        //            [self sendErrorMessage:@"invokeMember: Member signature was null" toCallback:[command callbackId] withKeepCallback:false];
        //            return;
        //        }

        AJ_MemberType memberType = AJ_GetMemberType(msgId, &memberSignature, &isSecure);

        AJ_Message msg;

        status = AJ_SetProxyObjectPath([self proxyObjects], msgId, [path UTF8String]);
        if(status != AJ_OK) {
            printf("AJ_SetProxyObjectPath failed with %s\n", AJ_StatusText(status));
            goto e_Exit;
        }

        printf("MemberType: %u, MemberSignature: %s, IsSecure %u\n", memberType, memberSignature, isSecure);
        switch(memberType) {
            case AJ_METHOD_MEMBER:
                status = AJ_MarshalMethodCall(&_bus, &msg, msgId, [destination UTF8String], [sessionId unsignedIntValue], 0, MSG_TIMEOUT);
                if(status != AJ_OK) {
                    printf("Failure marshalling method call");
                    goto e_Exit;
                }
                if(parameterTypes != nil && [parameterTypes length] > 0) {
                    [self marshalArgumentsFor:&msg withSignature:parameterTypes havingValues:parameters];
                }
                break;
            case AJ_SIGNAL_MEMBER:
                status = AJ_MarshalSignal(&_bus, &msg, msgId, [destination UTF8String], [sessionId unsignedIntValue], 0, MSG_TIMEOUT);
                if(status != AJ_OK) {
                    printf("AJ_MarshalSignal failed with %s\n", AJ_StatusText(status));
                    goto e_Exit;
                }

                if(parameterTypes != nil && [parameterTypes length] > 0) {
                    status = [self marshalArgumentsFor:&msg withSignature:parameterTypes havingValues:parameters];
                    if(status != AJ_OK) {
                        printf("Failure marshalling arguments: %s\n", AJ_StatusText(status));
                        goto e_Exit;
                    }
                }
                break;
            case AJ_PROPERTY_MEMBER:
                break;
            default:
                status = AJ_ERR_FAILURE;
                break;
        }

        if(AJ_OK == status) {
            status = AJ_DeliverMsg(&msg);


            NSNumber* methodKey = [NSNumber numberWithInt:AJ_REPLY_ID(msgId)];
            MethodReplyHandler methodReplyHandler = ^bool(AJ_Message* pMsg) {
                AJ_Status status;

                NSMutableDictionary* responseDictionary = [NSMutableDictionary new];
                NSMutableArray* outValues = [NSMutableArray new];

                if(outParameterSignature != nil && [outParameterSignature length] > 0) {
                    status = [self unmarshalArgumentsFor:pMsg withSignature:outParameterSignature toValues:outValues];
                }

                [responseDictionary setObject:@"invokeMember success" forKey:@"message"];
                [responseDictionary setObject:[NSString stringWithUTF8String:pMsg->sender] forKey:@"sender"];

                [responseDictionary setObject:outValues forKey:@"outValues"];
                [self sendProgressDictionary:responseDictionary toCallback:[command callbackId] withKeepCallback:false];
                [[self methodReplyHandlers] removeObjectForKey:methodKey];
                return true;
            };

            [[self methodReplyHandlers] setObject:methodReplyHandler forKey:methodKey];

        }


    e_Exit:
        if(status != AJ_OK) {
            [self sendErrorMessage:[NSString stringWithFormat:@"InvokeMember failure: %s", AJ_StatusText(status)] toCallback:[command callbackId] withKeepCallback:false];
        }
        return;

    }];
}

#define MAX_NESTED_CONTAINERS 10


-(AJ_Status)unmarshalArgumentsFor:(AJ_Message*)pMsg withSignature:(NSString*)signature toValues:(NSMutableArray*)values {
    AJ_Status status = AJ_OK;
    printf("unmarshalArgumentsFor: %s\n", [signature UTF8String]);

    unsigned long len = [signature length];

    AJ_Arg arg = {0};

    for(int i =0;i<len;i++) {
        char currentType = [signature UTF8String][i];
        //Reset arg to initial values
        arg.container = 0;
        arg.flags = 0;
        arg.len = 0;
        arg.sigPtr = 0;
        arg.val.v_data = NULL;

        switch(currentType) {
            case AJ_ARG_STRING: {
                status = AJ_UnmarshalArg(pMsg, &arg);
                NSString* stringArg = [NSString stringWithUTF8String:arg.val.v_string];
                [values addObject:stringArg];
                break;
            }
            default: {
                status = AJ_ERR_FAILURE;
                break;
            }
        }

        if(status != AJ_OK) {
            break;
        }
    }
e_Exit:
    return status;
}

-(AJ_Status)marshalArgumentsFor:(AJ_Message*)pMsg withSignature:(NSString*)signature havingValues:(NSArray*)values {

    printf("marshalArgumentsFor %s\n", [signature UTF8String]);
    AJ_Status status = AJ_OK;
    if(!pMsg) {
        status = AJ_ERR_INVALID;
        return status;
    }

    unsigned long len = [signature length];
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    double d;


    unsigned int currentParameter = 0;

    AJ_Arg arg = {0};
    AJ_Arg containerArgs[MAX_NESTED_CONTAINERS];
    unsigned int nextContainerArgIndex = 0;
    for(int i = 0; i < len; ++i) {
        char current = [signature UTF8String][i];
        //Reset arg to initial values
        arg.container = 0;
        arg.flags = 0;
        arg.len = 0;
        arg.sigPtr = 0;
        arg.val.v_data = NULL;
        arg.typeId = (uint8_t)current;


        //        /*
        //         * Message argument types
        //         */
        //#define AJ_ARG_INVALID           '\0'   /**< AllJoyn invalid type */
        //#define AJ_ARG_ARRAY             'a'    /**< AllJoyn array container type */
        //#define AJ_ARG_BOOLEAN           'b'    /**< AllJoyn boolean basic type */
        //#define AJ_ARG_DOUBLE            'd'    /**< AllJoyn IEEE 754 double basic type */
        //#define AJ_ARG_SIGNATURE         'g'    /**< AllJoyn signature basic type */
        //#define AJ_ARG_HANDLE            'h'    /**< AllJoyn socket handle basic type */
        //#define AJ_ARG_INT32             'i'    /**< AllJoyn 32-bit signed integer basic type */
        //#define AJ_ARG_INT16             'n'    /**< AllJoyn 16-bit signed integer basic type */
        //#define AJ_ARG_OBJ_PATH          'o'    /**< AllJoyn Name of an AllJoyn object instance basic type */
        //#define AJ_ARG_UINT16            'q'    /**< AllJoyn 16-bit unsigned integer basic type */
        //#define AJ_ARG_STRING            's'    /**< AllJoyn UTF-8 NULL terminated string basic type */
        //#define AJ_ARG_UINT64            't'    /**< AllJoyn 64-bit unsigned integer basic type */
        //#define AJ_ARG_UINT32            'u'    /**< AllJoyn 32-bit unsigned integer basic type */
        //#define AJ_ARG_VARIANT           'v'    /**< AllJoyn variant container type */
        //#define AJ_ARG_INT64             'x'    /**< AllJoyn 64-bit signed integer basic type */
        //#define AJ_ARG_BYTE              'y'    /**< AllJoyn 8-bit unsigned integer basic type */
        //#define AJ_ARG_STRUCT            '('    /**< AllJoyn struct container type */
        //#define AJ_ARG_DICT_ENTRY        '{'    /**< AllJoyn dictionary or map container type - an array of key-value pairs */

        switch (current) {

            case AJ_ARG_BOOLEAN:
                u32 = [[values objectAtIndex:currentParameter++] unsignedIntValue];
                arg.val.v_bool = &u32;
                break;
            case AJ_ARG_INT16:
                i16 = [[values objectAtIndex:currentParameter++] shortValue];
                arg.val.v_int16 = &i16;
                break;
            case AJ_ARG_INT32:
                i32 = [[values objectAtIndex:currentParameter++] intValue];
                arg.val.v_int32 = &i32;
                break;
            case AJ_ARG_INT64:
                i64 = [[values objectAtIndex:currentParameter++] longLongValue];
                arg.val.v_int64 = &i64;
                break;
            case AJ_ARG_UINT16:
                u16 = [[values objectAtIndex:currentParameter++] unsignedShortValue];
                arg.val.v_uint16 = &u16;
                break;
            case AJ_ARG_UINT32:
                u32 = [[values objectAtIndex:currentParameter++] unsignedIntValue];
                arg.val.v_uint32 = &u32;
                break;
            case AJ_ARG_UINT64:
                u64 = [[values objectAtIndex:currentParameter++] unsignedLongLongValue];
                arg.val.v_uint64 = &u64;
                break;
            case AJ_ARG_DOUBLE:
                d = [[values objectAtIndexedSubscript:currentParameter++] doubleValue];
                arg.val.v_double = &d;
                break;
            case AJ_ARG_BYTE:
                u8 = [[values objectAtIndexedSubscript:currentParameter++] charValue];
                arg.val.v_byte = &u8;
                break;
            case AJ_ARG_STRING:
                arg.val.v_string = [[values objectAtIndexedSubscript:currentParameter++] UTF8String];

                printf("string %s\n", arg.val.v_string);
                break;
            case AJ_ARG_ARRAY:
            case AJ_ARG_DICT_ENTRY:
            case AJ_ARG_HANDLE:
            case AJ_ARG_INVALID:
                status = AJ_ERR_INVALID;
                break;
            case AJ_ARG_OBJ_PATH:
            case AJ_ARG_SIGNATURE:
            case AJ_ARG_STRUCT:

            case AJ_ARG_VARIANT:
                status = AJ_ERR_REJECTED;
                break;

            default:
                status = AJ_ERR_UNKNOWN;
                break;
        }

        if(status == AJ_OK) {
            status = AJ_MarshalArg(pMsg, &arg);
        } else {
            break;
        }
    }

e_Exit:
    return status;
}

-(void)testMarshal:(CDVInvokedUrlCommand*)command {
    [self.commandDelegate runInBackground:^{
        NSArray* args = [command arguments];
        NSArray* objects = [command argumentAtIndex:0];
        NSNumber* objectAsNumber = [command argumentAtIndex:0];

        if([objectAsNumber isKindOfClass:[NSNumber class]]) {
            printf("objectAsNumber: 0x%llx\n", [objectAsNumber unsignedLongLongValue]);
        }
        int i = 0;
        for (id arg in args) {
            printf("%s\n",[[NSString stringWithFormat:@"Arg %d is %@\n", i++, arg] UTF8String]);
        }

        i = 0;

        unsigned long objectCount = [objects count];

        printf("Object Count: %lu\n", objectCount);

        AJ_Object* objectList = AJ_ObjectsCreate();

        for (NSDictionary* object in objects) {
            AJ_Object newObject = {0};

            //setup object
            newObject.flags = [[object objectForKey:@"flags"] unsignedIntValue];
            newObject.path = [[object objectForKey:@"path"] UTF8String];
            AJ_InterfaceDescription* interfaces = AJ_InterfacesCreate();
            for (NSDictionary* interface in [object objectForKey:@"interfaces"]) {
                printf("Interface %s\n", [[interface objectForKey:@"name"] UTF8String]);
                char** ifaceMethods = AJ_InterfaceDescriptionCreate([[interface objectForKey:@"name"] UTF8String]);
                for(NSString* method in [interface objectForKey:@"methods"]) {
                    printf("Method: %s\n", [method UTF8String]);
                    ifaceMethods = AJ_InterfaceDescriptionAdd(ifaceMethods, [method UTF8String]);
                }
                interfaces = AJ_InterfacesAdd(interfaces, ifaceMethods);
            }
            newObject.interfaces = interfaces;
            objectList = AJ_ObjectsAdd(objectList, newObject);

            printf("%s\n",[[NSString stringWithFormat:@"Inner %d is %@\n", i++, object] UTF8String]);
        }

        [self.commandDelegate sendPluginResult:[CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsMultipart:[NSArray arrayWithObject:[NSNumber numberWithUnsignedLongLong:objectList]]] callbackId:[command callbackId]];
    }];
}

// Constructor for plugin class
- (CDVPlugin*)initWithWebView:(UIWebView*)theWebView {
    self = [super initWithWebView:theWebView];

    AJ_InfoPrintf((" -- AllJoyn Plugin Class Init\n"));

    _connectedToBus = false;
    memset(&_bus, 0, sizeof(AJ_BusAttachment));
    // Create a dispatcher for background tasks (msg loop, msg sending, etc. )
    _dispatchQueue = dispatch_queue_create("org.cordova.plugin.AllJoyn", NULL);
    // Dictionary for method reply handlers
    _methodReplyHandlers = [NSMutableDictionary dictionaryWithObjectsAndKeys: nil];

    _proxyObjects = NULL;

    [self createDispatcherTimer];
    return self;
}

// Creates a timer but does NOT start it
dispatch_source_t CreateDispatchTimer(uint64_t interval,
                                      uint64_t leeway,
                                      dispatch_queue_t queue,
                                      dispatch_block_t block) {

    dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
                                                     0, 0, queue);
    if (timer)
    {
        dispatch_source_set_timer(timer, dispatch_time(DISPATCH_TIME_NOW, interval), interval, leeway);
        dispatch_source_set_event_handler(timer, block);
    }
    return timer;
}

// create a timer and set it to the property
-(void)createDispatcherTimer {
    dispatch_source_t aTimer = CreateDispatchTimer(1ull * NSEC_PER_SEC/10,
                                                   1ull * NSEC_PER_SEC /100,
                                                   _dispatchQueue,
                                                   ^{
                                                       [self msgLoop];
                                                   });
    // Store it somewhere for later use.
    if (aTimer)
    {
        [self setDispatchSource:aTimer];
    }
}

//- (void) joinSession:(CDVInvokedUrlCommand*)command {
//    printf("+joinSession\n");
//    dispatch_async([self dispatchQueue], ^{
//        printf("+joinSessionAsyc\n");
//        AJ_Status status = AJ_OK;
//        NSNumber* port = [command argumentAtIndex:0];
//        NSString* serviceName = [command argumentAtIndex:1];
//        NSString* callbackId = [command callbackId];
//
//        status = AJ_BusJoinSession(&_bus, [serviceName UTF8String], [port intValue], NULL);
//        if(status == AJ_OK) {
//            NSNumber* methodKey = [NSNumber numberWithInt:AJ_REPLY_ID(AJ_METHOD_JOIN_SESSION)];
//            MethodReplyHandler adjustVolumeHandler = ^bool(AJ_Message* pMsg) {
//
//                AJ_InfoPrintf((" -- Got reply to JoinSession ---\n"));
//                AJ_InfoPrintf(("MsgType: %d 0x%x\n", (*pMsg).hdr->msgType, (*pMsg).hdr->msgType));
//                uint32_t replyCode;
//                uint32_t sessionId;
//
//                if ((*pMsg).hdr->msgType == AJ_MSG_ERROR) {
//                    [self sendErrorMessage:@"Failure joining sessoin MSG ERROR" toCallback:callbackId withKeepCallback:false];
//                } else {
//                    AJ_UnmarshalArgs(pMsg, "uu", &replyCode, &sessionId);
//                    if (replyCode == AJ_JOINSESSION_REPLY_SUCCESS) {
//                        [self sendProgressMessage:[NSString stringWithFormat:@"JoinSession success SessionId %u Sender: %s", sessionId, [serviceName UTF8String]] toCallback:callbackId withKeepCallback:false];
//                    } else {
//                        [self sendErrorMessage:[NSString stringWithFormat:@"Failure joining session replyCode = 0x%x %d", replyCode, replyCode] toCallback:callbackId withKeepCallback:false];
//                    }
//                }
//
//                [[self methodReplyHandlers] removeObjectForKey:methodKey];
//                return true;
//
//            };
//
//            [[self methodReplyHandlers] setObject:adjustVolumeHandler forKey:methodKey];
//
//        } else {
//            [self sendErrorMessage:[NSString stringWithFormat:@"Failed to iniitate join session: %x %d %s", status, status, AJ_StatusText(status)] toCallback:callbackId withKeepCallback:false];
//        }
//
//        printf("-joinSessionAsync\n");
//    });
//    printf("-joinSession\n");
//}

- (void) disconnect:(CDVInvokedUrlCommand*) command {
    AJ_Disconnect(&_bus);
    [self setConnectedToBus:false];
    memset(&_bus, 0, sizeof(AJ_BusAttachment));

    // Stop background tasks
    dispatch_suspend([self dispatchSource]);
    [self sendProgressMessage:@"Disconnected" toCallback:[command callbackId] withKeepCallback:false];
}

-(void) sendProgressArray:(NSArray*)array toCallback:(NSString*)callbackId withKeepCallback:(Boolean)keepCallback {
    CDVPluginResult* pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsArray:array];
    [pluginResult setKeepCallbackAsBool:keepCallback];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:callbackId];

}

-(void) sendProgressDictionary:(NSDictionary*)dictionary toCallback:(NSString*)callbackId withKeepCallback:(Boolean)keepCallback {
    CDVPluginResult* pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsDictionary:dictionary];
    [pluginResult setKeepCallbackAsBool:keepCallback];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:callbackId];

}

-(void) sendProgressMessage:(NSString*)message toCallback:(NSString*) callbackId withKeepCallback:(Boolean)keepCallback {
    printf("SENDING: %s\n", [message UTF8String]);
    CDVPluginResult* pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsString:message];
    [pluginResult setKeepCallbackAsBool:keepCallback];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:callbackId];
}

-(void)sendProgressMessage:(NSString*)message {
    [self sendProgressMessage:message toCallback:[self initializeCallbackId] withKeepCallback:TRUE];
}

-(void)sendErrorMessage:(NSString*)message {
    [self sendErrorMessage:message toCallback:[self initializeCallbackId] withKeepCallback:TRUE];
}

-(void)sendErrorMessage:(NSString*)message toCallback:(NSString*) callbackId withKeepCallback:(Boolean)keepCallback {
    printf("SENDING ERROR: %s\n", [message UTF8String]);
    CDVPluginResult* pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:message];
    [pluginResult setKeepCallbackAsBool:keepCallback];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:[self initializeCallbackId]];
}

// After this method we should have bus
-(AJ_Status)internalConnectBus:(AJ_BusAttachment*) bus
{
    AJ_Status status = AJ_OK;

    status = AJ_FindBusAndConnect(bus, NULL, AJ_CONNECT_TIMEOUT);

    if (status != AJ_OK) {
        printf("Failed to AJ_FindAndConnect 0x%x %d %s\n", status, status, AJ_StatusText(status));
        goto Fail;
    }
Fail:
    return status;
}

-(AJ_Status)findService:(AJ_BusAttachment*)bus withName:(NSString*)serviceName {
    AJ_Status status = AJ_OK;
    status = AJ_BusFindAdvertisedName(bus, [serviceName UTF8String], AJ_BUS_START_FINDING);
    AJ_InfoPrintf(("AJ_StartClient(): AJ_BusFindAdvertisedName() %s\n", AJ_StatusText(status)));
    return status;
}

-(AJ_Status)askForAboutAnnouncements:(AJ_BusAttachment*)bus forObjectsImplementing:(NSArray*) interfaces
{
    AJ_Status status = AJ_OK;
    size_t ruleLen = 0;
    const char* base = "interface='org.alljoyn.About',sessionless='t'";
    const char* impl = ",implements='";
    char* rule = NULL;

    /*
     * Kick things off by registering for the Announce signal.
     * Optionally add the implements clause per given interface
     */
    ruleLen = strlen(base) + 1;
    if (interfaces != nil) {
        for(id interface in interfaces)
        {
            if([interface isKindOfClass:[NSString class]]) {
                ruleLen += strlen(impl) + strlen([interface UTF8String]);
            }

        }
    }
    rule = (char*) AJ_Malloc(ruleLen);
    if (rule == NULL) {
        status = AJ_ERR_RESOURCES;
        goto Fail;
    }
    strcpy(rule, base);
    if (interfaces != nil) {
        for(id interface in interfaces)
        {
            if([interface isKindOfClass:[NSString class]])
            {
                const char* currentIface = [interface UTF8String];
                strcat(rule, impl);
                if ((currentIface)[0] == '$') {
                    strcat(rule, &(currentIface)[1]);
                } else {
                    strcat(rule, currentIface);
                }
                strcat(rule, "'");
            }
        }
    }
    status = AJ_BusSetSignalRule(bus, rule, AJ_BUS_SIGNAL_ALLOW);
    AJ_InfoPrintf(("AJ_StartClient(): Client SetSignalRule: %s\n", rule));
Fail:
    if(rule)
    {
        AJ_Free(rule);
    }
    return status;
}

-(void)msgLoop
{
    AJ_InfoPrintf((" --- MSG LOOP ---\n"));
    AJ_InfoPrintf((" methodReplyHandlerCount: %lu\n", (unsigned long)[[self methodReplyHandlers] count]));
    if(![self connectedToBus]) {
        return;
    }

    AJ_Status status = AJ_OK;
    int i = 0;
    while (status == AJ_OK && (i++ < 1)) {
        AJ_InfoPrintf((" --- While ---\n"));
        AJ_Message msg;
        // get next message
        status = AJ_UnmarshalMsg(&_bus, &msg, MSG_TIMEOUT);

        // Check for errors we can ignore
        if(status == AJ_ERR_TIMEOUT) {
            // Nothing to do for now, continue i guess
            AJ_InfoPrintf(("Timeout getting MSG. Will try again...\n"));
            status = AJ_OK;
            continue;
        }
        if (status == AJ_ERR_NO_MATCH) {
            AJ_InfoPrintf(("AJ_ERR_NO_MATCH in main loop. Ignoring!\n"));
            // Ignore unknown messages
            status = AJ_OK;
            continue;
        }

        // If not ignored fail.
        if (status != AJ_OK) {
            AJ_ErrPrintf((" -- MainLoopError AJ_UnmarshalMsg returned status=%s\n", AJ_StatusText(status)));
            break;
        }
        AJ_InfoPrintf((" Executing handlers if any ... \n"));
        // If somebody has requested a handler for a specific msg
        NSNumber* msgIdAsNumber = [NSNumber numberWithInt:msg.msgId];
        MethodReplyHandler handler = [self methodReplyHandlers][msgIdAsNumber];
        bool handled = false;
        if (handler != nil && handler != NULL) {
            handled = handler(&msg);
        }
        if(!handled) {
            AJ_InfoPrintf((" Done Executing handlers if any ... \n"));
            switch (msg.msgId) {
                    //
                    // Method reples
                    //
                    //TODO: See if this is still needed
                case AJ_REPLY_ID(AJ_METHOD_JOIN_SESSION):
                    break;

                case AJ_REPLY_ID(AJ_METHOD_FIND_NAME):
                case AJ_REPLY_ID(AJ_METHOD_FIND_NAME_BY_TRANSPORT):
                {
                    if (msg.hdr->msgType == AJ_MSG_ERROR) {
                        AJ_ErrPrintf(("AJ_StartClient(): AJ_METHOD_FIND_NAME: %s\n", msg.error));
                        status = AJ_ERR_FAILURE;
                    } else {
                        uint32_t disposition;
                        AJ_UnmarshalArgs(&msg, "u", &disposition);
                        if ((disposition != AJ_FIND_NAME_STARTED) && (disposition != AJ_FIND_NAME_ALREADY)) {
                            AJ_ErrPrintf(("AJ_StartClient(): AJ_ERR_FAILURE\n"));
                            status = AJ_ERR_FAILURE;
                        }
                    }
                }
                    break;

                case AJ_SIGNAL_FOUND_ADV_NAME:
                {
                    AJ_Arg arg;
                    AJ_UnmarshalArg(&msg, &arg);
                    AJ_InfoPrintf(("FoundAdvertisedName(%s)\n", arg.val.v_string));

                    //                if (!found) {
                    //                    if (fullName) {
                    //                        strncpy(fullName, arg.val.v_string, arg.len);
                    //                        fullName[arg.len] = '\0';
                    //                    }
                    //                    found = TRUE;
                    //                    status = AJ_BusJoinSession(bus, arg.val.v_string, port, opts);
                    //                }

                    [self sendProgressMessage:[NSString stringWithFormat:@"AboutAnnouncement: %d %d %s", 1, 25, arg.val.v_string]];

                }
                    break;
                case AJ_SIGNAL_ABOUT_ANNOUNCE:
                {
                    uint16_t aboutVersion, aboutPort;
                    //#ifdef ANNOUNCE_BASED_DISCOVERY
                    //                status = AJ_AboutHandleAnnounce(&msg, &aboutVersion, &aboutPort, serviceName, &found);
                    //                if (interfaces != NULL) {
                    //                    found = TRUE;
                    //                }
                    //                if ((status == AJ_OK) && (found == TRUE)) {
                    //                    AJ_InfoPrintf(("AJ_StartClient(): AboutAnnounce from (%s) About Version: %d Port: %d\n", msg.sender, aboutVersion, aboutPort));
                    //#else
                    AJ_InfoPrintf(("AJ_StartClient(): AboutAnnounce from (%s)\n", msg.sender));
                    //                    if (!found) {
                    //                        found = TRUE;
                    AJ_UnmarshalArgs(&msg, "qq", &aboutVersion, &aboutPort);



                    //                AJ_AboutRegisterAnnounceHandlers(&pingServicePeer, 1);
                    //                AJ_AboutHandleAnnounce(&msg, &aboutVersion, &aboutPort, peerName, NULL);
                    AJ_InfoPrintf((" -- AboutVersion: %d, AboutPort: %d\n", aboutVersion, aboutPort));
                    //TODO: Get more about info and convert to proper callback
                    [self sendProgressMessage:[NSString stringWithFormat:@"AboutAnnouncement: %d %d %s", aboutVersion, aboutPort, msg.sender]];
                    //AJ_AboutUnmarshalProps(&msg, handleMandatoryProps, NULL);
                    //                        if (serviceName != NULL) {
                    //                            strncpy(serviceName, msg.sender, AJ_MAX_NAME_SIZE);
                    //                            serviceName[AJ_MAX_NAME_SIZE] = '\0';
                    //                        }
                    //#endif
                    /*
                     * Establish a session with the provided port.
                     * If port value is 0 use the About port unmarshalled from the Announcement instead.
                     */
                    //                        if (port == 0) {
                    //TODO: Make sure it works w/o this line
                    //status = AJ_BusJoinSession(&_bus, msg.sender, aboutPort, NULL);
                    //                        } else {
                    //                            status = AJ_BusJoinSession(bus, msg.sender, port, opts);
                    //                        }
                    //                        if (status != AJ_OK) {
                    //                            AJ_ErrPrintf(("AJ_StartClient(): BusJoinSession failed (%s)\n", AJ_StatusText(status)));
                    //                        }
                    //                    }
                }
                    break;

                case AJ_SIGNAL_SESSION_LOST_WITH_REASON:
                    /*
                     * Force a disconnect
                     */
                {
                    uint32_t id, reason;
                    AJ_UnmarshalArgs(&msg, "uu", &id, &reason);
                    AJ_InfoPrintf(("Session lost. ID = %u, reason = %u", id, reason));
                    [self sendProgressMessage:@"lost session :("];
                }
                    AJ_ErrPrintf((" -- (): AJ_SIGNAL_SESSION_LOST_WITH_REASON: AJ_ERR_READ\n"));
                    status = AJ_ERR_READ;
                    break;
                    
                default:
                    /*
                     * Pass to the built-in bus message handlers
                     */
                    AJ_InfoPrintf((" -- (): AJ_BusHandleBusMessage()\n"));
                    status = AJ_BusHandleBusMessage(&msg);
                    break;
            }
        }
        AJ_CloseMsg(&msg);
    }
    if(status != AJ_OK) {
        printf("ERROR: Main loop had a non-succesful iteration. Exit status: %d 0x%x %s", status, status, AJ_StatusText(status));
        [self sendErrorMessage:[NSString stringWithFormat:@"Error encountered: %d 0x%x %s", status, status, AJ_StatusText(status)]];
    }
}

@end
