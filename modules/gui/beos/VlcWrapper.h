/*****************************************************************************
 * VlcWrapper.h: BeOS plugin for vlc (derived from MacOS X port)
 *****************************************************************************
 * Copyright (C) 2001 VideoLAN
 * $Id: VlcWrapper.h,v 1.8 2002/11/27 05:36:41 titer Exp $
 *
 * Authors: Florian G. Pflug <fgp@phlo.org>
 *          Jon Lech Johansen <jon-vl@nanocrew.net>
 *          Tony Castley <tony@castley.net>
 *          Stephan Aßmus <stippi@yellowbites.com>
 *          Eric Petit <titer@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#define SEEKSLIDER_RANGE 2048

class InterfaceWindow;
class VlcWrapper;

/*****************************************************************************
 * intf_sys_t: internal variables of the BeOS interface
 *****************************************************************************/
struct intf_sys_t
{
    InterfaceWindow * p_window;
    
    vlc_bool_t        b_disabled_menus;
    vlc_bool_t        b_loop;
    vlc_bool_t        b_mute;
    int	              i_part;
    int               i_saved_volume;
    int               i_channel;
    
    VlcWrapper * p_wrapper;
};

/*****************************************************************************
 * VlcWrapper
 *****************************************************************************
 * This class makes the link between the BeOS interface and the vlc core.
 * There is only one VlcWrapper instance at any time, which is stored
 * in p_intf->p_sys->p_wrapper
 *****************************************************************************/
class VlcWrapper
{
public:
    VlcWrapper( intf_thread_t *p_intf );
    ~VlcWrapper();
    
    bool UpdateInputAndAOut();
    
    /* input */
    int InputStatus();
    int InputRate();
    int InputTell();
    int InputSize();
    void InputSlower();
    void InputFaster();
    void openFiles( BList *o_files, bool replace = true );
    void openDisc( BString o_type, BString o_device,
                   int i_title, int i_chapter );
    void toggleLanguage( int i_language );
    void toggleSubtitle( int i_subtitle );
    const char* getTimeAsString();
    float getTimeAsFloat();
    void setTimeAsFloat( float i_offset );

        
    /* Playlist */
    int PlaylistSize();
    char *PlaylistItemName( int );
    int PlaylistCurrent();
    int PlaylistStatus();
    bool PlaylistPlay();
    void PlaylistPause();
    void PlaylistStop();
    void PlaylistNext();
    void PlaylistPrev();
    void PlaylistSkip(int i);
    void PlaylistGoto(int i);
    void PlaylistLoop(); 
    BList* PlaylistAsArray();
    bool PlaylistPlaying();
    void getPlaylistInfo( int32& currentIndex,
                            int32& maxIndex );
    void PlaylistJumpTo( int );
    void getNavCapabilities( bool* canSkipPrev,
                             bool* canSkipNext );
	void navigatePrev();
	void navigateNext();

    /* audio */
    void volume_mute();
    void volume_restore();
    void set_volume(int value);
    void toggle_mute( );
    bool is_muted();
    bool is_playing();
    void maxvolume();
    bool has_audio();

    /* DVD */
    bool HasTitles();
    void PrevTitle();
    void NextTitle();
    bool HasChapters();
    void PrevChapter();
    void NextChapter();
    void TitleInfo( int32& currentIndex, int32& maxIndex );
    void ChapterInfo( int32& currentIndex, int32& maxIndex );
    void toggleTitle( int i_title );
    void toggleChapter( int i_chapter );
    
private:
    intf_thread_t * p_intf;
    input_thread_t * p_input;
    playlist_t * p_playlist;
    aout_instance_t * p_aout;
};

