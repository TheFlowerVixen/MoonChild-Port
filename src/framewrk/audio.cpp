#include "frm_int.hpp"

float calcVolume(INT32 volume);
float calcPan(INT32 pan);

extern char *FullPath( char *a_File );
extern char *FullAudioPath( char *a_File );
Caudio::Caudio(void)
{
    // NO NEED TO INIT AUDIO...ALREADY DONE BY THE INITAUDIO FUNCTION IN THE wii_audio.h FILE
}

Caudio::~Caudio(void)
{
}

UINT16 Caudio::play_cd(UINT16 tracknr)
{
    printf("play_cd(%d)\n", tracknr);
    
    TrackInfo track = getTrackInfo((TrackID)tracknr);
    
    if (!track.track) return 0;

    loadMusicFile(FullAudioPath((char *)track.track));
    playMusicLooping(track.volume);

	return 0;
}

void Caudio::checkVolume()
{
    // removed
}


void Caudio::play_cd_cb(UINT16 tracknr)
{
    // removed
}


void Caudio::stop_cd(void)
{
    stopMusic();
}

const char *soundFilenames[] = 
{
    "", //0
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",  //15
    "mcskid.wav",  //16
    "spring.wav", //17
    "mcwall.wav", //18
    "mcdrain.wav", //19
    "mcfall.wav", //20
    "blub.wav", //21
    "door.wav",  //22
    "switch.wav", //23
    "bonus.wav", //24
    "warp.wav", //25
    "shoot.wav",  //26
    "rumble.wav", //27
    "waterval.wav", //28
    "mcloop1.wav", //29
    "mcloop2.wav", //30
    "sapstap1.wav", //31
    "sapstap2.wav", //32
    "helmlp1.wav", //33
    "helmlp2.wav", //34
    "houtpunt.wav", //35
    "ball.wav", //36
    "segmshot.wav", //37
    "segmexpl.wav", //38
    "segmhit.wav", //39
    "vlamwerp.wav",  //40
    "tanden.wav", //41
    "tandenm.wav", //42
    "camstart.wav",  //43
    "camstop.wav",  //44
    "cammove.wav",  //45
    "madeit.wav", //46
    "heks.wav", //47
    "graspod.wav", //48
    "bat.wav", //49
    "vogel.wav", //50
    "vuur.wav", //51
    "drilboor.wav", //52
    "loopband.wav", //53
    "ventltor.wav", //54
    "bee.wav", //55
    "bee2.wav", //56
    "ptoei.wav", //57
    "schuif.wav", //58
    "smexp.wav", //59
    "backpak.wav", //60
    "restart.wav", //61
    "bigexp.wav", //62
    "stroom.wav", //63
    "cannon.wav", //64
    "gewicht.wav", //65
    "wheel.wav", //66
    "appel.wav", //67
    "mcdood.wav", //68
    "mcfart.wav", //69
    "chemo.wav", //70
    "tik.wav", //71
    "tak.wav",//72
    "raket.wav", //73
    "chemo2.wav", //74
    "helmdood.wav", //75
    "ketting.wav", //76
    "dimndsht.wav", //77
    "glasblok.wav", //78
    "hyprlift.wav", //79
    "lightwav.wav", //80
    "morphsht.wav", //81
    "mushup.wav", //82
    "mushdwn.wav", //83
    "plntlft.wav", //84
    "pltfdwn.wav", //85
    "pltfup.wav", //86
    "qbert1.wav", //87
    "qbert2.wav", //88
    "roltnlp.wav", //89
    "slowlift.wav", //90
    "tangjmp.wav", //91
    "tangclos.wav", //92
    "woeiwoei.wav", //93
    "heatskr.wav", //94
    "demo.wav", //95
    "explo.wav",
    "mcdrainold.wav"
};

HSNDOBJ Caudio::create_sound(int SoundID, int nrof_simult)
{
    int soundHandle = loadWaveSample(FullAudioPath((char *)soundFilenames[SoundID]));
	return (long)soundHandle;
}


void Caudio::destroy_sound(HSNDOBJ sound)
{
    if (sound < 0) return;
    freeWaveSample((int)sound);
}

void Caudio::reset_audio()
{
}

void Caudio::stop_sound(HSNDOBJ sound)
{
    if (sound < 0) return;
    stopWaveSample((int)sound);
}


void Caudio::stop_cursound(HSNDOBJ sound)
{
    if (sound < 0) return;
    stopWaveSample((int)sound);
}


void Caudio::play_sound_1shot(HSNDOBJ sound, INT32 volume, INT32 pan)
{
    if (sound < 0) return;

    int channel = playWaveOneshot((int)sound);
    if (channel < 0) return;

    sound_volume(channel, volume);
    sound_pan(channel, pan);
}


void Caudio::play_sound_loop(HSNDOBJ sound, INT32 volume, INT32 pan)
{
    int channel = playWaveLooping((int)sound);
    if (channel < 0) return;

    sound_volume(channel, volume);
    sound_pan(channel, pan);
}


void Caudio::sound_volume(HSNDOBJ sound, INT32 volume)
{
    if (sound < 0) return;
    int convertedVolume = ((float)(volume + 4000) / 4000.0f) * MAXVOLUME;
 //   convertedVolume = 0; // TODO: remove this
    volumeWaveSample((int)sound, convertedVolume);
}


// pan is from -1000 (to far away on the left side, to 0 (center), to 1000 (to far away on the right side)
void Caudio::sound_pan(HSNDOBJ sound, INT32 pan)
{
    if (pan < -1000) pan = -1000;
    if (pan > 1000) pan = 1000;
    int leftPan = 0;
    int rightPan = 0;
    INT32 absPan = pan < 0 ? -pan : pan;
    if (absPan > 1000) {
        absPan = 1000;
    }
    int linearPart = (255 * (1000 - absPan)) / 1000;
    int squaredPart = (linearPart * (1000 - absPan)) / 1000;
    if (pan >= 0) {
        leftPan = squaredPart;
        rightPan = linearPart;
    } else {
        leftPan = linearPart;
        rightPan = squaredPart;
    }
//    leftPan = 0;
 //   rightPan = 0;
    panWaveSample((int)sound, leftPan, rightPan);
}

int Caudio::get_dsound(void)
{
	return 1;
}

  
