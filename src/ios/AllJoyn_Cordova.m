#define AJ_MODULE BASIC_CLIENT

#import "AllJoyn_Cordova.h"
#include <stdio.h>
#include <stdlib.h>
#include "aj_debug.h"
#include "alljoyn.h"

@implementation AllJoyn_Cordova

- (void)discover:(CDVInvokedUrlCommand*)command
{
    CDVPluginResult* pluginResult = nil;
    NSMutableDictionary* discoveredDevices = [NSMutableDictionary dictionary];
    
    [discoveredDevices setObject: @"some data" forKey: @"device1"];
    [discoveredDevices setObject: @"more data" forKey: @"device2"];
    
    if (true) {
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsDictionary:discoveredDevices];
    } else {
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR];
    }
    
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

@end


