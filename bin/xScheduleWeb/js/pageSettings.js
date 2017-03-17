$(document).ready(function() {
  loadSettings();
  populateSideBar();

  $("#toggleMuteSetting").mouseup(function() {
    if ($("#smartVolumeSetting").prop("checked") == true)
      $("#smartVolumeSetting").prop("checked", false);
  });
  $("#smartVolumeSetting").mouseup(function() {
    if ($("#toggleMuteSetting").prop("checked") == true)
      $("#toggleMuteSetting").prop("checked", false);
  });
});

//if enter
$(document).keypress(function(e) {
  if (e.which == 13) {
    updateSettings();
  }
});

function loadSettings() {
  if (uiSettings != undefined) {
    $('#webNameSetting').val(uiSettings.webName);
    $('#webColorSetting').val(uiSettings.webColor);
    $('#notificationLevelSetting').val(uiSettings.notificationLevel);
    //home

    $("#homeNavSetting").prop("checked", getPage(uiSettings, "home").values[0]);
    $("#homeStatusSetting").prop("checked", getPage(uiSettings, "home").values[1]);
    //playlists
    $("#playlistNavSetting").prop("checked", getPage(uiSettings, "playlists").values[0]);
    $("#playlistStatusSetting").prop("checked", getPage(uiSettings, "playlists").values[1]);
    //settings
    $("#settingsNavSetting").prop("checked", getPage(uiSettings, "settings").values[0]);
    $("#settingsStatusSetting").prop("checked", getPage(uiSettings, "settings").values[1]);
    //nav buttons
    $("#smartVolumeSetting").prop("checked", uiSettings.navbuttons[0]);
    $("#smartBrightnessSetting").prop("checked", uiSettings.navbuttons[1]);
    $("#outputToLightsSetting").prop("checked", uiSettings.navbuttons[2]);
    $("#repeatPlaylistSetting").prop("checked", uiSettings.navbuttons[3]);
    $("#repeatStepsSetting").prop("checked", uiSettings.navbuttons[4]);
    $("#toggleRandomSetting").prop("checked", uiSettings.navbuttons[5]);
    $("#toggleMuteSetting").prop("checked", uiSettings.navbuttons[6]);

  } else {
    sleep(100),
      function() {
        loadSettings();
      }
  }
}


function updateSettings() {

  var updatedSettings = {
    "webName": $('#webNameSetting').val(),
    "webColor": $('#webColorSetting').val(),
    "notificationLevel": $('#notificationLevelSetting').val(),
    "pages": [{
      "page": "home",
      "values": [
        $("#homeNavSetting").prop("checked"),
        $("#homeStatusSetting").prop("checked")
      ]
    }, {
      "page": "playlists",
      "values": [
        $("#playlistNavSetting").prop("checked"),
        $("#playlistStatusSetting").prop("checked")
      ]
    }, {
      "page": "settings",
      "values": [
        $("#settingsNavSetting").prop("checked"),
        $("#settingsStatusSetting").prop("checked")
      ]
    }],
    "navbuttons": [
      $("#smartVolumeSetting").prop("checked"),
      $("#smartBrightnessSetting").prop("checked"),
      $("#outputToLightsSetting").prop("checked"),
      $("#repeatPlaylistSetting").prop("checked"),
      $("#repeatStepsSetting").prop("checked"),
      $("#toggleRandomSetting").prop("checked"),
      $("#toggleMuteSetting").prop("checked")
    ]
  };
  //Save settings, Update UI
  storeKey('uiSettings', JSON.stringify(updatedSettings));
  uiSettings = JSON.parse(updatedSettings);
  populateUI();
  populateSideBar();

}
