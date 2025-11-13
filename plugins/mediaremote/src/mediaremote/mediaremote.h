// Copyright (c) 2017-2025 Manuel Schneider

// https://github.com/khangthk/SketchyBar/blob/5b7d0e07cf41ae8413393b1bb8f5c184a6615695/src/media.m
// https://github.com/khangthk/SketchyBar/blob/master/src/media.m
// https://github.com/theos/headers/blob/master/MediaRemote/MediaRemote.h

#pragma once
#include <CoreFoundation/CoreFoundation.h>

extern "C" {

extern CFStringRef kMRActiveNowPlayingPlayerPathUserInfoKey;
extern CFStringRef kMRMediaRemoteNowPlayingApplicationDidChangeNotification;
extern CFStringRef kMRMediaRemoteNowPlayingApplicationDisplayNameUserInfoKey;
extern CFStringRef kMRMediaRemoteNowPlayingApplicationIsPlayingDidChangeNotification;
extern CFStringRef kMRMediaRemoteNowPlayingApplicationIsPlayingUserInfoKey;
extern CFStringRef kMRMediaRemoteNowPlayingApplicationPIDUserInfoKey;
extern CFStringRef kMRMediaRemoteNowPlayingClientsDidChangeNotification;
extern CFStringRef kMRMediaRemoteNowPlayingClientsDidChangeNotification;
extern CFStringRef kMRMediaRemoteNowPlayingInfoAlbum;
extern CFStringRef kMRMediaRemoteNowPlayingInfoAlbumiTunesStoreAdamIdentifier;
extern CFStringRef kMRMediaRemoteNowPlayingInfoArtist;
extern CFStringRef kMRMediaRemoteNowPlayingInfoArtistiTunesStoreAdamIdentifier;
extern CFStringRef kMRMediaRemoteNowPlayingInfoArtworkData;
extern CFStringRef kMRMediaRemoteNowPlayingInfoArtworkIdentifier;
extern CFStringRef kMRMediaRemoteNowPlayingInfoArtworkMIMEType;
extern CFStringRef kMRMediaRemoteNowPlayingInfoChapterNumber;
extern CFStringRef kMRMediaRemoteNowPlayingInfoClientPropertiesData;
extern CFStringRef kMRMediaRemoteNowPlayingInfoComposer;
extern CFStringRef kMRMediaRemoteNowPlayingInfoContentItemIdentifier;
extern CFStringRef kMRMediaRemoteNowPlayingInfoDidChangeNotification;
extern CFStringRef kMRMediaRemoteNowPlayingInfoDuration;
extern CFStringRef kMRMediaRemoteNowPlayingInfoElapsedTime;
extern CFStringRef kMRMediaRemoteNowPlayingInfoGenre;
extern CFStringRef kMRMediaRemoteNowPlayingInfoIsAdvertisement;
extern CFStringRef kMRMediaRemoteNowPlayingInfoIsBanned;
extern CFStringRef kMRMediaRemoteNowPlayingInfoIsInWishList;
extern CFStringRef kMRMediaRemoteNowPlayingInfoIsLiked;
extern CFStringRef kMRMediaRemoteNowPlayingInfoIsMusicApp;
extern CFStringRef kMRMediaRemoteNowPlayingInfoMediaType;
extern CFStringRef kMRMediaRemoteNowPlayingInfoPlaybackRate;
extern CFStringRef kMRMediaRemoteNowPlayingInfoProhibitsSkip;
extern CFStringRef kMRMediaRemoteNowPlayingInfoQueueIndex;
extern CFStringRef kMRMediaRemoteNowPlayingInfoRadioStationHash;
extern CFStringRef kMRMediaRemoteNowPlayingInfoRadioStationIdentifier;
extern CFStringRef kMRMediaRemoteNowPlayingInfoRepeatMode;
extern CFStringRef kMRMediaRemoteNowPlayingInfoShuffleMode;
extern CFStringRef kMRMediaRemoteNowPlayingInfoStartTime;
extern CFStringRef kMRMediaRemoteNowPlayingInfoSupportsFastForward15Seconds;
extern CFStringRef kMRMediaRemoteNowPlayingInfoSupportsIsBanned;
extern CFStringRef kMRMediaRemoteNowPlayingInfoSupportsIsLiked;
extern CFStringRef kMRMediaRemoteNowPlayingInfoSupportsRewind15Seconds;
extern CFStringRef kMRMediaRemoteNowPlayingInfoTimestamp;
extern CFStringRef kMRMediaRemoteNowPlayingInfoTitle;
extern CFStringRef kMRMediaRemoteNowPlayingInfoTotalChapterCount;
extern CFStringRef kMRMediaRemoteNowPlayingInfoTotalDiscCount;
extern CFStringRef kMRMediaRemoteNowPlayingInfoTotalQueueCount;
extern CFStringRef kMRMediaRemoteNowPlayingInfoTotalTrackCount;
extern CFStringRef kMRMediaRemoteNowPlayingInfoTrackNumber;
extern CFStringRef kMRMediaRemoteNowPlayingInfoUniqueIdentifier;
extern CFStringRef kMRMediaRemoteNowPlayingInfoiTunesStoreIdentifier;
extern CFStringRef kMRMediaRemoteNowPlayingInfoiTunesStoreSubscriptionAdamIdentifier;
extern CFStringRef kMRMediaRemoteNowPlayingPlaybackQueueDidChangeNotification;
extern CFStringRef kMRMediaRemoteOptionMediaType;
extern CFStringRef kMRMediaRemoteOptionSourceID;
extern CFStringRef kMRMediaRemoteOptionStationHash;
extern CFStringRef kMRMediaRemoteOptionStationID;
extern CFStringRef kMRMediaRemoteOptionTrackID;
extern CFStringRef kMRMediaRemoteOriginUserInfoKey;
extern CFStringRef kMRMediaRemotePickableRoutesDidChangeNotification;
extern CFStringRef kMRMediaRemotePlaybackStateUserInfoKey;
extern CFStringRef kMRMediaRemoteRouteDescriptionUserInfoKey;
extern CFStringRef kMRMediaRemoteRouteStatusDidChangeNotification;
extern CFStringRef kMRMediaRemoteRouteStatusUserInfoKey;
extern CFStringRef kMRMediaRemoteUpdatedContentItemsUserInfoKey;
extern CFStringRef kMRNowPlayingClientUserInfoKey;
extern CFStringRef kMRNowPlayingPlaybackQueueChangedNotification;
extern CFStringRef kMRNowPlayingPlayerPathUserInfoKey;
extern CFStringRef kMRNowPlayingPlayerUserInfoKey;
extern CFStringRef kMROriginActiveNowPlayingPlayerPathUserInfoKey;
extern CFStringRef kMRPlaybackQueueContentItemsChangedNotification;


typedef enum {
    /*
     * Use nil for userInfo.
     */
    kMRPlay = 0,
    kMRPause = 1,
    kMRTogglePlayPause = 2,
    kMRStop = 3,
    kMRNextTrack = 4,
    kMRPreviousTrack = 5,
    kMRToggleShuffle = 6,
    kMRToggleRepeat = 7,
    kMRStartForwardSeek = 8,
    kMREndForwardSeek = 9,
    kMRStartBackwardSeek = 10,
    kMREndBackwardSeek = 11,
    kMRGoBackFifteenSeconds = 12,
    kMRSkipFifteenSeconds = 13,

    /*
     * Use a NSDictionary for userInfo, which contains three keys:
     * kMRMediaRemoteOptionTrackID
     * kMRMediaRemoteOptionStationID
     * kMRMediaRemoteOptionStationHash
     */
    kMRLikeTrack = 0x6A,
    kMRBanTrack = 0x6B,
    kMRAddTrackToWishList = 0x6C,
    kMRRemoveTrackFromWishList = 0x6D
} MRCommand;

typedef const struct __MRNowPlayingClient *MRNowPlayingClientRef;
typedef const struct __MROrigin           *MROriginRef;

typedef void (^MRMediaRemoteGetAvailableOriginsBlock)(CFArrayRef information);
typedef void (^MRMediaRemoteGetNowPlayingApplicationIsPlayingBlock)(Boolean isPlaying);
typedef void (^MRMediaRemoteGetNowPlayingApplicationPIDBlock)(int PID);
typedef void (^MRMediaRemoteGetNowPlayingClientBlock)(CFStringRef client);
typedef void (^MRMediaRemoteGetNowPlayingClientsBlock)(CFArrayRef clients);
typedef void (^MRMediaRemoteGetNowPlayingInfoBlock)(CFDictionaryRef information);

Boolean MRMediaRemotePickedRouteHasVolumeControl();
Boolean MRMediaRemoteSendCommand(MRCommand command, id userInfo);
CFArrayRef MRMediaRemoteCopyPickableRoutes();
CFArrayRef MRMediaRemoteCopyPickableRoutesForCategory(CFStringRef *category);
CFStringRef MRNowPlayingClientGetBundleIdentifier(id clientObj);
CFStringRef MRNowPlayingClientGetParentAppBundleIdentifier(id clientObj);
CFStringRef BundleIdentifier(MRNowPlayingClientRef client);
id MRMediaRemoteSendCommandToClient(int, NSDictionary *, id, id, int, int, id);
int MRMediaRemoteSelectSourceWithID(CFStringRef identifier);
void MRMediaRemoteBeginRouteDiscovery();
void MRMediaRemoteEndRouteDiscovery();
void MRMediaRemoteGetAvailableOrigins(dispatch_queue_t queue, MRMediaRemoteGetAvailableOriginsBlock completion);
void MRMediaRemoteGetNowPlayingApplicationIsPlaying(dispatch_queue_t queue, MRMediaRemoteGetNowPlayingApplicationIsPlayingBlock completion);
void MRMediaRemoteGetNowPlayingApplicationPID(dispatch_queue_t queue, MRMediaRemoteGetNowPlayingApplicationPIDBlock completion);
void MRMediaRemoteGetNowPlayingClient(dispatch_queue_t queue, MRMediaRemoteGetNowPlayingClientBlock block);
void MRMediaRemoteGetNowPlayingClients(dispatch_queue_t queue, MRMediaRemoteGetNowPlayingClientsBlock block);
void MRMediaRemoteGetNowPlayingInfo(dispatch_queue_t queue, MRMediaRemoteGetNowPlayingInfoBlock completion);
void MRMediaRemoteKeepAlive();
void MRMediaRemoteRegisterForNowPlayingNotifications(dispatch_queue_t queue);
void MRMediaRemoteSetCanBeNowPlayingApplication(Boolean can);
void MRMediaRemoteSetElapsedTime(double elapsedTime);
void MRMediaRemoteSetNowPlayingApplicationOverrideEnabled(Boolean enabled);
void MRMediaRemoteSetNowPlayingInfo(CFDictionaryRef information);
void MRMediaRemoteSetPickedRouteWithPassword(CFStringRef route, CFStringRef password);
void MRMediaRemoteSetPlaybackSpeed(int speed);
void MRMediaRemoteSetRepeatMode(int mode);
void MRMediaRemoteSetShuffleMode(int mode);
void MRMediaRemoteUnregisterForNowPlayingNotifications();

}

