// Copyright (c) 2017-2025 Manuel Schneider

#include "mediaremote.h"
#include "player.h"
#include <AppKit/AppKit.h>
#include <albert/iconutil.h>
#include <albert/logging.h>
#if  ! __has_feature(objc_arc)
#error This file must be compiled with ARC.
#endif

Player::Player(int pid)
{
    auto *app = [NSRunningApplication runningApplicationWithProcessIdentifier:pid];

    if (!app) {
        WARN << "Failed to get running application with PID:" << pid;
        return;
    }

    name_ = QString::fromNSString(app.localizedName);
    bundle_path_ = QString::fromNSString(app.bundleURL.path);

    // Initialize is_playing_
    MRMediaRemoteGetNowPlayingApplicationIsPlaying(dispatch_get_main_queue(),
                                                   ^(Boolean isPlaying) { is_playing_ = isPlaying; });

    // Watch is_playing_
    notification_observation_ = [[NSNotificationCenter defaultCenter]
        addObserverForName:(__bridge NSString*)kMRMediaRemoteNowPlayingApplicationIsPlayingDidChangeNotification
                    object:nil
                     queue:[NSOperationQueue mainQueue]
                usingBlock:^(NSNotification *note) {
                    is_playing_ =
                        [note.userInfo[(__bridge NSString*)kMRMediaRemoteNowPlayingApplicationIsPlayingUserInfoKey]
                        boolValue];
                }];
}

Player::~Player() { [[NSNotificationCenter defaultCenter] removeObserver:notification_observation_]; }

QString Player::name() const { return name_; }

std::unique_ptr<albert::Icon> Player::icon() { return albert::makeFileTypeIcon(bundle_path_); }

bool Player::isPlaying() const { return is_playing_; }

bool Player::canPlay() const { return true; }

bool Player::canPause() const { return true; }

bool Player::canGoNext() const { return true; }

bool Player::canGoPrevious() const { return true; }

void Player::play() { MRMediaRemoteSendCommand(kMRPlay, nil); }

void Player::pause() { MRMediaRemoteSendCommand(kMRPause, nil); }

void Player::next() { MRMediaRemoteSendCommand(kMRNextTrack, nil); }

void Player::previous() { MRMediaRemoteSendCommand(kMRPreviousTrack, nil); }

// // Initialize info…
// const auto getNowPlayingBlock = ^(CFDictionaryRef information) {
//     if (information)
//     {
//         const auto dict = (__bridge NSDictionary *)information;

//         for (::id key in dict)
//             NSLog(@"%@: %@", key, dict[key]);

//         NSString *title = dict[(__bridge NSString *)kMRMediaRemoteNowPlayingInfoTitle];
//         NSString *artist = dict[(__bridge NSString *)kMRMediaRemoteNowPlayingInfoArtist];
//         // NSString *album = dict[(__bridge NSString *)kMRMediaRemoteNowPlayingInfoAlbum];
//         now_playing_info_ = QStringLiteral("%1 · %2")
//                                 .arg(QString::fromNSString(title),
//                                      QString::fromNSString(artist));
//         // QString::fromNSString(album));
//         DEBG << "Now playing info:" << now_playing_info_;
//     }
// };

// MRMediaRemoteGetNowPlayingInfo(dispatch_get_main_queue(), getNowPlayingBlock);

// // Watch info
// [[NSNotificationCenter defaultCenter]
//     addObserverForName:(__bridge NSString*)kMRMediaRemoteNowPlayingInfoDidChangeNotification
//                 object:nil
//                  queue:[NSOperationQueue mainQueue]
//             usingBlock:^(NSNotification *) {
//                 // Suboptimal but the less private API touched the better.
//                 MRMediaRemoteGetNowPlayingInfo(dispatch_get_main_queue(), getNowPlayingBlock);
//             }];
