$(document).ready(function() {
  loadSettings();
  populateSideBar();


        $('#webColorSetting').colorpicker();

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
})

function loadSettings() {
  if (uiSettings != undefined) {
    $('#webNameSetting').val(uiSettings.webName);
    $('#webColorSetting').val(uiSettings.webColor);
    $('#notificationLevelSetting').val(uiSettings.notificationLevel);
    //home

    $("#homeNavSetting").prop("checked", getPage(uiSettings, "home").values[0]);
    $("#homeStatusSetting").prop("checked", getPage(uiSettings, "home").values[1]);
    //playlists
    $("#playlistsNavSetting").prop("checked", getPage(uiSettings, "playlists").values[0]);
    $("#playlistsStatusSetting").prop("checked", getPage(uiSettings, "playlists").values[1]);
    //settings
    $("#settingsNavSetting").prop("checked", getPage(uiSettings, "settings").values[0]);
    $("#settingsStatusSetting").prop("checked", getPage(uiSettings, "settings").values[1]);
    //nav buttons
    $("#smartVolumeSetting").prop("checked", uiSettings.navbuttons[0].volumeMute);
    $("#smartBrightnessSetting").prop("checked", uiSettings.navbuttons[0].brightnessLevel);
    $("#outputToLightsSetting").prop("checked", uiSettings.navbuttons[0].outputtolights);
    $("#repeatPlaylistSetting").prop("checked", uiSettings.navbuttons[0].playlistlooping);
    $("#repeatStepsSetting").prop("checked", uiSettings.navbuttons[0].steplooping);
    $("#toggleRandomSetting").prop("checked", uiSettings.navbuttons[0].random);
    $("#toggleMuteSetting").prop("checked", uiSettings.navbuttons[0].toggleMute);

  } else {
    sleep(100),
      function() {
        loadSettings();
      }
  }
}


function updateSettings() {
  window.scrollTo(0, 0);
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
        $("#playlistsNavSetting").prop("checked"),
        $("#playlistsStatusSetting").prop("checked")
      ]
    }, {
      "page": "settings",
      "values": [
        $("#settingsNavSetting").prop("checked"),
        $("#settingsStatusSetting").prop("checked")
      ]
    }],
    "navbuttons": [{
      "random": $("#toggleRandomSetting").prop("checked"),
      "steplooping": $("#repeatStepsSetting").prop("checked"),
      "playlistlooping": $("#repeatPlaylistSetting").prop("checked"),
      "volumeMute": $("#smartVolumeSetting").prop("checked"),
      "brightnessLevel": $("#smartBrightnessSetting").prop("checked"),
      "outputtolights": $("#outputToLightsSetting").prop("checked"),
      "toggleMute": $("#toggleMuteSetting").prop("checked")
    }]
  };
  //Save settings, Update UI
  storeKey('uiSettings', JSON.stringify(updatedSettings));
  uiSettings = updatedSettings;
  populateUI();
  populateSideBar();


}
