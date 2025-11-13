// Copyright (c) 2017-2025 Manuel Schneider

#pragma once
#include <QString>
#include <objc/runtime.h>
#include <memory>
namespace albert { class Icon; }

class Player
{
public:

    Player(int pid);
    ~Player();

    QString name() const;
    std::unique_ptr<albert::Icon> icon();

    bool isPlaying() const;
    bool canPlay() const;
    bool canPause() const;
    bool canGoNext() const;
    bool canGoPrevious() const;

    void play();
    void pause();
    void next();
    void previous();

private:

    bool is_playing_;
    QString name_;
    QString bundle_path_;
    ::id notification_observation_;

};


