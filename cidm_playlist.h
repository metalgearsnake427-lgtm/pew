#ifndef VIDM_PLAYLIST_H
#define VIDM_PLAYLIST_H

#include <string>
#include <vector>

struct VidMTrack
{
    int id;

    std::string title;
    std::string artist;
    std::string album;

    std::string filepath;

    long duration;
};

class VidMPlaylist
{
public:

    bool addTrack(
        const VidMTrack& track
    );

    bool removeTrack(
        int id
    );

    bool moveTrack(
        int from,
        int to
    );

    bool clear();

    const std::vector<VidMTrack>&
    tracks() const;

private:

    std::vector<VidMTrack> m_tracks;
};

#endif