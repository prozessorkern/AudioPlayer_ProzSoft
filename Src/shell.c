#include "pcm1755.h"
#include "data_store_global.h"
#include "data_store_track.h"
#include "version.h"
#include "string.h"
#include "dmx.h"
#include "power_amp.h"
#include "prozStdio.h"
#include "shell.h"
#include "remote.h"
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

static void shellPrintPrompt(shellBuffer_t *buffer);
static int32_t shellGetArg(shellBuffer_t *buffer, uint8_t argIdx, uint8_t *data, uint32_t dataLen, uint8_t readRest);
static void cmdHelp(shellBuffer_t *buffer, uint8_t data);
static void cmdVer(shellBuffer_t *buffer, uint8_t data);
static void cmdLoad(shellBuffer_t *buffer, uint8_t data);
static void cmdPlay(shellBuffer_t *buffer, uint8_t data);
static void cmdPause(shellBuffer_t *buffer, uint8_t data);
static void cmdStop(shellBuffer_t *buffer, uint8_t data);
static void cmdInfo(shellBuffer_t *buffer, uint8_t data);
static void cmdName(shellBuffer_t *buffer, uint8_t data);
static void cmdDesc(shellBuffer_t *buffer, uint8_t data);
static void cmdSave(shellBuffer_t *buffer, uint8_t data);
static void cmdReset(shellBuffer_t *buffer, uint8_t data);
static void cmdAmp(shellBuffer_t *buffer, uint8_t data);
static void cmdVol(shellBuffer_t *buffer, uint8_t data);
static void cmdStats(shellBuffer_t *buffer, uint8_t data);
static void cmdDmx(shellBuffer_t *buffer, uint8_t data);
static void cmdAdr(shellBuffer_t *buffer, uint8_t data);
static void cmdRemote(shellBuffer_t *buffer, uint8_t data);

static uint32_t cmdRun = UINT32_MAX;
static uint32_t tempCmdRun = UINT32_MAX;
static uint32_t trackIdx;

static shellCommand_t commands[] =
{
    {"help", &cmdHelp, "prints this help"},
    {"ver", &cmdVer, "prints the version of the firmware"},
    {"load", &cmdLoad, "load a specified file number play 1..4 - unload if no number given"},
    {"play", &cmdPlay, "plays the loaded file"},
    {"pause", &cmdPause, "pauses the loaded file"},
    {"stop", &cmdStop, "stops the loaded file"},
    {"info", &cmdInfo, "reads the information of the selected track"},
    {"name", &cmdName, "reads/writes the name of the selected track"},
    {"desc", &cmdDesc, "reads/writes the description of the selected track"},
    {"save", &cmdSave, "saves all changes to the SD Card"},
    {"reset", &cmdReset, "resets the player"},
    {"amp", &cmdAmp, "enables/disables the power amplifier"},
    {"vol", &cmdVol, "reads/sets the volume of the selected track"},
    {"stats", &cmdStats, "read/init the stats"},
    {"dmx", &cmdDmx, "set size / record"},
    {"adr", &cmdAdr, "get/set remote adress"},
    {"remote", &cmdRemote, "open a connection to a slave"},
};

void shellDoProcess()
{
  if(localRingBuffer.readPtr != localRingBuffer.writePtr)
  {
    shellByteReveiced(&localShellBuffer, localRingBuffer.data[localRingBuffer.readPtr]);

    if(localRingBuffer.readPtr < PRINT_BUFFER_SIZE)
    {
      localRingBuffer.readPtr ++;
    }
    else
    {
      localRingBuffer.readPtr = 0u;
    }
  }

  if(remoteRingBuffer.readPtr != remoteRingBuffer.writePtr)
  {
    shellByteReveiced(&remoteShellBuffer, remoteRingBuffer.data[remoteRingBuffer.readPtr]);

    if(remoteRingBuffer.readPtr < PRINT_BUFFER_SIZE)
    {
      remoteRingBuffer.readPtr ++;
    }
    else
    {
      remoteRingBuffer.readPtr = 0u;
    }
  }

  if(0 == remoteShellBuffer.sendBuffer.busy)
  {
    RemoteDoProcess();
  }
}

void shellByteReveiced(shellBuffer_t *buffer, uint8_t data)
{
  uint32_t cmdCnt = 0u;

  if(cmdRun == UINT32_MAX)
  {

    if('\b' == data)
    {
      if(buffer->receiveBuffer.writePtr > 0)
      {
        buffer->receiveBuffer.writePtr --;
        prozPrintf(buffer, "\b \b");
      }
    }
    else if(  ('\e' == data) ||
              (0x03 == data))
    {
      buffer->receiveBuffer.writePtr = 0u;
      buffer->receiveBuffer.readPtr = 0u;

      shellPrintPrompt(buffer);
    }
    else if(buffer->receiveBuffer.writePtr < (PRINT_BUFFER_SIZE - 1))
    {
      buffer->receiveBuffer.data[buffer->receiveBuffer.writePtr] = data;
      buffer->receiveBuffer.writePtr ++;
      sendByte(buffer, data);
    }

    /*! - check for return */
    if('\r' == data)
    {
      sendByte(buffer, '\n');
      /*command*/
      for(cmdCnt = 0u; cmdCnt < (sizeof(commands)/sizeof(commands[0])); cmdCnt ++)
      {
        if(0u == memcmp(buffer->receiveBuffer.data, commands[cmdCnt].command, strlen(commands[cmdCnt].command)))
        {
          tempCmdRun = cmdCnt;
          commands[cmdCnt].cmdFunction(buffer, data);
          break;
        }
      }

      buffer->receiveBuffer.writePtr = 0u;
      buffer->receiveBuffer.readPtr = 0u;

      if(UINT32_MAX == cmdRun)
      {
        shellPrintPrompt(buffer);
      }
    }
  }
  else if(cmdRun < (sizeof(commands)/sizeof(commands[0])))
  {
    commands[cmdRun].cmdFunction(buffer, data);
  }
}

static void shellPrintPrompt(shellBuffer_t *buffer)
{
  prozPrintf(buffer, "\n\raudioPlayer->");
}

static int32_t shellGetArg(shellBuffer_t *buffer, uint8_t argIdx, uint8_t *data, uint32_t dataLen, uint8_t readRest)
{
  uint8_t argCnt = 0u;
  uint32_t byteCnt = buffer->receiveBuffer.readPtr;
  uint32_t copyCnt = 0u;
  int32_t ret = -1;

  /*! - loop through the input stream and search for the demanded argument index */
  while((byteCnt < buffer->receiveBuffer.writePtr) && ((argCnt <= argIdx) || (0u != readRest)))
  {
    if(buffer->receiveBuffer.data[byteCnt] == ' ')
    {
      argCnt ++;
    }

    if(((argCnt == argIdx) && (buffer->receiveBuffer.data[byteCnt] != ' ')) ||
        ((argCnt > argIdx) && (0u != readRest)))
    {
      if(copyCnt < dataLen)
      {
        /*! - copy one byte */
        data[copyCnt] = buffer->receiveBuffer.data[byteCnt];
        copyCnt ++;
        ret = copyCnt + 1;
      }
      else
      {
        /*! - buffer overrun */
        ret = -1;
        byteCnt = buffer->receiveBuffer.writePtr;
      }
    }

    byteCnt ++;
  }

  if(copyCnt < dataLen)
  {
    data[copyCnt] = 0u;
  }
  else
  {
    ret = -1;
  }

  return ret;

}

static void cmdHelp(shellBuffer_t *buffer, uint8_t data)
{
  uint32_t cmdCnt = 0u;

  prozPrintf(buffer, "\n\rhelp:\n\r");

  for(cmdCnt = 0u; cmdCnt < (sizeof(commands)/sizeof(commands[0])); cmdCnt ++)
  {
    prozPrintf(buffer, "%s \t\t%s\n\r", commands[cmdCnt].command, commands[cmdCnt].help);
  }

  prozPrintf(buffer, "\n\r");

}

static void cmdVer(shellBuffer_t *buffer, uint8_t data)
{

  prozPrintf(buffer, "\n\rFirmware Version:\n\r");
  prozPrintf(buffer, "%d.%02d\n\r", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR);
  prozPrintf(buffer, "%s\n\r", __DATE__);

}

static void cmdLoad(shellBuffer_t *buffer, uint8_t data)
{
  uint8_t argBfr[10];
  uint32_t fileIdx;

  /*! - null termination for sscanf */
  buffer->receiveBuffer.data[PRINT_BUFFER_SIZE] = 0u;

  if(-1 != shellGetArg(buffer, 1, argBfr, sizeof(argBfr), 0u))
  {
    if(1 == sscanf(argBfr, "%u", &fileIdx))
    {
      if((fileIdx > 0) && (fileIdx <= 4))
      {
        prozPrintf(buffer, "file %d loaded\n\r", fileIdx);
        trackIdx = fileIdx;
        player_load_file(fileIdx);
      }
      else
      {
        prozPrintf(buffer, "error: unsupported index %d\n\r", fileIdx);
        trackIdx = 0u;
        player_stop();
      }
    }
    else
    {
      prozPrintf(buffer, "error wrong format\n\r");
      trackIdx = 0u;
      player_stop();
    }
  }
  else
  {
    prozPrintf(buffer, "unload File\n\r");
    trackIdx = 0u;
    player_stop();
  }


}

static void cmdPlay(shellBuffer_t *buffer, uint8_t data)
{
  if((trackIdx > 0) && (trackIdx <= 4))
  {
    player_play();
    prozPrintf(buffer, "Track %d started:\n\r", trackIdx);
  }
  else
  {
    prozPrintf(buffer, "No Track loaded\n\r");
  }
}

static void cmdPause(shellBuffer_t *buffer, uint8_t data)
{
  if((trackIdx > 0) && (trackIdx <= 4))
  {
    player_pause();
    prozPrintf(buffer, "Track %d paused:\n\r", trackIdx);
  }
  else
  {
    prozPrintf(buffer, "No Track loaded\n\r");
  }
}

static void cmdStop(shellBuffer_t *buffer, uint8_t data)
{
  if((trackIdx > 0) && (trackIdx <= 4))
  {
    player_stop();
    player_load_file(trackIdx);
    prozPrintf(buffer, "Track %d stopped:\n\r", trackIdx);
  }
  else
  {
    prozPrintf(buffer, "No Track loaded\n\r");
  }
}
static void cmdInfo(shellBuffer_t *buffer, uint8_t data)
{
  uint8_t nameBuffer[32];
  uint8_t descBuffer[128];
  uint8_t trackCnt = 0u;

  data_store_global_get_name(nameBuffer);
  data_store_global_get_description(descBuffer);

  prozPrintf(buffer, "%s\n\r%s\n\r\n\r", nameBuffer, descBuffer);

  for(trackCnt = 1u; trackCnt <= 4u; trackCnt ++)
  {
    player_load_file(trackCnt);
    data_store_track_get_name(nameBuffer);
    data_store_track_get_description(descBuffer);
    prozPrintf(buffer, "Track %d:\n\r%s\n\r%s\n\r\n\r", trackCnt, nameBuffer, descBuffer);
  }

  player_load_file(trackIdx);

}

static void cmdName(shellBuffer_t *buffer, uint8_t data)
{
  uint8_t argBfr[10];
  uint8_t nameBuffer[GLOBAL_NAME_LENGTH];

  if(-1 != shellGetArg(buffer, 1, argBfr, sizeof(argBfr), 0u))
  {
    if(0 == memcmp(argBfr, "set", 3))
    {
      if(-1 != shellGetArg(buffer, 2, nameBuffer, sizeof(nameBuffer), 1u))
      {
        if((trackIdx > 0) && (trackIdx <= 4))
        {
          data_store_track_set_name(nameBuffer);
          data_store_track_save();
          prozPrintf(buffer, "Track %d set name:\n\r%s\n\r", trackIdx, nameBuffer);
        }
        else
        {
          data_store_global_set_name(nameBuffer);
          data_store_global_save();
          prozPrintf(buffer, "set global name to:\n\r%s\n\r", nameBuffer);
        }
      }
      else
      {
        prozPrintf(buffer, "error\n\r");
      }
    }
    else
    {
      prozPrintf(buffer, "error\n\r");
    }
  }
  else
  {
    if((trackIdx > 0) && (trackIdx <= 4))
    {
      data_store_track_get_name(nameBuffer);
      prozPrintf(buffer, "Track %d:\n\r%s\n\r", trackIdx, nameBuffer);
    }
    else
    {
      data_store_global_get_name(nameBuffer);
      prozPrintf(buffer, "%s\n\r", nameBuffer);
    }
  }
}

static void cmdDesc(shellBuffer_t *buffer, uint8_t data)
{
  uint8_t argBfr[10];
  uint8_t descBuffer[GLOBAL_DESCRIPTION_LENGTH];

  if(-1 != shellGetArg(buffer, 1, argBfr, sizeof(argBfr), 0u))
  {
    if(0 == memcmp(argBfr, "set", 3))
    {
      if(-1 != shellGetArg(buffer, 2, descBuffer, sizeof(descBuffer), 1u))
      {
        if((trackIdx > 0) && (trackIdx <= 4))
        {
          data_store_track_set_description(descBuffer);
          data_store_track_save();
          prozPrintf(buffer, "Track %d set description:\n\r%s\n\r", trackIdx, descBuffer);
        }
        else
        {
          data_store_global_set_description(descBuffer);
          data_store_global_save();
          prozPrintf(buffer, "set global description to:\n\r%s\n\r", descBuffer);
        }
      }
      else
      {
        prozPrintf(buffer, "error\n\r");
      }
    }
    else
    {
      prozPrintf(buffer, "error\n\r");
    }
  }
  else
  {
    if((trackIdx > 0) && (trackIdx <= 4))
    {
      data_store_track_get_description(descBuffer);
      prozPrintf(buffer, "Track %d:\n\r%s\n\r", trackIdx, descBuffer);
    }
    else
    {
      data_store_global_get_description(descBuffer);
      prozPrintf(buffer, "%s\n\r", descBuffer);
    }
  }
}

static void cmdSave(shellBuffer_t *buffer, uint8_t data)
{
  prozPrintf(buffer, "Save global Data.\n\r");
  data_store_global_save();

  if((trackIdx > 0) && (trackIdx <= 4))
  {
    prozPrintf(buffer, "Save data of Track %d\n\r", trackIdx);
    data_store_track_save();
  }
}

static void cmdReset(shellBuffer_t *buffer, uint8_t data)
{
  NVIC_SystemReset();
}

static void cmdAmp(shellBuffer_t *buffer, uint8_t data)
{
  dataStorageAudio_t audio_data;
  uint8_t argBfr[16];

  if((trackIdx > 0) && (trackIdx <= 4))
  {
    data_store_track_get_audio(&audio_data);

    if(-1 != shellGetArg(buffer, 1, argBfr, sizeof(argBfr), 0u))
    {
      if(0 == memcmp(argBfr, "enable", 6))
      {
        audio_data.amp_active = 1u;
        data_store_track_set_audio(&audio_data);
        data_store_track_save();
        amp_start();
        prozPrintf(buffer, "Track %d - amp enabled\n\r", trackIdx);
      }
      else if(0 == memcmp(argBfr, "disable", 7))
      {
        audio_data.amp_active = 0u;
        data_store_track_set_audio(&audio_data);
        data_store_track_save();
        amp_stop();
        prozPrintf(buffer, "Track %d - amp disabled\n\r", trackIdx);
      }
      else
      {
        prozPrintf(buffer, "error\n\r");
      }
    }
    else
    {
      prozPrintf(buffer, "Track %d - amp %s\n\r", trackIdx, (audio_data.amp_active != 0) ? "enabled" : "disabled");
    }
  }
  else
  {
    prozPrintf(buffer, "No Track loaded\n\r");
  }
}

static void cmdVol(shellBuffer_t *buffer, uint8_t data)
{
  static dataStorageAudio_t audio_data;
  uint8_t argBfr[16];
  int tempVolume;
  uint8_t success = 0u;

  if((trackIdx > 0) && (trackIdx <= 4))
  {
    if(UINT32_MAX != cmdRun)
    {
      if( ('+' == data) ||
          ('-' == data) ||
          ('q' == data) ||
          ('a' == data) ||
          ('w' == data) ||
          ('s' == data))
      {
        switch(data)
        {
        case '+':
          audio_data.volume[0] ++;
          audio_data.volume[1] ++;
          break;
        case '-':
          audio_data.volume[0] --;
          audio_data.volume[1] --;
          break;
        case 'q':
          audio_data.volume[0] ++;
          break;
        case 'a':
          audio_data.volume[0] --;
          break;
        case 'w':
          audio_data.volume[1] ++;
          break;
        case 's':
          audio_data.volume[1] --;
          break;
        default:
          break;
        }

        pcm_set_volume(SPEAKER_LEFT, audio_data.volume[0]);
        pcm_set_volume(SPEAKER_RIGHT, audio_data.volume[1]);

        prozPrintf(buffer, "Track %d - volume:\n\rl: %d\n\rr: %d\n\r", trackIdx, audio_data.volume[0], audio_data.volume[1]);
      }
      else if(  ('\e' == data) ||
                (0x03 == data))
      {
        cmdRun = UINT32_MAX;

        buffer->receiveBuffer.writePtr = 0u;
        buffer->receiveBuffer.readPtr = 0u;

        data_store_track_get_audio(&audio_data);
        pcm_set_volume(SPEAKER_LEFT, audio_data.volume[0]);
        pcm_set_volume(SPEAKER_RIGHT, audio_data.volume[1]);

        prozPrintf(buffer, "reset Volume: Track %d - volume:\n\rl: %d\n\rr: %d\n\r", trackIdx, audio_data.volume[0], audio_data.volume[1]);

        shellPrintPrompt(buffer);
      }
      else if ('\r' == data)
      {
        cmdRun = UINT32_MAX;
        data_store_track_set_audio(&audio_data);
        data_store_track_save();
        prozPrintf(buffer, "save Volume: Track %d - volume:\n\rl: %d\n\rr: %d\n\r", trackIdx, audio_data.volume[0], audio_data.volume[1]);
        shellPrintPrompt(buffer);
      }
      else
      {
        prozPrintf(buffer, "unsupported");
      }
    }
    else
    {
      if(-1 != shellGetArg(buffer, 1, argBfr, sizeof(argBfr), 0u))
      {
        if(0 == memcmp(argBfr, "set", 3))
        {
          data_store_track_get_audio(&audio_data);

          if(-1 != shellGetArg(buffer, 2, argBfr, sizeof(argBfr), 0u))
          {
            if(1u == sscanf(argBfr, "%d", &tempVolume))
            {
              if(tempVolume >= 0 && tempVolume <= 255)
              {
                audio_data.volume[0] = (uint8_t)tempVolume;
                if(-1 != shellGetArg(buffer, 3, argBfr, sizeof(argBfr), 0u))
                {
                  if(1u == sscanf(argBfr, "%d", &tempVolume))
                  {
                    if(tempVolume >= 0 && tempVolume <= 255)
                    {
                      audio_data.volume[1] = (uint8_t)tempVolume;
                      success = 1u;
                      data_store_track_set_audio(&audio_data);
                      data_store_track_save();
                      pcm_set_volume(SPEAKER_LEFT, audio_data.volume[0]);
                      pcm_set_volume(SPEAKER_RIGHT, audio_data.volume[1]);
                      prozPrintf(buffer, "save Volume: Track %d - volume:\n\rl: %d\n\rr: %d\n\r", trackIdx, audio_data.volume[0], audio_data.volume[1]);

                    }
                  }
                }
              }
            }
            if(0u == success)
            {
              prozPrintf(buffer, "error");
            }
          }
          else
          {
            /*! - activate this command for further shit */
            cmdRun = tempCmdRun;
            prozPrintf(buffer, "entered volume mode \n\rTrack %d - volume:\n\rl: %d\n\rr: %d\n\r", trackIdx, audio_data.volume[0], audio_data.volume[1]);
          }

        }

        else
        {
          prozPrintf(buffer, "error\n\r");
        }
      }
      else
      {
        data_store_track_get_audio(&audio_data);
        prozPrintf(buffer, "Track %d - volume:\n\rl: %d\n\rr: %d\n\r", trackIdx, audio_data.volume[0], audio_data.volume[1]);
      }
    }
  }
  else
  {
    prozPrintf(buffer, "No Track loaded\n\r");
    cmdRun = UINT32_MAX;
  }

  if(UINT32_MAX != cmdRun)
  {
    prozPrintf(buffer, "\n\rvolume(both +/- | left q/a | right w/s | return to save->");
  }

}
static void cmdStats(shellBuffer_t *buffer, uint8_t data)
{
  operationData_t stats;
  data_store_get_operation_data(&stats);
  uint8_t argBfr[16];

  if(-1 != shellGetArg(buffer, 1, argBfr, sizeof(argBfr), 0u))
  {
    if(0 == memcmp(argBfr, "init", 4))
    {
      prozPrintf(buffer, "Initializing stats\n\r");
      stats.operating_time      = 0u;
      stats.buttons_pressed[0]  = 0u;
      stats.buttons_pressed[1]  = 0u;
      stats.buttons_pressed[2]  = 0u;
      stats.buttons_pressed[3]  = 0u;

      data_store_set_operation_data(&stats);
      data_store_global_save();

    }
    else
    {
      prozPrintf(buffer, "error\n\r");
    }
  }
  else
  {
    prozPrintf(buffer, "Statistics:\n\ropTime: %d:%02d\n\rbtn1: %d\n\rbtn2: %d\n\rbtn3: %d\n\rbtn4: %d\n\r",
        stats.operating_time/60,
        stats.operating_time%60,
        stats.buttons_pressed[0],
        stats.buttons_pressed[1],
        stats.buttons_pressed[2],
        stats.buttons_pressed[3]);
  }
}

static void cmdDmx(shellBuffer_t *buffer, uint8_t data)
{
  uint8_t argBfr[10];
  uint16_t dmxSize = data_store_get_dmx_size();
  uint32_t dmxCounter = 0u;

  if(-1 != shellGetArg(buffer, 1, argBfr, sizeof(argBfr), 0u))
  {
    if(0 == memcmp(argBfr, "set", 3))
    {
      if(-1 != shellGetArg(buffer, 2, argBfr, sizeof(argBfr), 0u))
      {
        if(1 == sscanf(argBfr, "%u", &dmxSize))
        {
          if(dmxSize <= DMX_MAX_CHANNEL)
          {
            data_store_set_dmx_size(dmxSize);
            data_store_global_save();
            prozPrintf(buffer, "Set DMX size to %u\n\r", dmxSize);
          }
          else
          {
            prozPrintf(buffer, "Invalid DMX size: %u\n\r", dmxSize);
          }
        }
      }
    }
    else if(0 == memcmp(argBfr, "record", 6))
    {
      dmx_received_sets = 0;

      if((trackIdx > 0) && (trackIdx <= 4))
      {
        prozPrintf(buffer, "DMX start record of track: %u\n\r", trackIdx);
        player_load_file(trackIdx);
        dmx_start_receive();
        player_start_dmx_record(trackIdx);
      }
      else
      {
        dmx_start_receive();

        while(dmx_received_sets < 3 && (dmxCounter < 10000000))
        {
          dmxCounter ++;
        }

        if(dmx_received_sets >= 3)
        {
          data_store_record_default_dmx();
          data_store_global_save();
          prozPrintf(buffer, "Stored the default DMX data");
        }
        else
        {
          prozPrintf(buffer, "DMX timeout");
        }
      }

    }
  }
  else
  {
    prozPrintf(buffer, "DMX size: %u\n\r", dmxSize);
  }

}

static void cmdAdr(shellBuffer_t *buffer, uint8_t data)
{
  uint8_t argBfr[16];
  int tempAdr;

  if(-1 != shellGetArg(buffer, 1, argBfr, sizeof(argBfr), 0u))
  {
    if(0 == memcmp(argBfr, "set", 3))
    {
      if(-1 != shellGetArg(buffer, 2, argBfr, sizeof(argBfr), 0u))
      {
        if(1 == sscanf(argBfr, "%u", &tempAdr))
        {
          if(tempAdr > 0 & tempAdr <= UINT8_MAX)
          {
            data_store_global_set_adress((uint8_t)tempAdr);
            data_store_global_save();
            prozPrintf(buffer, "Remote adress set to: %d\n\r", data_store_global_get_adress());
          }
          else
          {
            prozPrintf(buffer, "unsupported adress: %d\n\r", tempAdr);
          }
        }
        else
        {
          prozPrintf(buffer, "error\n\r");
        }
      }
      else
      {
        prozPrintf(buffer, "error\n\r");
      }
    }
    else
    {
      prozPrintf(buffer, "error\n\r");
    }
  }
  else
  {
    prozPrintf(buffer, "Remote adress: %d\n\r", data_store_global_get_adress());
  }
}

static void cmdRemote(shellBuffer_t *buffer, uint8_t data)
{
  uint8_t argBfr[10];
  static uint32_t remoteAdr;

  if(UINT32_MAX != cmdRun)
  {
    if(0x03 == data)
    {
      cmdRun = UINT32_MAX;
      prozPrintf(buffer, "closing remnote to: %u", remoteAdr);
      shellPrintPrompt(buffer);
    }
    else
    {
      sendByte(&remoteShellBuffer, data);
      RemoteMasterTransmit(remoteAdr);
    }
  }
  else if(buffer == &remoteShellBuffer)
  {
    prozPrintf(buffer, "no nested remotes :(");
  }
  else
  {
    if(-1 != shellGetArg(buffer, 1, argBfr, sizeof(argBfr), 0u))
    {
      if(1 == sscanf(argBfr, "%u", &remoteAdr))
      {
        if((remoteAdr > 0) && (remoteAdr <= 254))
        {
          prozPrintf(buffer, "openig remote to: %u", remoteAdr);
          cmdRun = tempCmdRun;
        }
        else
        {
          prozPrintf(buffer, "invalid adress: %u", remoteAdr);
        }
      }
      else
      {
        prozPrintf(buffer, "remote adress missing");
      }
    }
    else
    {
      prozPrintf(buffer, "remote adress missing");
    }
  }
}
