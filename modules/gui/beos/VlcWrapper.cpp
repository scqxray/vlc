/*****************************************************************************
 * VlcWrapper.cpp: BeOS plugin for vlc (derived from MacOS X port)
 *****************************************************************************
 * Copyright (C) 2001 VideoLAN
 * $Id: VlcWrapper.cpp,v 1.12 2002/11/27 05:36:41 titer Exp $
 *
 * Authors: Florian G. Pflug <fgp@phlo.org>
 *          Jon Lech Johansen <jon-vl@nanocrew.net>
 *          Tony Casltey <tony@castley.net>
 *          Stephan Aßmus <stippi@yellowbites.com>
 *          Eric Petit <titer@videolan.org>
 *
 * This program is free software{} you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation{} either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/
#include <SupportKit.h>

#include <vlc/vlc.h>
#include <vlc/intf.h>
#include <audio_output.h>
#include <aout_internal.h>

#include "VlcWrapper.h"

/* constructor */
VlcWrapper::VlcWrapper( intf_thread_t *p_interface )
{
    p_intf = p_interface;
    p_input = NULL;
    p_aout = NULL;
    p_playlist = (playlist_t *)vlc_object_find( p_intf, VLC_OBJECT_PLAYLIST,
                                                FIND_ANYWHERE );
}

/* destructor */
VlcWrapper::~VlcWrapper()
{
    if( p_input )
    {
        vlc_object_release( p_input );
    }
    if( p_playlist )
    {
        vlc_object_release( p_playlist );
    }
    if( p_aout )
    {
        vlc_object_release( p_aout );
    }
}

/* UpdateInputAndAOut: updates p_input and p_aout, returns true if the
   interface needs to be updated */
bool VlcWrapper::UpdateInputAndAOut()
{
    if( p_input == NULL )
    {
        p_input = (input_thread_t *)vlc_object_find( p_intf, VLC_OBJECT_INPUT,
                                                     FIND_ANYWHERE );
    }
    if( p_aout == NULL )
    {
        p_aout = (aout_instance_t*)vlc_object_find( p_intf, VLC_OBJECT_AOUT,
                                                    FIND_ANYWHERE );
    }
        
    if( p_input != NULL )
    {
        if( p_input->b_dead )
        {
            vlc_object_release( p_input );
            p_input = NULL;
            
            if( p_aout )
            {
                vlc_object_release( p_aout );
                p_aout = NULL;
            }
        }
        return true;
    }
    return false;
}


/***************************
 * input infos and control *
 ***************************/

/* status (UNDEF_S, PLAYING_S, PAUSE_S, FORWARD_S, BACKWARD_S,
   REWIND_S, NOT_STARTED_S, START_S) */
int VlcWrapper::InputStatus()
{
    if( !p_input )
    {
        return UNDEF_S;
    }
    return p_input->stream.control.i_status;
}

int VlcWrapper::InputRate()
{
    if( !p_input )
    {
        return DEFAULT_RATE;
    }
    return p_input->stream.control.i_rate;
}

/* tell: location in the current stream (in arbitrary units) */
int VlcWrapper::InputTell()
{
    if( !p_input )
    {
        return -1;
    }
    return p_input->stream.p_selected_area->i_tell;
}

/* size: total size of the current stream (in arbitrary units) */
int VlcWrapper::InputSize()
{
    if( !p_input )
    {
        return -1;
    }
    return p_input->stream.p_selected_area->i_size;
}

void VlcWrapper::InputSlower()
{
    if( p_input != NULL )
    {
        input_SetStatus( p_input, INPUT_STATUS_SLOWER );
    }
    if( p_input->stream.control.i_rate == DEFAULT_RATE)
    {
        toggle_mute();
    }
    else
    {
        toggle_mute();
    }
}

void VlcWrapper::InputFaster()
{
    if( p_input != NULL )
    {
        input_SetStatus( p_input, INPUT_STATUS_FASTER );
    }
    if( p_input->stream.control.i_rate == DEFAULT_RATE)
    {
        toggle_mute();
    }
    else
    {
        toggle_mute();
    }
}

void VlcWrapper::openFiles( BList* o_files, bool replace )
{
    BString *o_file;

    while( ( o_file = (BString *)o_files->LastItem() ) )
    {
        o_files->RemoveItem(o_files->CountItems() - 1);
        playlist_Add( p_playlist, o_file->String(),
                  PLAYLIST_APPEND | PLAYLIST_GO, PLAYLIST_END );
        delete o_file;
    }
}

void VlcWrapper::openDisc(BString o_type, BString o_device, int i_title, int i_chapter)
{
    BString o_source("");
    o_source << o_type << ":" << o_device ;

    playlist_Add( p_playlist, o_source.String(),
                  PLAYLIST_APPEND | PLAYLIST_GO, PLAYLIST_END );
}



void VlcWrapper::toggleLanguage(int i_language)
{

    int32 i_old = -1;
    int i_cat = AUDIO_ES;

    vlc_mutex_lock( &p_input->stream.stream_lock );
    for( int i = 0; i < p_input->stream.i_selected_es_number ; i++ )
    {
        if( p_input->stream.pp_selected_es[i]->i_cat == i_cat )
        {
            i_old = i;
            break;
        }
    }
    vlc_mutex_unlock( &p_input->stream.stream_lock );

    msg_Info( p_intf, "Old: %d,  New: %d", i_old, i_language);
    if( i_language != -1 )
    {
        input_ToggleES( p_input, 
                        p_input->stream.pp_selected_es[i_language],
                        VLC_TRUE );
    }

    if( (i_old != -1) && (i_old != i_language) )
    {
        input_ToggleES( p_input, 
                        p_input->stream.pp_selected_es[i_old],
                        VLC_FALSE );
    }
}

void VlcWrapper::toggleSubtitle(int i_subtitle)
{
    int32 i_old = -1;
    int i_cat = SPU_ES;

    vlc_mutex_lock( &p_input->stream.stream_lock );
    for( int i = 0; i < p_input->stream.i_selected_es_number ; i++ )
    {
        if( p_input->stream.pp_selected_es[i]->i_cat == i_cat )
        {
            i_old = i;
            break;
        }
    }
    vlc_mutex_unlock( &p_input->stream.stream_lock );
    
    msg_Info( p_intf, "Old: %d,  New: %d", i_old, i_subtitle);
    if( i_subtitle != -1 )
    {
        input_ToggleES( p_input, 
                        p_input->stream.pp_selected_es[i_subtitle],
                        VLC_TRUE );
    }

    if( (i_old != -1) && (i_old != i_subtitle) )
    {
        input_ToggleES( p_input, 
                        p_input->stream.pp_selected_es[i_old],
                        VLC_FALSE );
    }
}

const char*  VlcWrapper::getTimeAsString()
{
    static char psz_currenttime[ OFFSETTOTIME_MAX_SIZE ];
        
    if( p_input == NULL )
    {
        return ("-:--:--");
    }     
   
    input_OffsetToTime( p_input, 
                        psz_currenttime, 
                        p_input->stream.p_selected_area->i_tell );        

    return(psz_currenttime);
}

float  VlcWrapper::getTimeAsFloat()
{
    float f_time = 0.0;

    if( p_input != NULL )
    {
        f_time = (float)p_input->stream.p_selected_area->i_tell / 
                 (float)p_input->stream.p_selected_area->i_size;
    }    
    else
    {
        f_time = 0.0;
    }
    return( f_time );
}

void VlcWrapper::setTimeAsFloat(float f_position)
{
    if( p_input != NULL )
    {
        input_Seek( p_input, 
                   (long long int)(p_input->stream.p_selected_area->i_size
                       * f_position / SEEKSLIDER_RANGE ), 
                   INPUT_SEEK_SET);
    }
}


/******************************
 * playlist infos and control *
 ******************************/
int VlcWrapper::PlaylistSize()
{
    vlc_mutex_lock( &p_playlist->object_lock );
    int i_size = p_playlist->i_size;
    vlc_mutex_unlock( &p_playlist->object_lock );
    return i_size;
}

char *VlcWrapper::PlaylistItemName( int i )
{
   return p_playlist->pp_items[i]->psz_name;
}

int VlcWrapper::PlaylistCurrent()
{
    return p_playlist->i_index;
}

int  VlcWrapper::PlaylistStatus()
{
    return p_playlist->i_status;
}

bool VlcWrapper::PlaylistPlay()
{
    if( PlaylistSize() )
    {
        playlist_Play( p_playlist );
    }
    return( true );
}

void VlcWrapper::PlaylistPause()
{
    toggle_mute();
    if( p_input )
    {
        input_SetStatus( p_input, INPUT_STATUS_PAUSE );
    }
}

void VlcWrapper::PlaylistStop()
{
    volume_mute();
    playlist_Stop( p_playlist );
}

void VlcWrapper::PlaylistNext()
{
    playlist_Next( p_playlist );
}

void VlcWrapper::PlaylistPrev()
{
    playlist_Prev( p_playlist );
}

void VlcWrapper::PlaylistSkip( int i )
{
    playlist_Skip( p_playlist, i );
}

void VlcWrapper::PlaylistGoto( int i )
{
    playlist_Goto( p_playlist, i );
}

void VlcWrapper::PlaylistLoop()
{
    if ( p_intf->p_sys->b_loop )
    {
        playlist_Delete( p_playlist, p_playlist->i_size - 1 );
    }
    else
    {
        playlist_Add( p_playlist, "vlc:loop",
                      PLAYLIST_APPEND | PLAYLIST_GO,
                      PLAYLIST_END );
    }
    p_intf->p_sys->b_loop = !p_intf->p_sys->b_loop;
}

BList * VlcWrapper::PlaylistAsArray()
{ 
    int i;
    BList* p_list = new BList(p_playlist->i_size);
    
    vlc_mutex_lock( &p_playlist->object_lock );

    for( i = 0; i < p_playlist->i_size; i++ )
    {
        p_list->AddItem(new BString(p_playlist->pp_items[i]->psz_name));
    }

    vlc_mutex_unlock( &p_playlist->object_lock );
    return( p_list );
}

void VlcWrapper::getPlaylistInfo( int32& currentIndex, int32& maxIndex )
{
	currentIndex = -1;
	maxIndex = -1;
	if ( p_playlist )
	{
		maxIndex = p_playlist->i_size;
		if ( maxIndex > 0 )
			currentIndex = p_playlist->i_index + 1;
		else
			maxIndex = -1;
	}
}


void VlcWrapper::PlaylistJumpTo( int pos )
{
#if 0
	// sanity checks
	if ( pos < 0 )
		pos = 0;
	int size = playlistSize();
	if (pos >= size)
		pos = size - 1;
	// weird hack
    if( p_input_bank->pp_input[0] != NULL )
		pos--;
	// stop current stream
	playlistStop();
	// modify current position in playlist
	playlistLock();
	p_main->p_playlist->i_index = pos;
	playlistUnlock();
	// start playing
	playlistPlay();
#endif
}

void VlcWrapper::getNavCapabilities( bool *canSkipPrev, bool *canSkipNext )
{
	if ( canSkipPrev && canSkipNext )
	{
		// init the parameters
		*canSkipPrev = false;
		*canSkipNext = false;
		// get playlist info
		int pos = PlaylistCurrent();
		int size = PlaylistSize();

		// see if we have got a stream going		
		if ( p_input )
		{
			vlc_mutex_lock( &p_input->stream.stream_lock );

			bool hasTitles = p_input->stream.i_area_nb > 1;
			int numChapters = p_input->stream.p_selected_area->i_part_nb;
			bool hasChapters = numChapters > 1;
			// first, look for chapters
			if ( hasChapters )
			{
				*canSkipPrev = p_input->stream.p_selected_area->i_part > 0;
				*canSkipNext = p_input->stream.p_selected_area->i_part <
									 p_input->stream.p_selected_area->i_part_nb - 1;
			}
			// if one of the skip capabilities is false,
			// make it depend on titles instead
			if ( !*canSkipPrev && hasTitles )
				*canSkipPrev = p_input->stream.p_selected_area->i_id > 1;
			if ( !*canSkipNext && hasTitles )
				*canSkipNext = p_input->stream.p_selected_area->i_id <
				                   p_input->stream.i_area_nb - 1;

			vlc_mutex_unlock( &p_input->stream.stream_lock );
		}
		// last but not least, make capabilities depend on playlist
		if ( !*canSkipPrev )
			*canSkipPrev = pos > 0;
		if ( !*canSkipNext )
			*canSkipNext = pos < size - 1;
	}
}

void VlcWrapper::navigatePrev()
{
	bool hasSkiped = false;

	// see if we have got a stream going		
	if ( p_input )
	{
		// get information from stream (lock it while looking at it)
		vlc_mutex_lock( &p_input->stream.stream_lock );

		int currentTitle = p_input->stream.p_selected_area->i_id;
		int currentChapter = p_input->stream.p_selected_area->i_part;
		int numTitles = p_input->stream.i_area_nb;
		bool hasTitles = numTitles > 1;
		int numChapters = p_input->stream.p_selected_area->i_part_nb;
		bool hasChapters = numChapters > 1;

		vlc_mutex_unlock( &p_input->stream.stream_lock );

		// first, look for chapters
		if ( hasChapters )
		{
			// skip to the previous chapter
			currentChapter--;

			if ( currentChapter >= 0 )
			{
				toggleChapter( currentChapter );
				hasSkiped = true;
			}
		}
		// if we couldn't skip chapters, try titles instead
		if ( !hasSkiped && hasTitles )
		{
			// skip to the previous title
			currentTitle--;
			// disallow area 0 since it is used for video_ts.vob
			if( currentTitle > 0 )
			{
				toggleTitle(currentTitle);
				hasSkiped = true;
			}
		}

	}
	// last but not least, skip to previous file
	if ( !hasSkiped )
		PlaylistPrev();
}

void VlcWrapper::navigateNext()
{
	bool hasSkiped = false;

	// see if we have got a stream going		
	if ( p_input )
	{
		// get information from stream (lock it while looking at it)
		vlc_mutex_lock( &p_input->stream.stream_lock );

		int currentTitle = p_input->stream.p_selected_area->i_id;
		int currentChapter = p_input->stream.p_selected_area->i_part;
		int numTitles = p_input->stream.i_area_nb;
		bool hasTitles = numTitles > 1;
		int numChapters = p_input->stream.p_selected_area->i_part_nb;
		bool hasChapters = numChapters > 1;

		vlc_mutex_unlock( &p_input->stream.stream_lock );

		// first, look for chapters
		if ( hasChapters )
		{
			// skip to the next chapter
			currentChapter++;
			if ( currentChapter < numChapters )
			{
				toggleChapter( currentChapter );
				hasSkiped = true;
			}
		}
		// if we couldn't skip chapters, try titles instead
		if ( !hasSkiped && hasTitles )
		{
			// skip to the next title
			currentTitle++;
			// disallow area 0 since it is used for video_ts.vob
			if ( currentTitle < numTitles - 1 )
			{
				toggleTitle(currentTitle);
				hasSkiped = true;
			}
		}

	}
	// last but not least, skip to next file
	if ( !hasSkiped )
		PlaylistNext();
}


/***************************
 * audio infos and control *
 ***************************/

void VlcWrapper::volume_mute()
{
    if( p_aout != NULL )
    {
	    if( !p_intf->p_sys->b_mute )
		{
		    p_intf->p_sys->i_saved_volume = p_aout->output.i_volume;
		    p_aout->output.i_volume = 0;
		    p_intf->p_sys->b_mute = 1;
		}
    }

}

void VlcWrapper::volume_restore()
{
    if( p_aout != NULL )
    {
	    p_aout->output.i_volume = p_intf->p_sys->i_saved_volume;
		p_intf->p_sys->i_saved_volume = 0;
	    p_intf->p_sys->b_mute = 0;
    }

}

void VlcWrapper::set_volume(int value)
{
    if( p_aout != NULL )
    {
		// make sure value is within bounds
		if (value < 0)
			value = 0;
		if (value > AOUT_VOLUME_MAX)
			value = AOUT_VOLUME_MAX;
		vlc_mutex_lock( &p_aout->mixer_lock );
		// unmute volume if muted
		if ( p_intf->p_sys->b_mute )
		{
			p_intf->p_sys->b_mute = 0;
            p_aout->output.i_volume = value;
		}
		vlc_mutex_unlock( &p_aout->mixer_lock );
    }
}

void VlcWrapper::toggle_mute()
{
    if( p_aout != NULL )
   	{
	    if ( p_intf->p_sys->b_mute )
	    {
	        volume_restore();
	    }
	    else
	    {
	        volume_mute();
	    }
	}
}

bool VlcWrapper::is_muted()
{
	bool muted = true;
	
    if( p_aout != NULL )
	{
		vlc_mutex_lock( &p_aout->mixer_lock );
		if( p_aout->output.i_volume > 0 )
		{
			muted = false;
		}
		vlc_mutex_unlock( &p_aout->mixer_lock );
// unfortunately, this is not reliable!
//		return p_main->p_intf->p_sys->b_mute;
	}
	return muted;
}

bool VlcWrapper::is_playing()
{

	bool playing = false;
	if ( p_input )
	{
		switch ( p_input->stream.control.i_status )
		{
			case PLAYING_S:
			case FORWARD_S:
			case BACKWARD_S:
			case START_S:
				playing = true;
	            break;
			case PAUSE_S:
			case UNDEF_S:
			case NOT_STARTED_S:
			default:
				break;
		}
	}
	return playing;

}

void VlcWrapper::maxvolume()
{
    if( p_aout != NULL )
    {
	    if( p_intf->p_sys->b_mute )
	    {
	        p_intf->p_sys->i_saved_volume = AOUT_VOLUME_MAX;
	    }
	    else
	    {
	        p_aout->output.i_volume = AOUT_VOLUME_MAX;
	    }
    }
}

bool VlcWrapper::has_audio()
{
    return( p_aout != NULL );
}

/*******
 * DVD *
 *******/
bool VlcWrapper::HasTitles()
{
    if( !p_input )
    {
        return false;
    }
    return ( p_input->stream.i_area_nb > 1 );
}

void VlcWrapper::PrevTitle()
{
    int i_id;
    i_id = p_input->stream.p_selected_area->i_id - 1;
    if( i_id > 0 )
    {
        toggleTitle(i_id);
    }
}

void VlcWrapper::NextTitle()
{
    int i_id;
    i_id = p_input->stream.p_selected_area->i_id + 1;
    if( i_id < p_input->stream.i_area_nb )
    {
        toggleTitle(i_id);
    }
}

bool VlcWrapper::HasChapters()
{
    if( !p_input )
    {
        return false;
    }
    return ( p_input->stream.p_selected_area->i_part_nb > 1 );
}

void VlcWrapper::PrevChapter()
{
    int i_id;
    i_id = p_input->stream.p_selected_area->i_part - 1;
    if( i_id >= 0 )
    {
        toggleChapter(i_id);
    }
}

void VlcWrapper::NextChapter()
{
    int i_id;
    i_id = p_input->stream.p_selected_area->i_part + 1;
    if( i_id >= 0 )
    {
        toggleChapter(i_id);
    }
}

void VlcWrapper::TitleInfo( int32 &currentIndex, int32 &maxIndex )
{
	currentIndex = -1;
	maxIndex = -1;
	if ( p_input )
	{
		vlc_mutex_lock( &p_input->stream.stream_lock );

		maxIndex = p_input->stream.i_area_nb - 1;
		if ( maxIndex > 0)
			currentIndex = p_input->stream.p_selected_area->i_id;
		else
			maxIndex = -1;

		vlc_mutex_unlock( &p_input->stream.stream_lock );
	}
}

void VlcWrapper::ChapterInfo( int32 &currentIndex, int32 &maxIndex )
{
	currentIndex = -1;
	maxIndex = -1;
	if ( p_input )
	{
		vlc_mutex_lock( &p_input->stream.stream_lock );

		maxIndex = p_input->stream.p_selected_area->i_part_nb - 1;
		if ( maxIndex > 0)
			currentIndex = p_input->stream.p_selected_area->i_part;
		else
			maxIndex = -1;

		vlc_mutex_unlock( &p_input->stream.stream_lock );
	}
}

void VlcWrapper::toggleTitle(int i_title)
{
    if( p_input != NULL )
    {
        input_ChangeArea( p_input,
                          p_input->stream.pp_areas[i_title] );

        vlc_mutex_lock( &p_input->stream.stream_lock );

        vlc_mutex_unlock( &p_input->stream.stream_lock );
    }
}

void VlcWrapper::toggleChapter(int i_chapter)
{
    if( p_input != NULL )
    {
        p_input->stream.p_selected_area->i_part = i_chapter;
        input_ChangeArea( p_input,
                          p_input->stream.p_selected_area );

        vlc_mutex_lock( &p_input->stream.stream_lock );
        vlc_mutex_unlock( &p_input->stream.stream_lock );
    }
}
