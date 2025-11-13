// Copyright (c) 2017-2025 Manuel Schneider

#include "mediaremote.h"
#include "plugin.h"
#include <Foundation/Foundation.h>
#include <mutex>
using namespace std;
#if  ! __has_feature(objc_arc)
#error This file must be compiled with ARC.
#endif

struct Plugin::Private {};

Plugin::Plugin() : d(make_unique<Private>())
{
    MRMediaRemoteRegisterForNowPlayingNotifications(dispatch_get_main_queue());

    auto updatePlayerBlock = ^(int pid) {
        scoped_lock lock(players_mtx_);
        players_.clear();
        if (pid != 0)
        {
            auto player = make_shared<Player>(pid);
            players_.emplace(player->name(), ::move(player));
        }
    };

    // Initialize player
    MRMediaRemoteGetNowPlayingApplicationPID(dispatch_get_main_queue(), updatePlayerBlock);

    // Watch player
    [[NSNotificationCenter defaultCenter]
        addObserverForName:(__bridge NSString*)kMRMediaRemoteNowPlayingApplicationDidChangeNotification
                    object:nil
                     queue:[NSOperationQueue mainQueue]
                usingBlock:^(NSNotification *note) {
                    int pid = [note.userInfo[(__bridge NSString*)kMRMediaRemoteNowPlayingApplicationPIDUserInfoKey]
                        intValue];
                    updatePlayerBlock(pid);
                }];
}

Plugin::~Plugin() {}

// void getClients()
// {
//     CRIT << "LIST CLIENTS";
//     // MRMediaRemoteGetNowPlayingClients(dispatch_get_main_queue(), ^(CFArrayRef cf_clients) {
//     //     CRIT << "LIST CLIENTS CALLED";
//     //     NSArray *clients = (__bridge NSArray *)cf_clients;
//     //     NSLog(@"Client: %@", clients);
//     //     for (id obj in clients) {
//     //         MRNowPlayingClientRef client = (__bridge MRNowPlayingClientRef)obj;
//     //         CFStringRef bundleID = MRNowPlayingClientGetBundleIdentifier(client);
//     //         NSLog(@"Bundle ID: %@", bundleID);
//     //         MRMediaRemoteSendCommandToClient(kMRPause, nil, nil, obj, 1, 0, nil);
//     //     }
//     // });
//     MRMediaRemoteGetAvailableOrigins(dispatch_get_main_queue(),
//                                      ^(CFArrayRef origins) {
//                                          NSLog(@"Bundle ID: %@", origins);
//                                      });
// }
