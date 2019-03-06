#pragma once

#include <Arduino.h>

class AutoUpdate
{
  String baseURL;

  uint32_t serverVerNum;    // The available version number or -1 if not found
  String serverVerStr;      // the string for version name
  uint8_t serverSha256[32]; // the sha256 for the file on the server

public:
  /**
       We look for update control files at this location (*.ucf)
     */
  AutoUpdate(String baseURL);
  /**
       Return true if the server says an update is available
     */
  bool isUpdateAvailable();

  /**
       if an update is needed or force is true, do an update
     */
  void update(bool force);

  /**
       Something is really wrong with this appload, force a rollback to the previous version (if possible).
       If this routine succeeds it will not return
     */
  void doRollback();

  /** Tell the upgrader that the currently running appload is good in all important ways and further failures should not trigger a rollback
     */
  void declareSuccess();
};

void dump_partitions();