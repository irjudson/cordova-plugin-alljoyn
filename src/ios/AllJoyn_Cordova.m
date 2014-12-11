#define AJ_MODULE BASIC_CLIENT

#import "AllJoyn_Cordova.h"
#include <stdio.h>
#include <stdlib.h>
#include "aj_debug.h"
#include "alljoyn.h"
#include "aj_disco.h"

@implementation AllJoyn_Cordova

#define AJ_CONNECT_TIMEOUT 5000

static const char* const testInterface[] = {
    "org.allseen.LSF.LampService",
    "?my_ping inStr<s outStr>s",
    NULL
};

static const AJ_InterfaceDescription testInterfaces[] = {
    testInterface,
    NULL
};

static const AJ_Object AppObjects[] = {
    { "/org/allseen/LSF/Lamp", testInterfaces },
    { NULL }
};

/*
 Hacky beginning effort for initialize method exposed to JS
 Lots of bits hard coded.
 */
- (void)initialize:(CDVInvokedUrlCommand*)command
{
    //First deal with incoming parameters
    NSString* callbackId = [command callbackId];
    NSString* discoverServiceName = [[command arguments] objectAtIndex:0];
    
    AJ_Status status;
    AJ_Service service;
    AJ_BusAttachment bus;
    uint32_t timeout = AJ_CONNECT_TIMEOUT;
    
    CDVPluginResult* pluginResult = nil;
    NSMutableDictionary* discoveredDevices = [NSMutableDictionary dictionary];
    
    // Setup service
    service.addrTypes = AJ_ADDR_IPV4;
    
    // Initialize
    AJ_Initialize();
    AJ_RegisterObjects(AppObjects, NULL);
    
    // Discover
    status = AJ_Discover([discoverServiceName UTF8String], &service, timeout);
    
    if (status != AJ_OK) {
        // or retry discovery if status is aj_timeout probably...
        goto Fail;
    }
    
    // Connect
    memset(&bus, 0, sizeof(AJ_BusAttachment));
    
    status = AJ_Net_Connect(&bus.sock, service.ipv4port, service.addrTypes & AJ_ADDR_IPV4, &service.ipv4);
    if (status != AJ_OK) {
        // you would presumably keep retrying discovery if connect failed
        goto Fail;
    }
    
    // Return some fake dictionary of data for now
    [discoveredDevices setObject: @"some data" forKey: @"device1"];
    [discoveredDevices setObject: @"more data" forKey: @"device2"];

    
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsDictionary:discoveredDevices];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:callbackId];
    
Fail:
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:callbackId];
}

@end


